#include <SPIFFS.h>
#include <DHT.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// Configuración WiFi
const char *ssid = "Lab4.0";
const char *password = "arduino123";

// Pines y configuración para el sensor ultrasónico US-100
#define TRIGGER_PIN 25
#define ECHO_PIN 26

// Pines y configuración para el sensor DHT22
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Variables para almacenar las lecturas de los sensores
float distancia = 0;
float temperatura = 0;
float humedad = 0;

// Crear objetos AsyncWebServer y AsyncWebSocket
AsyncWebServer server(80);
WebSocketsServer websockets(81);

// Definición de tareas
TaskHandle_t Task0;
TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

void setup() {
  Serial.begin(115200);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dht.begin();

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS...");
    return;
  }

  // Configurar rutas del servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/script.js", "application/javascript");
  });



  server.begin();

  websockets.begin();
  websockets.onEvent(webSocketEvent);

  // Crear tareas
  xTaskCreatePinnedToCore(Task0code, "Task0", 10000, NULL, 1, &Task0, 1);
  xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 1, &Task2, 1);
  xTaskCreatePinnedToCore(Task3code, "Task3", 10000, NULL, 1, &Task3, 1);

  Serial.println("Sistema inicializado. Esperando lecturas...");
}

void loop() {
  // El loop principal queda vacío
}

void Task0code(void *pvParameters) {
  for (;;) {
    websockets.loop();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void Task1code(void *pvParameters) {
  for (;;) {
    float medida = medirDistancia(TRIGGER_PIN, ECHO_PIN);
    if (medida < 9999.0) {  // Asumiendo que 9999.0 es nuestro valor de "fuera de rango"
      distancia = medida;
    } else {
      // Manejar el error, por ejemplo:
      Serial.println("Error al medir distancia");
      distancia = medida;
      // Podrías optar por mantener el último valor válido de distancia
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void Task2code(void *pvParameters) {
  for (;;) {
    humedad = dht.readHumidity();
    temperatura = dht.readTemperature();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void Task3code(void *pvParameters) {
  for (;;) {
    String json = "{\"distancia\":" + (distancia <= 9999.0 ? String(distancia) : "null") + ",\"temperatura\":" + (isnan(temperatura) ? "null" : String(temperatura)) + ",\"humedad\":" + (isnan(humedad) ? "null" : String(humedad)) + "}";
    websockets.broadcastTXT(json);
    Serial.println(json);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

float medirDistancia(int trigger, int echo) {
  digitalWrite(trigger, LOW);
  delayMicroseconds(5);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  long duracion = pulseIn(echo, HIGH, 30000);
  if (duracion == 0) {
    return 9999.0;  // Un valor grande que indique "fuera de rango"
  }
  return duracion * 0.034 / 2.0;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Desconectado!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Conectado desde %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
    case WStype_TEXT:
      handleWebSocketMessage(num, payload, length);
      break;
    case WStype_BIN:
      Serial.printf("[%u] Mensaje binario recibido\n", num);
      break;
  }
}

void handleWebSocketMessage(uint8_t num, uint8_t *payload, size_t length) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
}
