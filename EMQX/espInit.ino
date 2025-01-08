#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

// Configuración del sensor DHT
#define DHTPIN 32      // Pin donde está conectado el DHT21
#define DHTTYPE DHT21  // Tipo de sensor: DHT21 (AM2301)

// Configuración WiFi
const char* ssid = "";
const char* password = "";

// Configuración del broker MQTT
const char* mqtt_server = "";
const int mqtt_port = ;
const char* mqtt_user = "";
const char* mqtt_password = "";

// Tópicos MQTT
const char* topic_temp = "";
const char* topic_hum = "";

static const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
BLA BLA BLA
-----END CERTIFICATE-----
)EOF";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// Inicialización del sensor DHT
DHT dht(DHTPIN, DHTTYPE);

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
    String clientId = "ESP32_lab_one";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
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

// Función setup
void setup() {
  Serial.begin(115200);

  // Inicialización del sensor DHT
  dht.begin();

  // Conexión WiFi
  setupWiFi();

  // Configurar certificado
  espClient.setCACert(ca_cert);


  client.setServer(mqtt_server, mqtt_port);
}

// Función loop
void loop() {
  // Conectar al broker MQTT si no está conectado
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // Leer datos del DHT21
  float temperature = dht.readTemperature();  // Temperatura en °C
  float humidity = dht.readHumidity();        // Humedad en %

  // Verificar si las lecturas son válidas
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error al leer el sensor DHT21");
    delay(2000);
    return;
  }

  // Convertir valores a cadenas
  char tempStr[8];
  char humStr[8];
  dtostrf(temperature, 6, 2, tempStr);  // Convertir float a string
  dtostrf(humidity, 6, 2, humStr);

  // Publicar datos en los tópicos MQTT
  client.publish(topic_temp, tempStr);
  client.publish(topic_hum, humStr);

  Serial.print("Enviado -> ");
  Serial.print("Temperatura: ");
  Serial.print(tempStr);
  Serial.print(" °C, Humedad: ");
  Serial.print(humStr);
  Serial.println(" %");

  delay(1000);  // Enviar datos cada 5 segundos
}
