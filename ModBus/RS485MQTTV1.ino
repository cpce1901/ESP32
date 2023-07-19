#include <WiFi.h>
#include <esp_wifi.h>
#include <PubSubClient.h>
#include <ModbusMaster.h>
#include <ArduinoJson.h>

//************************ Definimos pines puerto Serie2 ***********************//
#define RXD2 16
#define TXD2 17

//************************ VAriables globales ***********************//

#define sensor_id 1
float total_kw, v1, i1, p1, hz;
String jsonToSend;

//************************ Datos broker ***********************//
const char* mqtt_server = "";
const char* id = "esp32/test";
const char* user = "";
const char* codePass = "";
int puerto = 1883;
const char* topico = "TEst/test";

//************************ Tiempos ***********************//

unsigned long Task_1 = 0;
unsigned long Task_2 = 0;
unsigned long Task_3 = 0;
int tiempo_envio_mqtt = 5000;  //ms
int tiempo_envio_api = 60000;  //ms

//************************ Objetos ***********************//

ModbusMaster node;
WiFiClient espClient;
PubSubClient client(espClient);

//************************ Conexion Wifi ***********************//

void setup_wifi(const char* ssid, const char* password) {
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
      Serial.println("connectado");
    } else {
      Serial.print("Falla, Estado: ");
      Serial.print(client.state());
      Serial.println("Intentando en 5 segundos..");
      delay(5000);
    }
  }
}

//************************ Comienzo set ***********************//
void setup() {

  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node.begin(1, Serial2);

  //setup_wifi("DemoColegio2.4", "democolegio2.4");
  setup_wifi("", "");
  Serial.println("Conexión WiFi establecida");

  client.setServer(mqtt_server, puerto);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if millis() > 

  if (millis() - Task_1 >= 500) {
    Task_1 = millis();

    // Para cada operacion es necesario enviar una direccion de registro y un multiplicador //

    total_kw = readTwoRegisterPower(0, 0.1);
    v1 = readOneRegisterData(2, 0.01);
    i1 = readTwoRegisterPower(3, 1) * 0.001;
    p1 = readTwoRegisterPower(5, 0.1);
    hz = readOneRegisterData(11, 0.01);
  }

  //*************************************** Tiempo de envio por mqtt ************************************//

  if (millis() - Task_2 >= tiempo_envio_mqtt) {
    Task_2 = millis();
    jsonToSend = createJson();
    client.publish(topico, jsonToSend.c_str());
  }

  //*************************************** Tiempo de envio por server ************************************//

  if (millis() - Task_3 >= tiempo_envio_api) {
    Task_3 = millis();

    client.publish(topico, jsonToSend.c_str());
  }
}

//*************************************** Funciones ************************************//

String createJson() {
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["sensor"] = sensor_id;
  jsonDoc["v1"] = v1;
  jsonDoc["i1"] = i1;
  jsonDoc["p1"] = p1;
  jsonDoc["hz"] = hz;

  String postData;
  serializeJson(jsonDoc, postData);

  return postData;
}

float readOneRegisterData(int registerRead, float plus) {
  uint8_t result;
  result = node.readHoldingRegisters(registerRead, 1);

  if (result == node.ku8MBSuccess) {
    uint16_t a = node.getResponseBuffer(0);
    float value = a * plus;
    return value;
  }
}

float readTwoRegisterData(int registerRead, float plus) {
  uint8_t result;
  result = node.readHoldingRegisters(registerRead, 2);

  if (result == node.ku8MBSuccess) {
    uint16_t a, b;

    a = node.getResponseBuffer(0);
    b = node.getResponseBuffer(1);

    String c = String(a) + String(b);

    int c_int = c.toInt();

    float value = c_int * plus;
    return value;

  } else {
    Serial.print("Error al leer el registro: ");
    Serial.println(result);
    return 0;
  }
}

// Para leer debe rescatar 2 registros el primero multiplicarlo por el factor y el segundo asigna positivo o negativo
float readTwoRegisterPower(int registerRead, float plus) {
  uint8_t result;
  result = node.readHoldingRegisters(registerRead, 2);

  if (result == node.ku8MBSuccess) {
    int a, b;

    a = node.getResponseBuffer(0) * plus;
    b = node.getResponseBuffer(1);

    long c = (b << 16) | a;

    float value = c;
    return value;

  } else {
    Serial.print("Error al leer el registro: ");
    Serial.println(result);
    return 0;
  }
}
