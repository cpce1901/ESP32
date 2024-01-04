#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <PubSubClient.h>

//*********************** Variable show in screem ***********************//

String text = "";
char _topic[50];  // Adjust the size as needed
String _payload;

//************************Datos de Wifi***********************//
const char* ssid = "Lab2.4";
const char* password = "arduino123";

//************************Datos de Brocker***********************//


const char* mqtt_server = "broker.hivemq.com";
const char* id = "Esp32/SSB/lab4.0/";
const char* user = "";
const char* codePass = "";
int puerto = 1883;
const char* topico = "ssb/lab4.0/sample/";

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R2, /* clock=*/18, /* data=*/23, /* CS=*/5, /* reset=*/22);  // ESP32F
WiFiClient espClient;
PubSubClient client(espClient);

//************************ Conexion Wifi ***********************//

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    text = "Conectando... " + String(ssid);
    showTextChar(text.c_str(), 0, 30);
    delay(500);
  }
  Serial.println("OK, Estamos conectado en WIFI");
  text = "Conectado!";
    showTextChar(text.c_str(), 0, 30);
}

//************************ Reconect ***********************//

void reconnect() {
  while (!client.connected()) {
    Serial.println("Intentando concetar con servidor MQTT...");
    text = "Conectando a Broker...";
    showTextChar(text.c_str(), 0, 30);
    if (client.connect(id, user, codePass)) {
      Serial.println("connectado");
      text = "Conectado a Broker!";
    showTextChar(text.c_str(), 0, 30);
      client.subscribe(topico);
    } else {
      Serial.print("Falla, Estado: ");
      Serial.print(client.state());
      Serial.println("Intentando en 5 segundos..");
      text = "Reconectando a Broker...";
    showTextChar(text.c_str(), 0, 30);
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  String conc_payload_;
  for (int i = 0; i < length; i++) {
    conc_payload_ += (char)payload[i];
  }
  strcpy(_topic, topic);
  _payload = conc_payload_;
}
void setup(void) {
  Serial.begin(115200);
  u8g2.begin();
  setup_wifi();
  client.setServer(mqtt_server, puerto);
  client.setCallback(callback);
}

void loop(void) {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.println(_topic);
  Serial.println(_payload);

  text = _payload;
  showTextChar(text.c_str(), 0, 30);
}

void showTextChar(const char* txt, int x, int y) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(x, y, txt);
  u8g2.sendBuffer();
}
