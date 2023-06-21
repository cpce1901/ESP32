#include <WiFiClient.h> 
#include <HTTPClient.h>     
#include <ArduinoJson.h>
#include <WiFi.h>

// URL para ingresar datos
const char* serverName = "http://192.168.0.101:8000/lectures/api-add/";

#define run 17
#define statusM 16

// Numero de sensor
#define id 1

bool readRunBefore = false;
bool readStatusBefore = false;

int statusMachine = 0;
bool change = false;

unsigned long currentTime = 0;
int interval = 30;  // Intervalo de tiempo antes de envio sin actividad en segundos

void setup() {
  Serial.begin(115200);
  pinMode(run, INPUT);
  pinMode(statusM, INPUT);

  // Inicializar la conexión WiFi
  WiFi.begin("TP-Link_7F77", "66298755");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red WiFi...");
  }

  Serial.println("Conexión WiFi establecida");

  send(statusMachine);
}


void loop() {
  // Lectura de las entradas digitales de estado y de arranque
  bool readRun = digitalRead(run);
  bool readStatus = digitalRead(statusM);

  // Cuenta regresiva para envio de sin actividad
  static int cuenta = interval; 

  // Identificamos el rissing and falling de la lectura statusM
  if (readStatus != readStatusBefore) {
    if (readStatus == HIGH) {
      Serial.println("Status Rissing");
      change = true;
      statusMachine = 1;
    } else if (readStatus == LOW) {
      Serial.println("Status Falling");
      change = true;
      statusMachine = 0;
    }
  }

  if (readRun && change) {
    Serial.println("Enviar dato: Maquina encendida y ha habido un cambio");
    cuenta = interval;
    send(statusMachine);
  }
  // Accion cuando readRun este encendido y no haya un cambio en statusMachine
  else if (readRun && !change) {
    if (millis() - currentTime >= 1000) {
      currentTime = millis();
      cuenta--;
      Serial.print("Maquina encendida y no ha habido un cambio");
      Serial.print("cuenta regresiba:");
      Serial.println(cuenta);
      if (cuenta <= 0) {
        Serial.print("Enviar dato: Maquina encendida y no ha habido un cambio hace 30 seg.");
        cuenta = interval;
        send(statusMachine);
      }
    }
  }
  readStatusBefore = readStatus;
  change = false;
}

void send(int estado) {

  DynamicJsonDocument jsonDoc(512);
  jsonDoc["machine"] = id;
  jsonDoc["status"] = estado;

  String postData;
  serializeJson(jsonDoc, postData); 
  
  Serial.println(postData);

  if (WiFi.status() == WL_CONNECTED) {

    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName);

    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(postData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();

  } else {
    Serial.println("WiFi Disconnected");
    ESP.restart();
    delay(500);
  }
}
