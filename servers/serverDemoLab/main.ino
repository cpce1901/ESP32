#include <SPIFFS.h>
#include <DHT.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

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
AsyncWebSocket ws("/ws");

// Definición de tareas
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



  // Configurar WebSocket
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Iniciar servidor
  server.begin();

  // Crear tareas
  xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 1, &Task2, 1);
  xTaskCreatePinnedToCore(Task3code, "Task3", 10000, NULL, 1, &Task3, 1);

  Serial.println("Sistema inicializado. Esperando lecturas...");
}

void loop() {
  // El loop principal queda vacío
}

void Task1code(void *pvParameters) {
  for (;;) {
    distancia = medirDistancia(TRIGGER_PIN, ECHO_PIN);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
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
    String json = "{\"distancia\":" + String(distancia) + ",\"temperatura\":" + String(temperatura) + ",\"humedad\":" + String(humedad) + "}";
    ws.textAll(json);
    Serial.println(json);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
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
    return -1;
  }
  return duracion * 0.034 / 2.0;
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  }
}
