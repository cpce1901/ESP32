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
#define led_envio 19 //19 pin led salida 2 pin interno led

//************************ Definimos pines salida Rele ***********************//

#define rele_out 27

//************************ Definimos pines puerto Serie2 ***********************//

#define RXD2 16
#define TXD2 17

//************************ Tiempos de envio ***********************//

unsigned short tiempo_lecturas = 200;  // Tiempo entre cada envio
unsigned short tiempo_envio_mqtt = 1000;  // Tiempo entre cada lectura
unsigned int tiempo_envio_endpoint = 10000; // Tiempo entre cada lectura

//************************ Tiempos de tareas ***********************//

unsigned long Task_1 = 0;
unsigned long Task_2 = 0;
unsigned long Task_3 = 0;
unsigned long Task_4 = 0;
unsigned long led_start_time = 0;

//************************ Objetos de sistema ***********************//

WiFiClient espClient; // Objeto para conexion WiFi
PubSubClient client(espClient); // Objeto para conexion MQTT
ModbusMaster node; // Objeto para Modbus


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

void setup() {

  pinMode(led_conectado_wifi, OUTPUT);
  pinMode(led_conectado_mqtt, OUTPUT);
  pinMode(led_envio, OUTPUT);
  pinMode(rele_out, OUTPUT);

  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node.begin(1, Serial2);
  setup_wifi();
  client.setServer(mqtt_server, puerto);

}

