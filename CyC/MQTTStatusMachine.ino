#include <WiFi.h>
#include <esp_wifi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>  // Graphics and font library for ST7735 driver chip
#include <SPI.h>

//************************Definiciones de pines***********************//
#define variador 2

//************************Definiciones colores***********************//

#define TFT_GREY 0x5AEB  // Color del fondo de pantalla

//************************Datos de Wifi***********************//
const char* ssid = "TP-Link_7F77";
const char* password = "66298755";


//************************Datos de Brocker***********************//

const char* mqtt_server = "broker.hivemq.com";
const char* id = "pepitopagadoble";
const char* user = "pepitopagadoble";
const char* codePass = "";
int puerto = 1883;
const char* topico1 = "cyc/papelera/prueba/var1/pepitopagadoble";

//************************Manejo de tiempos***********************//

long lastMsg = 0;
unsigned long Task_1 = 0;
int tiempo_envio = 1000;  //ms

//************************Variables globales***********************//

int statusMachine = 0;
bool read_before = false;

//************************Creando objetos***********************//

WiFiClient espClient;
PubSubClient client(espClient);
TFT_eSPI tft = TFT_eSPI();

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
  Serial.begin(115200);

  tft.init(); 
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK); 
  tft.setTextColor(TFT_WHITE); 
  tft.setTextSize(2);  
  tft.setCursor(50, 50);
  tft.println("¡Hola, CyC!");

  pinMode(variador, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, puerto);
}

void loop() {
  

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - Task_1 > 500) {
    Task_1 = millis();

    bool read = digitalRead(variador);  // Leer el estado actual de la señal

    if (read != read_before) {
      if (read == HIGH) {
        if (statusMachine == 0) {
          statusMachine = 1;
        } else {
          statusMachine = 0;
        }
        Serial.println("Rising");
        send(0);
        send(statusMachine);
        show();
      } else {
        if (statusMachine == 0) {
          statusMachine = 1;
        } else {
          statusMachine = 0;
        }
        Serial.println("Falling");
        send(1);
        send(statusMachine);
        show();
      }

      read_before = read;  // Actualizar el estado anterior con el estado actual
    }
  }
}

void send(int status) {
  Serial.println(status);
  String cadena = String(status);
  const char* envio = cadena.c_str();
  client.publish(topico1, envio);
}

void show(){

  tft.fillScreen(TFT_BLACK);  
  tft.setCursor(120, 50);
  tft.print(statusMachine);
}
