/*
  Librerias para manejar interner wifi, HTTP, MQTT, lectura modbus y Json
*/
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ModbusMaster.h>

//************************ Datos gateway ***********************//

const char* cliente = "SCollege";
const char* lugar = "Salaelectrica";
const char* sensor1 = "1";
const char* sensor2 = "2";
const char* sensor3 = "3";
const char* sensor4 = "4";
const char* sensor5 = "5";

//************************Datos de Wifi***********************//
const char* ssid = "Jessica2.4";
const char* password = "167832873";

//************************Datos de Brocker***********************//

const char* mqtt_server = "146.190.124.66";
const char* id = "SCollege/Salaelectrica/1-5";
const char* user = "Esp321-1";
const char* codePass = "Esp321-1";
unsigned short puerto = 1883;

//************************Datos de Servidor receptor***********************//

const char* endpoint = "http://164.92.64.11/lectures/add/";


//************************ Definimos pines indicadores LED ***********************//

#define led_conectado_wifi 5
#define led_conectado_mqtt 23
#define led_envio 19  //19 pin led salida 2 pin interno led

//************************ Definimos pines salida Rele ***********************//

#define rele_out 27

//************************ Definimos pines puerto Serie2 ***********************//

#define RXD2 17
#define TXD2 16

//************************ Tiempos de envio ***********************//

unsigned short tiempo_lecturas = 500;        // Tiempo entre cada envio
unsigned short tiempo_envio_mqtt = 1500;     // Tiempo entre cada lectura
unsigned int tiempo_envio_endpoint = 60000;  // Tiempo entre cada lectura

//************************ Tiempos de tareas ***********************//

unsigned long Task_1 = 0;
unsigned long Task_2 = 0;
unsigned long Task_3 = 0;
unsigned long Task_4 = 0;
unsigned long led_start_time = 0;

//************************ Objetos de sistema ***********************//

WiFiClient espClient;            // Objeto para conexion WiFi
PubSubClient client(espClient);  // Objeto para conexion MQTT
ModbusMaster node1;              // Objeto para Modbus
ModbusMaster node2;              // Objeto para Modbus
ModbusMaster node3;              // Objeto para Modbus
ModbusMaster node4;              // Objeto para Modbus
ModbusMaster node5;              // Objeto para Modbus

//************************ VAriables globales ***********************//

String datos_sensor_1 = "";
String datos_sensor_2 = "";
String datos_sensor_3 = "";
String datos_sensor_4 = "";
String datos_sensor_5 = "";

bool ledIsOn = false;

bool send_1_mqtt = false;
bool send_2_mqtt = false;
bool send_3_mqtt = false;
bool send_4_mqtt = false;
bool send_5_mqtt = false;

bool active_sensor1 = false;
bool active_sensor2 = false;
bool active_sensor3 = false;
bool active_sensor4 = false;
bool active_sensor5 = false;

//************************ Funcion Conexion Wifi ***********************//

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led_conectado_wifi, LOW);
    Serial.print(".");
    delay(500);
  }
  Serial.println("OK, Estamos conectado en WIFI");
  digitalWrite(led_conectado_wifi, HIGH);
}

//************************ Reconect Broker MQTT ***********************//

void reconnect() {
  while (!client.connected()) {
    Serial.println("Intentando concetar con servidor MQTT...");
    if (client.connect(id, user, codePass)) {
      Serial.println("connectado a Broker");
      digitalWrite(led_conectado_mqtt, HIGH);
    } else {
      digitalWrite(led_conectado_mqtt, LOW);
      Serial.print("Falla, Estado: ");
      Serial.print(client.state());
      delay(250);
    }
  }
}

bool detectDevice(ModbusMaster& node, int address) {
  uint16_t dummyData;
  uint8_t result = node.readHoldingRegisters(0, 1);
  return (result == node.ku8MBSuccess);
}

