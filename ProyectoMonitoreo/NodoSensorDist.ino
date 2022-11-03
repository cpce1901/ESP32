#include <WiFi.h>
#include <esp_wifi.h>
#include <PubSubClient.h>

//************************Datos de Wifi***********************//
const char* ssid = "Jessica2.4";
const char* password = "167832873";

//************************Datos de Brocker***********************//

const char* mqtt_server = "node02.myqtthub.com";
const char* id = "ESP2";
const char* user = "cpce1901";
const char* codePass = "cpce.1901";
int puerto = 1883;
const char* topicod = "machali1231/escritorio/sd1";

//************************Manejo de tiempos***********************//

long lastMsg = 0;
unsigned long Task_1 = 0;
unsigned long Task_2 = 0;
unsigned long Task_3 = 0;
int tiempo_envio = 60000; //ms

//************************Creando objetos***********************//

WiFiClient espClient;
PubSubClient client(espClient);

//************************Variables de trabajo***********************//

#define RXD2 23
#define TXD2 22
String dato = "";
String envio = "";

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

//************************ Reconect ***********************//

void reconnect() {
  while (!client.connected()) {
    Serial.println("Intentando concetar con servidor MQTT...");
    if (client.connect(id, user, codePass)) {
      Serial.println("connectado");
    } else {
      Serial.print("Falla, Estado: ");
      Serial.print(client.state());
      Serial.println("Intentando en 5 segundos..");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); //SERIAL_8N1 8-bit No parity 1 Stop bit
  setup_wifi();
  client.setServer(mqtt_server, puerto);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - Task_1 > 100 ) {
    Task_1 = millis();

    if (Serial2.available() > 0) {
      String leer = Serial2.readStringUntil('\n');
      if (leer.startsWith("d:")) {
        for (int i = 0; i < leer.length(); i++) {
          if (isDigit(leer[i])) {
            dato += (String(leer[i]));
          }
        }
        if (millis() - Task_2 > 130 ) {
          Task_2 = millis();
          envio = dato;
        }
        dato = "";
      }
    }
  }

  long now = millis();
  if (now - lastMsg > tiempo_envio) {
    lastMsg = now;

    char out_d[10];
    envio.toCharArray(out_d, 10);

    Serial.println(out_d);

    client.publish(topicod, out_d);
  }
}