void loop() {

  //************************ Trabajo 1: Crear valores ***********************//

  if (millis() - Task_1 >= tiempo_lecturas) {
    Task_1 = millis();
    datos_sensor_1 = "";
    datos_sensor_2 = "";
    datos_sensor_3 = "";
    datos_sensor_4 = "";
    datos_sensor_5 = "";

    DynamicJsonDocument jsonDoc_1(1024);
    sensor_lectura(1, jsonDoc_1);
    serializeJson(jsonDoc_1, datos_sensor_1);

    DynamicJsonDocument jsonDoc_2(1024);
    sensor_lectura(2, jsonDoc_2);
    serializeJson(jsonDoc_2, datos_sensor_2);

    DynamicJsonDocument jsonDoc_3(1024);
    sensor_lectura(3, jsonDoc_3);
    serializeJson(jsonDoc_3, datos_sensor_3);

    DynamicJsonDocument jsonDoc_4(1024);
    sensor_lectura(4, jsonDoc_4);
    serializeJson(jsonDoc_4, datos_sensor_4);

    DynamicJsonDocument jsonDoc_5(1024);
    sensor_lectura(5, jsonDoc_5);
    serializeJson(jsonDoc_5, datos_sensor_5);

  }

  //************************ Trabajo 2 envio a broker MQTT ***********************//

  if (millis() - Task_2 >= tiempo_envio_mqtt) {
    Task_2 = millis();

    // Creamos topico N°1 y su valor rescatado de variable global

    if (datos_sensor_1.length() != 0) {
      String topico_sensor_1 = String(cliente) + "/" + String(lugar) + "/" + sensor1;
      const char* topico_sensor_1_char = topico_sensor_1.c_str();
      const char* datos_sensor_1_char = datos_sensor_1.c_str();
      send_1_mqtt = client.publish(topico_sensor_1_char, datos_sensor_1_char);
      //Serial.println(datos_sensor_1_char);
    }

    // Creamos topico N°2 y su valor rescatado de variable global

    if (datos_sensor_2.length() != 0) {
      String topico_sensor_2 = String(cliente) + "/" + String(lugar) + "/" + sensor2;
      const char* topico_sensor_2_char = topico_sensor_2.c_str();
      const char* datos_sensor_2_char = datos_sensor_2.c_str();
      send_2_mqtt =  client.publish(topico_sensor_2_char, datos_sensor_2_char);
      //Serial.println(datos_sensor_2_char);
    }
    // Creamos topico N°3 y su valor rescatado de variable global

    if (datos_sensor_3.length() != 0) {
      String topico_sensor_3 = String(cliente) + "/" + String(lugar) + "/" + sensor3;
      const char* topico_sensor_3_char = topico_sensor_3.c_str();
      const char* datos_sensor_3_char = datos_sensor_3.c_str();
      send_3_mqtt = client.publish(topico_sensor_3_char, datos_sensor_3_char);
      //Serial.println(datos_sensor_3_char);
    }
    // Creamos topico N°4 y su valor rescatado de variable global

    if (datos_sensor_4.length() != 0) {
      String topico_sensor_4 = String(cliente) + "/" + String(lugar) + "/" + sensor4;
      const char* topico_sensor_4_char = topico_sensor_4.c_str();
      const char* datos_sensor_4_char = datos_sensor_4.c_str();
      send_4_mqtt = client.publish(topico_sensor_4_char, datos_sensor_4_char);
      //Serial.println(datos_sensor_4_char);
    }
    // Creamos topico N°5 y su valor rescatado de variable global

    if (datos_sensor_5.length() != 0) {
      String topico_sensor_5 = String(cliente) + "/" + String(lugar) + "/" + sensor5;
      const char* topico_sensor_5_char = topico_sensor_5.c_str();
      const char* datos_sensor_5_char = datos_sensor_5.c_str();
      send_5_mqtt = client.publish(topico_sensor_5_char, datos_sensor_5_char);
      //Serial.println(datos_sensor_5_char);
    }

    // Evalúa si send_2_mqtt es verdadero y el temporizador no ha comenzado
    if (send_1_mqtt && send_2_mqtt && send_3_mqtt && send_4_mqtt && send_5_mqtt && !ledIsOn) {
      digitalWrite(led_envio, HIGH); // Enciende el LED
      ledIsOn = true; // Marca el LED como encendido
      led_start_time = millis(); // Registra el tiempo de inicio
      Serial.println("Envio MQTT exitoso");
    }

    // Si el temporizador ha comenzado y ha pasado el tiempo deseado
    if (ledIsOn && (millis() - led_start_time >= 500)) {
      digitalWrite(led_envio, LOW); // Apaga el LED
      ledIsOn = false; // Marca el LED como apagado
      send_1_mqtt = false;
      send_2_mqtt = false;
      send_3_mqtt = false;
      send_4_mqtt = false;
      send_5_mqtt = false;
    }
  }

  //************************ Trabajo 3 Envio a Endpoint ***********************//

  if (millis() - Task_3 > tiempo_envio_endpoint) {
    Task_3 = millis();
    send_post(datos_sensor_1);
    send_post(datos_sensor_2);
    send_post(datos_sensor_3);
    send_post(datos_sensor_4);
    send_post(datos_sensor_5);
  }


  //************************ Trabajo 4 Verifica si esta conectado al broker MQTT ***********************//
  if (millis() - Task_4 > 100) {
    Task_4 = millis();
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
}

void sensor_lectura(int number,  DynamicJsonDocument & jsonDoc) {

  jsonDoc["v1"] = 255.0;
  jsonDoc["v2"] = 255.0;
  jsonDoc["v3"] = 255.0;

  jsonDoc["v13"] = 255.0;
  jsonDoc["v12"] = 255.0;
  jsonDoc["v23"] = 255.0;

  jsonDoc["i1"] = 255.0;
  jsonDoc["i2"] = 255.0;
  jsonDoc["i3"] = 255.0;

  jsonDoc["p1"] = 255.0;
  jsonDoc["p2"] = 255.0;
  jsonDoc["p3"] = 255.0;

  jsonDoc["pa"] = 255.0;
  jsonDoc["fp"] = 255.0;
  jsonDoc["hz"] = 255.0;

  jsonDoc["sensor"] = number;
}

void send_post(String datos) {
  WiFiClient client;
  HTTPClient http; // Objeto para request

  http.begin(client, endpoint);

  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(datos);

  Serial.println(httpResponseCode);

  http.end();
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