void setup() {

  pinMode(led_conectado_wifi, OUTPUT);
  pinMode(led_conectado_mqtt, OUTPUT);
  pinMode(led_envio, OUTPUT);
  pinMode(rele_out, OUTPUT);

  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  setup_wifi();
  client.setServer(mqtt_server, puerto);

  node1.begin(1, Serial2);
  node2.begin(2, Serial2);
  node3.begin(3, Serial2);
  node4.begin(4, Serial2);
  node5.begin(5, Serial2);

  if (detectDevice(node1, 1)) {
    Serial.println("Sensor 1 detectado y disponible.");
    active_sensor1 = true;
  } else {
    Serial.println("Sensor 1 no detectado o no disponible.");
  }

  if (detectDevice(node2, 2)) {
    Serial.println("Sensor 2 detectado y disponible.");
    active_sensor2 = true;
  } else {
    Serial.println("Sensor 2 no detectado o no disponible.");
  }

  if (detectDevice(node3, 3)) {
    Serial.println("Sensor 3 detectado y disponible.");
    active_sensor3 = true;
  } else {
    Serial.println("Sensor 3 no detectado o no disponible.");
  }

  if (detectDevice(node4, 4)) {
    Serial.println("Sensor 4 detectado y disponible.");
    active_sensor4 = true;
  } else {
    Serial.println("Sensor 4 no detectado o no disponible.");
  }

  if (detectDevice(node5, 5)) {
    Serial.println("Sensor 5 detectado y disponible.");
    active_sensor5 = true;
  } else {
    Serial.println("Sensor 5 no detectado o no disponible.");
  }
}

