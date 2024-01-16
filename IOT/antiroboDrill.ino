#include <WiFi.h>
#include <esp_wifi.h>
#include <PubSubClient.h>

#define out 3

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


String text = "";
char _topic[50];  // Adjust the size as needed
String _payload;


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
      client.subscribe(topico1);
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
  strcpy(_topic, topic);
  _payload = conc_payload_;
}


void setup() {
  pinMode(out, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, puerto);
  client.setCallback(callback);
  delay(3000);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  if (_payload == "1") {
    digitalWrite(out, HIGH);
    Serial.println(_payload);
    delay(10000);
  } else if (_payload == "0") {
    digitalWrite(out, LOW);
    Serial.println(_payload);
    delay(3000);
  }
}
