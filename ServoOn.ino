#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>


// Datos de wifi
const char* ssid = "Lab4.0";
const char* password = "arduino123";

//Datos del broker
const char* mqtt_server = "broker.hivemq.com";
const char* id = "Esp32-Hands";
const char* user = "";
const char* codePass = "";
int puerto = 1883;
const char* topico = "uni/lub/int/servo/123456789";

const int pinServo = 12;

Servo miServo;
WiFiClient espClient;
PubSubClient client(espClient);

String _topic;
String _payload;

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
  miServo.attach(pinServo);

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

  if (_topic == "uni/lub/int/servo/123456789" && _payload == "1") {
    miServo.write(40);
  } else {
    miServo.write(85);
  }
Serial.println(_payload);
}