void loop() {

  //************************ Trabajo 1 Verifica si esta conectado al broker MQTT ***********************//
  if (millis() - Task_1 >= 10) {
    Task_1 = millis();
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }

  //************************ Trabajo 1: Realizar lecturas y crear valores ***********************//

  if (millis() - Task_2 >= tiempo_lecturas) {
    Task_2 = millis();

    datos_sensor_1 = "";
    datos_sensor_2 = "";
    datos_sensor_3 = "";
    datos_sensor_4 = "";
    datos_sensor_5 = "";

    if (active_sensor1) {
      DynamicJsonDocument jsonDoc_1(1024);
      readSensor(node1, jsonDoc_1, 1);
      serializeJson(jsonDoc_1, datos_sensor_1);
    }
    if (active_sensor2) {
      DynamicJsonDocument jsonDoc_2(1024);
      readSensor(node2, jsonDoc_2, 2);
      serializeJson(jsonDoc_2, datos_sensor_2);
    }
    if (active_sensor3) {
      DynamicJsonDocument jsonDoc_3(1024);
      readSensor(node3, jsonDoc_3, 3);
      serializeJson(jsonDoc_3, datos_sensor_3);
    }
    if (active_sensor4) {
      DynamicJsonDocument jsonDoc_4(1024);
      readSensor(node4, jsonDoc_4, 4);
      serializeJson(jsonDoc_4, datos_sensor_4);
    }
    if (active_sensor5) {
      DynamicJsonDocument jsonDoc_5(1024);
      readSensor(node5, jsonDoc_5, 5);
      serializeJson(jsonDoc_5, datos_sensor_5);
    }
  }

  //************************ Trabajo 2 envio a broker MQTT ***********************//

  if (millis() - Task_3 >= tiempo_envio_mqtt) {
    Task_3 = millis();

    // Creamos topico N°1 y su valor rescatado de variable global

    if (active_sensor1 && datos_sensor_1.length() != 0) {
      String topico_sensor_1 = String(cliente) + "/" + String(lugar) + "/" + sensor1;
      const char* topico_sensor_1_char = topico_sensor_1.c_str();
      const char* datos_sensor_1_char = datos_sensor_1.c_str();
      send_1_mqtt = client.publish(topico_sensor_1_char, datos_sensor_1_char);
      //Serial.println(datos_sensor_1_char);
      Serial.println("envio mqtt 1");
    }

    // Creamos topico N°2 y su valor rescatado de variable global

    if (active_sensor2 && datos_sensor_2.length() != 0) {
      String topico_sensor_2 = String(cliente) + "/" + String(lugar) + "/" + sensor2;
      const char* topico_sensor_2_char = topico_sensor_2.c_str();
      const char* datos_sensor_2_char = datos_sensor_2.c_str();
      send_2_mqtt = client.publish(topico_sensor_2_char, datos_sensor_2_char);
      //Serial.println(datos_sensor_2_char);
      Serial.println("envio mqtt 2");
    }
    // Creamos topico N°3 y su valor rescatado de variable global

    if (active_sensor3 && datos_sensor_3.length() != 0) {
      String topico_sensor_3 = String(cliente) + "/" + String(lugar) + "/" + sensor3;
      const char* topico_sensor_3_char = topico_sensor_3.c_str();
      const char* datos_sensor_3_char = datos_sensor_3.c_str();
      send_3_mqtt = client.publish(topico_sensor_3_char, datos_sensor_3_char);
      //Serial.println(datos_sensor_3_char);
      Serial.println("envio mqtt 3");
    }
    // Creamos topico N°4 y su valor rescatado de variable global

    if (active_sensor4 && datos_sensor_4.length() != 0) {
      String topico_sensor_4 = String(cliente) + "/" + String(lugar) + "/" + sensor4;
      const char* topico_sensor_4_char = topico_sensor_4.c_str();
      const char* datos_sensor_4_char = datos_sensor_4.c_str();
      send_4_mqtt = client.publish(topico_sensor_4_char, datos_sensor_4_char);
      //Serial.println(datos_sensor_4_char);
      Serial.println("envio mqtt 4");
    }
    // Creamos topico N°5 y su valor rescatado de variable global

    if (active_sensor5 && datos_sensor_5.length() != 0) {
      String topico_sensor_5 = String(cliente) + "/" + String(lugar) + "/" + sensor5;
      const char* topico_sensor_5_char = topico_sensor_5.c_str();
      const char* datos_sensor_5_char = datos_sensor_5.c_str();
      send_5_mqtt = client.publish(topico_sensor_5_char, datos_sensor_5_char);
      //Serial.println(datos_sensor_5_char);
      Serial.println("envio mqtt 5");
    }

    // Evalúa si send_2_mqtt es verdadero y el temporizador no ha comenzado
    if (send_1_mqtt && send_2_mqtt && send_3_mqtt && send_4_mqtt && send_5_mqtt && !ledIsOn) {
      digitalWrite(led_envio, HIGH);  // Enciende el LED
      ledIsOn = true;                 // Marca el LED como encendido
      led_start_time = millis();      // Registra el tiempo de inicio
      Serial.println("Envio MQTT exitoso");
    }

    // Si el temporizador ha comenzado y ha pasado el tiempo deseado
    if (ledIsOn && (millis() - led_start_time >= 500)) {
      digitalWrite(led_envio, LOW);  // Apaga el LED
      ledIsOn = false;               // Marca el LED como apagado
      send_1_mqtt = false;
      send_2_mqtt = false;
      send_3_mqtt = false;
      send_4_mqtt = false;
      send_5_mqtt = false;
    }
  }

  //************************ Trabajo 3 Envio a Endpoint ***********************//

  if (millis() - Task_4 >= tiempo_envio_endpoint) {
    Task_4 = millis();
    if (active_sensor1 && datos_sensor_1.length() != 0) {
      send_post(datos_sensor_1);
    }
    if (active_sensor2 && datos_sensor_2.length() != 0) {
      send_post(datos_sensor_2);
    }
    if (active_sensor3 && datos_sensor_3.length() != 0) {
      send_post(datos_sensor_3);
    }
    if (active_sensor4 && datos_sensor_4.length() != 0) {
      send_post(datos_sensor_4);
    }
    if (active_sensor5 && datos_sensor_5.length() != 0) {
      send_post(datos_sensor_5);
    }
  }
}

