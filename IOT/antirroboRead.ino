#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <PubSubClient.h>
#include "BluetoothSerial.h"

#define RXD2 16
#define TXD2 17

#define out 2

//************************Datos de Wifi***********************//
const char* ssid = "";
const char* password = "";


//************************Datos de Brocker***********************//

const char* mqtt_server = "";
const char* id = "";
const char* user = "";
const char* codePass = "";
int puerto = ;
const char* topico1 = "";


String dataSerial2 = "";  // Datos recibidos en el puerto serie 2
int min_d = 300;          // Valor por defecto de distancia minimo
int max_d = 1800;         // Valor por defecto de distancia maxima
int distance = 0;
int distance_min = 0;
int distance_max = 0;
bool error = false;


// Variables globales de tiempo
unsigned long work_1 = 0;
unsigned long work_2 = 0;
unsigned long work_3 = 0;
unsigned long work_4 = 0;
unsigned long work_5 = 0;
unsigned long work_6 = 0;
unsigned long work_7 = 0;
unsigned long work_8 = 0;

//************************Creando objetos***********************//

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
  pinMode(out, OUTPUT);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);  
  setup_wifi();
  client.setServer(mqtt_server, puerto);
  delay(3000);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (Serial2.available()) {
    char c = Serial2.read();
    
    if (c == ':') {
      distance = processReceivedData();
      Serial.println(distance);
      dataSerial2 = "";
    } else {
      dataSerial2 += c;
    }
  }

  // Verificar el estado del rango, si esta dentro de el o no lo esta
  if (millis() - work_1 >= 10) {
    work_1 = millis();
    if (distance <= min_d || distance >= max_d) {
      digitalWrite(out, 0);  // Deshabilita el encendido
      send(0);
    } else {
      digitalWrite(out, 1);  // Habilita el encendido
      send(1);
    }
  }
}

void send(int status) {
  static int status_ante = 0;
  if (status != status_ante) {
    Serial.println(status);
    String cadena = String(status);
    const char* envio = cadena.c_str();
    client.publish(topico1, envio);
  }
  status_ante = status;
}

int processReceivedData() {
  int endPos = dataSerial2.indexOf("mm");
  if (endPos != -1) {
    String numericValues = dataSerial2.substring(1, endPos);
    return numericValues.toInt();
  }
  return 0;
}

void listFilles() {
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.print("ARCHIVO: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }
  root.close();
  file.close();
}
