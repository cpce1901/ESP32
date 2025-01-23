#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <SI7021.h>


// Configuración WiFi
const char* ssid = "";
const char* password = "";

// Configuración del broker MQTT
const char* mqtt_server = "";
const int mqtt_port = ;
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* mqtt_id = "";

// Tópicos MQTT
const char* topic_temp = "";
const char* topic_hum = "";

static const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
BLA BLA BLA
-----END CERTIFICATE-----
)EOF";

#define SDA 39
#define SCL 38

SI7021 sensor;
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Función para leer sensor
void handle_root() {
  int temp = sensor.getCelsiusHundredths();
  float temperature = temp / 100.0;

  int hum = sensor.getHumidityBasisPoints();
  float humidity = hum / 100.0;

  Serial.print("Temperatura: ");
  Serial.print(temperature);  // Mostrar un solo decimal
  Serial.println(" °C");

  Serial.print("Humedad: ");
  Serial.print(humidity);  // Mostrar un solo decimal
  Serial.println(" %");

  // Convertir valores a cadenas
  char tempStr[8];
  char humStr[8];
  dtostrf(temperature, 6, 2, tempStr);
  dtostrf(humidity, 6, 2, humStr);

  // Publicar datos en los tópicos MQTT
  client.publish(topic_temp, tempStr);
  client.publish(topic_hum, humStr);
}

// Función para conectarse al WiFi
void setupWiFi() {
  Serial.print("Conectando a WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConectado a WiFi");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// Función para conectarse al broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando a MQTT...");
    client.setKeepAlive(20);
    if (client.connect(mqtt_id, mqtt_user, mqtt_password)) {
      Serial.println("conectado");
      // Suscríbete a tus topics aquí
      client.subscribe("test/topic");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  sensor.begin(SDA, SCL);

  // Conexión WiFi
  setupWiFi();

  // Configurar certificado
  espClient.setCACert(ca_cert);
  client.setKeepAlive(20);
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Conectar al broker MQTT si no está conectado
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  handle_root();
  delay(1000);
}