void readSensor(ModbusMaster& node, DynamicJsonDocument& jsonDoc, short number) {

  float v1 = readOneRegister(node, 0) * 0.01;
  float v2 = readOneRegister(node, 1) * 0.01;
  float v3 = readOneRegister(node, 2) * 0.01;

  float v12 = readOneRegister(node, 103) * 0.01;
  float v23 = readOneRegister(node, 104) * 0.01;
  float v13 = readOneRegister(node, 105) * 0.01;

  float i1 = readOneRegister(node, 3) * 0.001;
  float i2 = readOneRegister(node, 5) * 0.001;
  float i3 = readOneRegister(node, 7) * 0.001;

  float p1 = readOneRegister(node, 116) * 0.1;
  float p2 = readOneRegister(node, 118) * 0.1;
  float p3 = readOneRegister(node, 120) * 0.1;

  float pa = readOneRegister(node, 14) * 0.01;
  float fp = readOneRegister(node, 10) * 0.001;
  float hz = readOneRegister(node, 9) * 0.01;

  Serial.print(v1);
  Serial.print(" - ");
  Serial.print(v2);
  Serial.print(" - ");
  Serial.print(v3);
  Serial.print(" - ");
  Serial.print(v13);
  Serial.print(" - ");
  Serial.print(v12);
  Serial.print(" - ");
  Serial.print(v23);
  Serial.print(" - ");
  Serial.print(i1);
  Serial.print(" - ");
  Serial.print(i2);
  Serial.print(" - ");
  Serial.print(i3);
  Serial.print(" - ");
  Serial.print(p1);
  Serial.print(" - ");
  Serial.print(p2);
  Serial.print(" - ");
  Serial.print(p3);
  Serial.print(" - ");
  Serial.print(pa);
  Serial.print(" - ");
  Serial.print(fp);
  Serial.print(" - ");
  Serial.println(hz);

  // Conversion para limitar a 2 decimales

  char out_v1[8];
  char out_v2[8];
  char out_v3[8];

  char out_v12[8];
  char out_v13[8];
  char out_v23[8];

  char out_i1[8];
  char out_i2[8];
  char out_i3[8];

  char out_p1[8];
  char out_p2[8];
  char out_p3[8];

  char out_hz[8];
  char out_pa[32];
  char out_fp[8];

  // Conversiones
  dtostrf(v1, 3, 1, out_v1);
  dtostrf(v2, 3, 1, out_v2);
  dtostrf(v3, 3, 1, out_v3);

  dtostrf(v12, 3, 1, out_v12);
  dtostrf(v13, 3, 1, out_v13);
  dtostrf(v23, 3, 1, out_v23);

  dtostrf(i1, 3, 1, out_i1);
  dtostrf(i2, 3, 1, out_i2);
  dtostrf(i3, 3, 1, out_i3);

  dtostrf(p1, 3, 1, out_p1);
  dtostrf(p2, 3, 1, out_p2);
  dtostrf(p3, 3, 1, out_p3);

  dtostrf(hz, 3, 1, out_hz);
  dtostrf(pa, 3, 1, out_pa);
  dtostrf(fp, 3, 1, out_fp);

  jsonDoc["v1"] = out_v1;
  jsonDoc["v2"] = out_v2;
  jsonDoc["v3"] = out_v3;

  jsonDoc["v13"] = out_v13;
  jsonDoc["v12"] = out_v12;
  jsonDoc["v23"] = out_v23;

  jsonDoc["i1"] = out_i1;
  jsonDoc["i2"] = out_i2;
  jsonDoc["i3"] = out_i3;

  jsonDoc["p1"] = out_p1;
  jsonDoc["p2"] = out_p2;
  jsonDoc["p3"] = out_p3;

  jsonDoc["pa"] = out_pa;
  jsonDoc["fp"] = out_fp;
  jsonDoc["hz"] = out_hz;

  jsonDoc["sensor"] = number;
}

void send_post(String datos) {
  WiFiClient client;
  HTTPClient http;  // Objeto para request
  http.begin(client, endpoint);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(datos);
  Serial.println(httpResponseCode);
  http.end();
}

int readOneRegister(ModbusMaster& node, int Reg) {
  uint8_t result;
  result = node.readHoldingRegisters(Reg, 1);
  delay(100);
  if (result == node.ku8MBSuccess) {
    uint16_t a;
    a = node.getResponseBuffer(0);
    return a;
  } else {
    Serial.println("Error");
  }
}

long int readTwoRegister(ModbusMaster nodo, int Reg) {
  uint8_t result;
  result = nodo.readHoldingRegisters(Reg, 2);
  if (result == nodo.ku8MBSuccess) {
    uint16_t a, b;
    uint32_t c;
    unsigned long salida;
    a = nodo.getResponseBuffer(0);
    b = nodo.getResponseBuffer(1);
    c = a << 16 | b;   // Los unimos para obtener un entero sin signo de 32bits.
    salida = (long)c;  // Convierto el entero sin signo a long (con signo)
    return salida;
  }
}
