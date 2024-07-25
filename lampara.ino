#include <WiFi.h>
#include <PubSubClient.h>
#include <WS2812FX.h>

#define LED_PIN 10
#define LED_COUNT 58

// Datos de wifi
const char* ssid = "Lab4.0";
const char* password = "arduino123";

// Datos del broker
const char* mqtt_server = "146.190.124.66";
const char* id = "Esp32-Hands";
const char* user = "tester";
const char* codePass = "tester";
int puerto = 1883;
const char* topico1 = "hands/";
const char* topico2 = "color/";
const char* topico3 = "rainbow/";

String _topic;
String _payload;

WiFiClient espClient;
PubSubClient client(espClient);
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

bool isRainbowMode = false;
int currentBrightness = 30;
int currentColorValue = 0;

void reconnect() {
  while (!client.connected()) {
    Serial.println("Intentando conectar con servidor MQTT...");
    if (client.connect(id, user, codePass)) {
      Serial.println("Conectado");
      client.subscribe(topico1);
      client.subscribe(topico2);
      client.subscribe(topico3);
    } else {
      Serial.print("Falla, Estado: ");
      Serial.print(client.state());
      Serial.println(" Intentando en 5 segundos...");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String conc_payload_;
  for (int i = 0; i < length; i++) {
    conc_payload_ += (char)payload[i];
  }
  _topic = String(topic);
  _payload = conc_payload_;
}

uint32_t interpolateColor(uint8_t value) {
  uint8_t r1 = 0xFF, g1 = 0xFF, b1 = 0xFF; // Blanco frío
  uint8_t r2 = 0xFF, g2 = 0xD7, b2 = 0x00; // Blanco cálido
  uint8_t r = (r1 * (100 - value) + r2 * value) / 100;
  uint8_t g = (g1 * (100 - value) + g2 * value) / 100;
  uint8_t b = (b1 * (100 - value) + b2 * value) / 100;
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void setup() {
  Serial.begin(115200);
  ws2812fx.init();
  ws2812fx.setBrightness(currentBrightness);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(interpolateColor(currentColorValue));
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

  Serial.println("LED ready.");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting...");
  }
  Serial.println("WiFi connected.");

  client.setServer(mqtt_server, puerto);
  client.setCallback(callback);
  Serial.println("MQTT Settings Ready.");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (_topic == topico1) {
    currentBrightness = _payload.toInt();
    ws2812fx.setBrightness(currentBrightness);
  } else if (_topic == topico2 && !isRainbowMode) {
    currentColorValue = _payload.toInt();
    if (currentColorValue >= 0 && currentColorValue <= 100) {
      uint32_t color = interpolateColor(currentColorValue);
      ws2812fx.setColor(color);
    }
  } else if (_topic == topico3) {
    if (_payload == "on") {
      isRainbowMode = true;
      ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
    } else if (_payload == "off") {
      isRainbowMode = false;
      ws2812fx.setMode(FX_MODE_STATIC);
      ws2812fx.setColor(interpolateColor(currentColorValue));
    }
  }

  ws2812fx.service();
  _topic = "";
  _payload = "";
}
