/////////////////////////////////////////ENVIA/////////////////////////////////
#include <esp_now.h> // Libreria para comunicacion ESP-NOW
#include <esp_wifi.h> // Libreria para modificacion de comunicacion Wifi
#include <WiFi.h> // Libreria para modificacion de comunicacion Wifi
#include <DHT.h> //Libreria para uso de DHT sensor
#include <SPI.h> // Libreria para uso de de pantalla TFT por comunicacion SPI
#include <TFT_eSPI.h> // Libreria para uso de de pantalla TFT por comunicacion SPI
#include <ESP32Time.h> // Libreria para trabajar con hora

String nombreDispositivo = "ESP NOW"; // Titulo de dispositivo
int stado = 3; // Variable para reconocer el estadop del dispositivo 1: CONECTADO - 2: DESCONECTADO - 3: INICIANDO...

//Variables de trabajo durante el tiempo
unsigned long Task_1 = 0; 
unsigned long Task_2 = 0; 
unsigned long Task_3 = 0;

#define ID 1
//#define ID 2
//#define ID 3

#define color_Temp 0xFB8B
#define color_Hum 0x6F3F
#define color_Proc 0x54DF
#define color_cont 0xBE17
#define FONDO 0xDE99
#define NEGRO 0x0000
#define BLANCO 0xFFFF
#define Cabeza 0x09B2
#define ON 0x47E7
#define OFF 0xF820
#define ST 0xFFC5

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
DHT dht(13, DHT22 ); //SENSOR DE TEMP Y HUMED
ESP32Time rtc; //Creamos objeto reloj

uint8_t broadcastAddress[] = {0xE0, 0xE2, 0xE6, 0x0C, 0x51, 0x34}; //MAC PLACA RECEPTORA

// Estructura de datos para envio
typedef struct struct_message {
  int id;     // ID de placa envia
  float x;    // x = t
  float y;    // y = h
  float z;    // z = pr
} struct_message;


// Creamos la estructura de datos llamada myData
struct_message myData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// callback cuando el envio sea exitoso
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\n Estado del ultimo paquete enviado:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Entrega hecha" : "Entrega fallida");
  if (status == ESP_NOW_SEND_SUCCESS) {
    stado = 1;
  }
  else {
    stado = 2;
  }
}

void inicio() {

  tft.fillScreen(FONDO); // Color FONDO

  tft.drawRect(0, 0, 480, 50, NEGRO);
  tft.fillRect(1, 1, 478, 48, Cabeza);
  tft.setTextColor(BLANCO, Cabeza);
  tft.drawString(nombreDispositivo, 10, 17, 4);
  tft.drawString("ID :", 435, 10, 2);
  tft.drawString(String(ID), 460, 10, 2);


  tft.drawRect(25, 70, 200, 100, color_Temp);
  tft.drawRect(24, 69, 202, 102, color_Temp);
  tft.fillRect(26, 71, 198, 98, BLANCO);
  tft.setTextColor(color_Temp, BLANCO);
  tft.drawCentreString("Temperatura", 125, 75, 4);
  tft.drawCentreString("o", 185, 120, 1);
  tft.setTextColor(color_Temp, BLANCO);
  tft.setCursor(185, 130);
  tft.setTextFont(4);
  tft.println("C");


  tft.drawRect(25, 200, 200, 100, color_Hum);
  tft.drawRect(24, 199, 202, 102, color_Hum);
  tft.fillRect(26, 201, 198, 98, BLANCO);
  tft.setTextColor(color_Hum);
  tft.drawCentreString("Humedad", 125, 205, 4);
  tft.setTextColor(color_Hum, BLANCO);
  tft.setCursor(183, 255);
  tft.setTextFont(4);
  tft.println("%");

  tft.drawRect(255, 120, 200, 120, color_Proc);
  tft.drawRect(254, 119, 202, 122, color_Proc);
  tft.fillRect(256, 121, 198, 118, BLANCO);
  tft.setTextColor(color_Proc, BLANCO);
  tft.drawCentreString("Punto Rocio", 355, 130, 4);
  tft.drawCentreString("o", 420, 180, 1);
  tft.setTextColor(color_Proc, BLANCO);
  tft.setCursor(420, 190);
  tft.setTextFont(4);
  tft.println("C");
}

void fecha() {
  String relog = rtc.getTime();
  String dia = String(rtc.getDay());
  String mes = String(rtc.getMonth());
  String ano = String(rtc.getYear());
  String number = String('0');

  if (dia.length() == 1) {
    dia = number + dia;
  }

  if (mes.length() == 1) {
    mes = number + mes;
  }

  String fecha = (dia + "/" + mes + "/" + ano);
  tft.setTextColor(NEGRO, FONDO);
  tft.drawString(relog, 260, 260, 6);
  tft.setTextColor(BLANCO, Cabeza);
  tft.drawString(fecha, 390, 30, 2);
}

