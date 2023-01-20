#include <WiFi.h>
#include <esp_wifi.h>
#include <PubSubClient.h>
#include <ModbusMaster.h>

//************************ Definimos pines puerto Serie2 ***********************//
#define RXD2 16
#define TXD2 17

//************************Datos de Wifi***********************//
const char* ssid = "DemoColegio";
const char* password = "DemoColegio";

//************************Datos de Brocker***********************//

const char* mqtt_server = "192.168.100.67";
const char* id = "SensorRS485";
const char* user = "RS4851";
const char* codePass = "cpce1901";
int puerto = 1883;

//************************ Topicos ***********************//

const char* topicoV1 = "demo/V1"; //Voltaje L1
const char* topicoV2 = "demo/V2"; //Voltaje L2
const char* topicoV3 = "demo/V3"; //Voltaje L3

const char* topicoV12 = "demo/V12"; //Voltaje L1 y L2
const char* topicoV13 = "demo/V13"; //Voltaje L1 y L3
const char* topicoV23 = "demo/V23"; //Voltaje L2 y L2

const char* topicoI1 = "demo/I1"; //Corriente L1
const char* topicoI2 = "demo/I2"; //Corriente L2
const char* topicoI3 = "demo/I3"; //Corriente L3

const char* topicoP1 = "demo/P1"; //Potencia L1
const char* topicoP2 = "demo/P2"; //Potencia L2
const char* topicoP3 = "demo/P3"; //Potencia L3

const char* topicoHZ = "demo/HZ";  //HZ
const char* topicoPA = "demo/PA";  //Potencia Activa
const char* topicoFP = "demo/FP";  //Potencia Activa

//************************ Variables envio ***********************//

char out_V1[8];
char out_V2[8];
char out_V3[8];

char out_V12[8];
char out_V13[8];
char out_V23[8];

char out_I1[8];
char out_I2[8];
char out_I3[8];

char out_P1[8];
char out_P2[8];
char out_P3[8];

char out_HZ[8];
char out_PA[32];
char out_FP[8];

//************************ Variables globales ***********************//

#define timeSend 5000  // Tiempo entre cada envio
#define timeRead 2500  // Tiempo entre cada lectura


//************************ Manejo de tiempos ***********************//

unsigned long Task_1 = 0;
unsigned long Task_2 = 0;
unsigned long Task_3 = 0;

//************************ Objetos de sistema ***********************//

WiFiClient espClient;
PubSubClient client(espClient);
ModbusMaster node;

//************************ Conexion Wifi ***********************//

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("OK, Estamos conectado en WIFI");
}

//************************ Reconect ***********************//

void reconnect() {
  while (!client.connected()) {
    Serial.println("Intentando concetar con servidor MQTT...");
    if (client.connect(id, user, codePass)) {
      Serial.println("connectado a Broker");
    } else {
      Serial.print("Falla, Estado: ");
      Serial.print(client.state());
      delay(250);
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node.begin(1, Serial2);
  setup_wifi();
  client.setServer(mqtt_server, puerto);
}

void loop() {

  //************************ Trabajo 1: Crear valores ***********************//

  if (millis() - Task_1 > timeRead) {
    Task_1 = millis();

    float V1 = readOneRegister(0x2C) * 0.1;  
    float V2 = readOneRegister(4033) * 0.1;
    float V3 = readOneRegister(4034) * 0.1;

    float V12 = readOneRegister(4029) * 0.1;
    float V13 = readOneRegister(4031) * 0.1;
    float V23 = readOneRegister(4030) * 0.1;

    float I1 = readOneRegister(4019) * 0.1;
    float I2 = readOneRegister(4020) * 0.1;
    float I3 = readOneRegister(4021) * 0.1;

    float P1 = readOneRegister(4035) * 0.1;
    float P2 = readOneRegister(4036) * 0.1;
    float P3 = readOneRegister(4037) * 0.1;

    float HZ = readOneRegister(4012) * 0.01;
    float FP = readOneRegister(4008) * 0.0001;
    float PA = readTwoRegister(3999) * 0.1;

    // Conversiones
    dtostrf(V1, 3, 1, out_V1);
    dtostrf(V2, 3, 1, out_V2);
    dtostrf(V3, 3, 1, out_V3);

    dtostrf(V12, 3, 1, out_V12);
    dtostrf(V13, 3, 1, out_V13);
    dtostrf(V23, 3, 1, out_V23);

    dtostrf(I1, 3, 1, out_I1);
    dtostrf(I2, 3, 1, out_I2);
    dtostrf(I3, 3, 1, out_I3);

    dtostrf(P1, 3, 1, out_P1);
    dtostrf(P2, 3, 1, out_P2);
    dtostrf(P3, 3, 1, out_P3);

    dtostrf(HZ, 3, 1, out_HZ);
    dtostrf(PA, 3, 1, out_PA);
    dtostrf(FP, 3, 1, out_FP);

    Serial.print(V1);
    Serial.print(" - ");
    Serial.print(V2);
    Serial.print(" - ");
    Serial.print(V3);
    Serial.print(" - ");
    Serial.print(V12);
    Serial.print(" - ");
    Serial.print(V13);
    Serial.print(" - ");
    Serial.print(V23);
    Serial.print(" - ");
    Serial.print(I1);
    Serial.print(" - ");
    Serial.print(I2);
    Serial.print(" - ");
    Serial.print(I3);
    Serial.print(" - ");
    Serial.print(P1);
    Serial.print(" - ");
    Serial.print(P2);
    Serial.print(" - ");
    Serial.print(P3);
    Serial.print(" - ");
    Serial.print(HZ);
    Serial.print(" - ");
    Serial.print(PA);
    Serial.print(" - ");
    Serial.println(FP);

  }

  //************************ Trabajo 2 ***********************//

  if (millis() - Task_2 > timeSend) {
    Task_2 = millis();
    client.publish(topicoV1, out_V1);
    client.publish(topicoV2, out_V2);
    client.publish(topicoV3, out_V3);

    client.publish(topicoV12, out_V12);
    client.publish(topicoV13, out_V13);
    client.publish(topicoV23, out_V23);

    client.publish(topicoI1, out_I1);
    client.publish(topicoI2, out_I2);
    client.publish(topicoI3, out_I3);

    client.publish(topicoP1, out_P1);
    client.publish(topicoP2, out_P2);
    client.publish(topicoP3, out_P3);

    client.publish(topicoHZ, out_HZ);
    client.publish(topicoPA, out_PA);
    client.publish(topicoFP, out_FP);

    Serial.println("Enviando....");
  }

  //************************ Trabajo 3 ***********************//

  if (millis() - Task_3 > 100) {
    Task_3 = millis();
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

  }
}


int readOneRegister(uint16_t Reg) {
  uint8_t i, result;
  result = node.readHoldingRegisters(Reg, 1);

  if (result == node.ku8MBSuccess) {
    uint16_t a;
    a = node.getResponseBuffer(0);

    return a;
  }
}


long readTwoRegister(int Reg) {
  uint8_t result;
  result = node.readHoldingRegisters(Reg, 2);
  if ( result == node.ku8MBSuccess ) {
    uint16_t a, b;
    uint32_t c;
    long salida;
    a = node.getResponseBuffer(0);
    b = node.getResponseBuffer(1);
    c = a << 16 | b; // Los unimos para obtener un entero sin signo de 32bits.
    salida = (long)c; // Convierto el entero sin signo a long (con signo)

    return salida;
  }
}
