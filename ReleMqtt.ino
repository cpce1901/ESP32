#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//************************Datos de Wifi***********************//
const char* ssid = "Nombre de wifi";
const char* password = "pasword wifi";

//************************Datos de Brocker***********************//

const char* mqtt_server = "ingresa tu servidor"; 
const char* Id = "Ingresa tu ID";
const char* User = "Ingresa tu usuario";
const char* CodePass = "Ingresa tu password";


WiFiClient espClient;
PubSubClient client(espClient);
String _topic;
String _payload;

//************************Puertos de S/E ***********************//

int salida = 0;

//************************ Conexion Wifi ***********************//

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("OK, la hicimos");
}

//************************ Reconect ***********************//

void reconnect() {
  while (!client.connected()) {
    Serial.println("Intentando concetar con servidor MQTT...");
    if (client.connect(Id, User, CodePass)) {
      Serial.println("connectado");
      client.subscribe("Led1");
      
    } else {
      Serial.print("Falla, Estado: ");
      Serial.print(client.state());
      Serial.println("Intentando en 5 segundos..");
      delay(5000);
    }
  }
}

//************************ Callback ***********************//

void callback (char* topic, byte* payload, unsigned int length) {
  String conc_payload_;
  for (int i = 0; i < length; i++) {
    conc_payload_ += (char)payload[i];
  }
  _topic = topic;
  _payload = conc_payload_;

}

//************************ SETUP ***********************//

void setup() {
  Serial.begin(115200);
  Serial.println("iniciando");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(salida, OUTPUT);

}

//************************ Loop ***********************//

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.println(_topic);
  Serial.println(_payload);

  if (_topic == "Led1" && _payload == "On") {
    digitalWrite(salida, HIGH);
  }
  else {
    digitalWrite(salida, LOW);
    }
  delay(1000);


}
