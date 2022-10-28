#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//************************Datos de Wifi***********************//
const char* ssid = "xxxxxxx";
const char* password = "xxxxxxxxx";

//************************Datos de Brocker***********************//

const char* mqtt_server = "xxxxxxxxx"; 
const char* Id = "xxxxxxxx";
const char* User = "xxxxxxxx";
const char* CodePass = "xxxxxxxx";


WiFiClient espClient;
PubSubClient client(espClient);
String _topic;
String _payload;

//************************Puertos de S/E ***********************//

bool dato = LOW;
bool dato2 = LOW;
bool datoAnterior = LOW;
bool datoAnterior2 = LOW;
float SenTemp;
#define Button_1 D1
#define Button_2 D2
#define Led1 D3
#define Led2 D2

//************************ Conexion Wifi ***********************//

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("OK");
}

//************************ Reconect ***********************//

void reconnect() {
  while (!client.connected()) {
    Serial.println("Intentando concetar con servidor MQTT...");
    if (client.connect(Id, User, CodePass)) {
      Serial.println("connectado");
      client.subscribe("Led1");
      client.subscribe("Led2");

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

  pinMode(SenTemp, INPUT);
  pinMode(Button_1, INPUT);
  pinMode(Button_2, INPUT);
  pinMode(Led1, OUTPUT);
  pinMode(Led2, OUTPUT);


}

//************************ Loop ***********************//

void loop() {

  //************************ COMIENZO Flanco Motor 1 ***********************//

  dato = digitalRead(Button_1);
  if (dato != datoAnterior)
  {
    client.publish("Button_1", String(dato).c_str());
  }
  datoAnterior = dato;
  delayMicroseconds(5000);

//************************ COMIENZO Flanco Motor 2 ***********************//

  dato2 = digitalRead(Button_2);
  if (dato2 != datoAnterior2)
  {
    client.publish("Button_2", String(dato2).c_str());
  }
  datoAnterior2 = dato2;
  delayMicroseconds(5000);
  
  //************************ FIN Flanco Motor 1 - 2 ***********************//

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.println(_topic);
  Serial.println(_payload);

  if (_topic == "Led1" && _payload == "On") {
    digitalWrite(Led1, HIGH);
  }
  else if (_topic == "Led1" && _payload == "Off") {
    digitalWrite(Led1, LOW);
  }
  else if (_topic == "Led2" && _payload == "On") {
    digitalWrite(Led2, HIGH);
  }
  else if (_topic == "Led2" && _payload == "Off") {
    digitalWrite(Led2, LOW);
  }
delay(1000);


}
