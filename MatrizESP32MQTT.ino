#include <WiFi.h>
#include <esp_wifi.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_H

#define MAX_DEVICES 8
#define CLK_PIN 18
#define DATA_PIN 23
#define CS_PIN 5

// Creamos Los objetos para funcionamiento
MD_Parola Display = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
WiFiClient espClient;
PubSubClient client(espClient);

// Datos de wifi

const char* ssid = "";
const char* password = "";

//Datos del broker

const char* mqtt_server = "";
const char* id = "Esp32-1";
const char* user = "";
const char* codePass = "";
int puerto = 1883;
const char* topico = "text/";

// Variables globales de funcionamiento
String Time, hour, minute, seconds;
String text = "";
String _topic;
String _payload;
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
int intensity = 15;  // Nivel de intensidad (0-15)

// Funcion de reconeccion para brocker

void reconnect() {
  while (!client.connected()) {
    Serial.println("Intentando concetar con servidor MQTT...");
    if (client.connect(id, user, codePass)) {
      Serial.println("connectado");
      client.subscribe(topico);
    } else {
      Serial.print("Falla, Estado: ");
      Serial.print(client.state());
      Serial.println("Intentando en 5 segundos..");
      delay(5000);
    }
  }
}

// Funcion para recepcion de mensajes desde brocker

void callback(char* topic, byte* payload, unsigned int length) {
  String conc_payload_;
  for (int i = 0; i < length; i++) {
    conc_payload_ += (char)payload[i];
  }
  _topic = topic;
  _payload = conc_payload_;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting.");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  timeClient.begin();
  timeClient.setTimeOffset(-4 * 3600);

  Display.begin();
  Display.setIntensity(intensity);  // Ajusta la intensidad de los LED
  delay(1000);
  Display.displayClear();

  client.setServer(mqtt_server, puerto);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  if (Display.displayAnimate()) {
    //P.displayClear();
    Display.displayText(text.c_str(), PA_CENTER, 50, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }

  Serial.println(_topic);
  Serial.println(_payload);

  obtainTime();
}

void obtainTime() {
  timeClient.update();

  currentMillis = millis();

  if (currentMillis - previousMillis > interval) {
    previousMillis = millis();
    client.loop();
    text = _payload;
    hour = timeClient.getHours();
    minute = timeClient.getMinutes();
    seconds = timeClient.getSeconds();

    Time = hour + ":" + minute;
    Serial.println(Time);
    //Display.setTextAlignment(PA_CENTER);
    //Display.print(text);
  }
}
