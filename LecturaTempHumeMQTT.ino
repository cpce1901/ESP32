#include <WiFi.h>
#include <esp_wifi.h>
#include <PubSubClient.h>
#include "Adafruit_SHT31.h"
#include <Wire.h>

//************************Datos de Wifi***********************//
const char* ssid = "";
const char* password = "";

//************************Datos de Brocker***********************//

const char* mqtt_server = "";
const char* id = "";
const char* user = "";
const char* codePass = "";
int puerto = 1883;
const char* topicot = "";
const char* topicoh = "";


//************************Manejo de tiempos***********************//

long lastMsg = 0;
unsigned long Task_1 = 0;
int tiempo_envio = 60000; //ms

//************************Variables globales***********************//

float t = 0;
float h = 0;

//************************Filtros de medida***********************//

float filtro1 = 0;
float filtro2 = 0;

//************************Creando objetos***********************//

//SHT311 definiciones
#define SDA 21
#define SCL 22

Adafruit_SHT31 sht31 = Adafruit_SHT31();

void startSHT() {
  if (! sht31.begin(0x44)) {
    Serial.println("SHT31 No encontrado....");
    while (1)
      delay(1);
  }
}

WiFiClient espClient;
PubSubClient client(espClient);

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
      Serial.println("connectado");
    } else {
      Serial.print("Falla, Estado: ");
      Serial.print(client.state());
      Serial.println("Intentando en 5 segundos..");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, puerto);
  startSHT();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - Task_1 > 10) {      //Lectura de sensor cada 500ms
    Task_1 = millis();
    t = sht31.readTemperature();
    h = sht31.readHumidity();
    // normalizacion
    filtro1 = filtro1 + (t - filtro1) / 20.0;
    filtro2 = filtro2 + (h - filtro2) / 60.0;  

  }

  long now = millis();
  if (now - lastMsg > tiempo_envio) {
    lastMsg = now;
    
    // variables para guardar cadena
    char out_t[8];
    char out_h[8];
    // Conversiones
    dtostrf(filtro1, 3, 1, out_t);
    dtostrf(filtro2, 3, 1, out_h);

    Serial.print(filtro1);
    Serial.print("\t");
    Serial.print(out_t);
    Serial.print("\t");    
    Serial.print(filtro2);
    Serial.print("\t");
    Serial.println(out_h);

    client.publish(topicot, out_t);
    client.publish(topicoh, out_h);
  }
}
