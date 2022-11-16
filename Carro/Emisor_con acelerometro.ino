#include <Wire.h>
#include <LSM303.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>

LSM303 compass;

char report[80];
String dato = "";

uint8_t broadcastAddress[] = {0xE0, 0xE2, 0xE6, 0x0C, 0x51, 0x34}; //MAC PLACA RECEPTORA

// Estructura de datos para envio
typedef struct struct_message {
  int direccion;     // ID de placa envia
} struct_message;

// Creamos la estructura de datos llamada myData
struct_message myData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// callback cuando el envio sea exitoso
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\n Estado del ultimo paquete enviado:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Entrega hecha" : "Entrega fallida");
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  compass.init();
  compass.enableDefault();
  WiFi.mode( WIFI_AP_STA );//for AP mode
  int a = esp_wifi_set_protocol( WIFI_IF_AP, WIFI_PROTOCOL_LR );
  if (a == 0)
  {
    Serial.println(" ");
    Serial.print("Error = ");
    Serial.print(a);
    Serial.println(" , Mode LR OK!");
  }
  else//if some error in LR config
  {
    Serial.println(" ");
    Serial.print("Error = ");
    Serial.print(a);
    Serial.println(" , Error in Mode LR!");
  }
  delay(1000);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error inicio ESP-NOW");
    return;
  }
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
}

void loop()
{
  compass.read();

  snprintf(report, sizeof(report), "A: %6d %6d %6d M: %6d %6d %6d",
           compass.a.x, compass.a.y, compass.a.z,
           compass.m.x, compass.m.y, compass.m.z);


  for (int i = 2; i <= 8; i++) {
    dato += String(report[i]);
  }
  int numero = dato.toInt();

  int control = map(numero, -10000, 9000, -100, 100);

  if (control > 100) {
    control = 100;
  }

  else if (control < -100) {
    control = -100;
  }

  else if (control < 4 and control > - 10) {
    control = 0;
  }

  myData.direccion = control; // Dato a enviar
  
  //Serial.println(control); // Este sera el dato a enviar

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.print(control); Serial.print(" : "); Serial.println(myData.direccion);
  }
  else {
    Serial.println("Error al enviar");
  }

  dato = "";
  delay(200);
}