float medidas() {
  float h = dht.readHumidity(); //Leemos la Humedad
  float t = dht.readTemperature(); //Leemos la temperatura en grados Celsius
  float raiz = 1.25e-1;
  float base = h / 100.000;
  double pr_1 = pow(base, raiz);
  float pr_2 = pr_1 * (112.000 + t);
  float pr_3 = pr_2 - 112.000; // Calculo punto rocio
  int posX = 300;

  // variables para guardar cadena
  char out_t[8];
  char out_h[8];
  char out_pr[8];
  // Conversiones
  dtostrf(t, 4, 2, out_t);
  dtostrf(h, 4, 2, out_h);
  dtostrf(pr_3, 4, 2, out_pr);
  // Condicional de formato

  if (String(out_pr[1]) != ".") {
    posX = 270;
  }
  tft.setTextColor(color_Temp, BLANCO);
  tft.setTextSize(1);
  tft.drawString(out_t, 30, 110, 7);

  tft.setTextColor(color_Hum, BLANCO);
  tft.setTextSize(1);
  tft.drawString(out_h, 30, 235, 7);

  tft.fillRect(265, 166, 100, 55, BLANCO);
  tft.setTextColor(color_Proc, BLANCO);
  tft.setTextSize(1);
  tft.drawString(out_pr, posX, 170, 7);



  myData.id = ID;
  myData.x = t;
  myData.y = h;
  myData.z =  pr_3;
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);
  dht.begin();
  rtc.setTime(0, 12, 0, 3, 9, 2022); // ss - mm -hh - dd - mm - aa
  pantalla_negra();
  delay(1000);
  inicio();
  delay(100);
  medidas();
  delay(100);
  //fecha(); //Comentamos para que no aparezca fecha ni hora
  delay(100);
  estado();
  delay(100);
}

void loop() {

  if (millis() - Task_1 > 5000 ) {
    Task_1 = millis();
    medidas();
  }

  if (millis() - Task_2 > 1000 ) {     //Tomamos hora
    Task_2 = millis();
    //fecha(); //Comentamos para que no aparezca fecha ni hora

  }

  if (millis() - Task_3 > 10000 ) {
    Task_3 = millis();
    medidas();
    estado();

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    if (result == ESP_OK) {
      Serial.println(myData.x);
      Serial.println(myData.y);
      Serial.println(myData.z);
      Serial.println();
      Serial.println("Envio OK");
    }
    else {
      Serial.println("Error al enviar");
    }
  }
}

void estado() {

  if (stado == 1) {
    tft.fillCircle(270, 81, 11, ON);
    tft.drawCircle(270, 81, 11, NEGRO);
    tft.fillRect(290, 70, 200, 25, FONDO);
    tft.setTextColor(NEGRO, FONDO);
    tft.drawString("Conectado", 290, 72, 4);
  }
  else if (stado == 2) {
    tft.fillCircle(270, 81, 11, OFF);
    tft.drawCircle(270, 81, 11, NEGRO);
    tft.setTextColor(NEGRO, FONDO);
    tft.drawString("Desconectado", 290, 72, 4);
  }

  else if (stado == 3) {
    tft.fillCircle(270, 81, 11, ST);
    tft.drawCircle(270, 81, 11, NEGRO);
    tft.setTextColor(NEGRO, FONDO);
    tft.drawString("Iniciando...", 290, 72, 4);
  }

  else {
    tft.fillCircle(270, 81, 11, BLANCO);
    tft.drawCircle(270, 81, 11, NEGRO);
    tft.fillRect(280, 70, 200, 25, FONDO);
    tft.setTextColor(NEGRO, FONDO);
    tft.drawString("ERROR", 290, 72, 4);

  }
}

void pantalla_negra() {
  tft.fillScreen(BLANCO); // Color FONDO
  tft.fillRect(100, 10, 280, 280, NEGRO);

  // Iniciamos en modo AP para utilizar Long Range esta red no sera visible.
  WiFi.mode( WIFI_AP_STA );//for AP mode

  int a = esp_wifi_set_protocol( WIFI_IF_AP, WIFI_PROTOCOL_LR );
  if (a == 0)
  {
    Serial.println(" ");
    Serial.print("Error = ");
    Serial.print(a);
    Serial.println(" , Mode LR OK!");
    tft.setTextColor(BLANCO, NEGRO);
    tft.drawString("Mode LR OK...!", 150, 20, 1);

  }
  else//if some error in LR config
  {
    Serial.println(" ");
    Serial.print("Error = ");
    Serial.print(a);
    Serial.println(" , Error in Mode LR!");
    tft.setTextColor(BLANCO, NEGRO);
    tft.drawString("Mode LR ERROR...!", 150, 20, 1);
  }
  delay(1000);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicio ESP-NOW");
    tft.setTextColor(BLANCO, NEGRO);
    tft.drawString("Error inicio ESP-NOW...", 150, 40, 1);
    return;
  }
  tft.setTextColor(BLANCO, NEGRO);
  tft.drawString("ESP-NOW OK...", 150, 40, 1);
  Serial.println("ESP-NOW ok");

  esp_now_register_send_cb(OnDataSent); //Datos de prueba

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Falla al agregar un peer");
    return;
  }

  delay(1000);

  String mac = WiFi.macAddress();
  tft.setTextColor(BLANCO, NEGRO);
  tft.drawString("Tu dirección MAC es: ", 150, 60, 1);
  tft.drawString(mac, 150, 80, 1);

  delay(1000);

  tft.setTextColor(BLANCO, NEGRO);
  tft.drawString("Realizado por Claudio Carreno", 150, 120, 1);
  tft.drawString("+56 9 63044658", 150, 140, 1);
  delay(3000);

}
