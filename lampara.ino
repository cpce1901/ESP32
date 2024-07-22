#include <WiFi.h>
#include <PubSubClient.h>
#include <WS2812FX.h>

#define LED_PIN 10
#define LED_COUNT 20

// Datos de wifi
const char* ssid = "Jessica2.4";
const char* password = "167832873";

// Datos del broker
const char* mqtt_server = "146.190.124.66";
const char* id = "Esp32-Hands";
const char* user = "tester";
const char* codePass = "tester";
int puerto = 1883;
const char* topico1 = "hands/";
const char* topico2 = "color/";

String _topic;
String _payload;

WiFiClient espClient;
PubSubClient client(espClient);
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void reconnect() {
  while (!client.connected()) {
    Serial.println("Intentando concetar con servidor MQTT...");
    if (client.connect(id, user, codePass)) {
      Serial.println("conectado");
      client.subscribe(topico1);
      client.subscribe(topico2);
    } else {
      Serial.print("Falla, Estado: ");
      Serial.print(client.state());
      Serial.println("Intentando en 5 segundos..");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String conc_payload_;
  for (int i = 0; i < length; i++) {
    conc_payload_ += (char)payload[i];
  }

  _topic = topic;
  _payload = conc_payload_;
}

uint32_t interpolateColor(uint8_t value) {
  // Blanco frío: 0xFFFFFF, Blanco cálido: 0xFFD700
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
  ws2812fx.setBrightness(30);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

  for (int i = 0; i <= 255; i++) {
    ws2812fx.setBrightness(i);
    ws2812fx.service();
    delay(10);
  }

  Serial.println("");
  Serial.println("LED ready.");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting.");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  client.setServer(mqtt_server, puerto);
  client.setCallback(callback);
  Serial.println("");
  Serial.println("MQTT Settings Ready.");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (_topic == "hands/") {
    ws2812fx.setBrightness(_payload.toInt());
    ws2812fx.service();
  } else if (_topic == "color/") {
    int value = _payload.toInt();
    if (value >= 0 && value <= 100) {
      uint32_t color = interpolateColor(value);
      ws2812fx.setColor(color);
      ws2812fx.service();
    }
  }

  Serial.println(_payload);
}
