#include <esp_now.h>
#include <WiFi.h>

unsigned long Task_1 = 0;
unsigned long Task_2 = 0;

// Dirección MAC del receptor
uint8_t broadcastAddress[] = { 0x44, 0x17, 0x93, 0xFD, 0x6F, 0x30 };

// Pin del potenciómetro
#define pot 34  // Asegúrate de que este pin admite entrada analógica

// Estructura de datos a enviar
typedef struct struct_message {
  int id;     // ID del dispositivo o acción
  int angle;  // Ángulo para el servo (entero)
} struct_message;

struct_message myData;

// Información del peer
esp_now_peer_info_t peerInfo;

// Callback para notificar el estado del envío
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nEstado del último paquete enviado:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Éxito en la entrega" : "Fallo en la entrega");
}

void setup() {
  // Inicializa el monitor serie
  Serial.begin(115200);

  // Configura el ESP32 como estación Wi-Fi
  WiFi.mode(WIFI_STA);

  // Inicializa ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  // Registra el callback para el estado del envío
  esp_now_register_send_cb(OnDataSent);

  // Configura el peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Añade el peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error al añadir peer");
    return;
  }
}

void loop() {

  if (millis() - Task_1 > 90) {
    Task_1 = millis();
    int potValue = analogRead(pot);
    int angle;
    if (potValue <= 2047) {
      angle = map(potValue, 0, 2047, 15, 65);
    } else {
      angle = map(potValue, 2048, 4095, 65, 115);
    }

    myData.id = 1;
    myData.angle = angle;
    Serial.println(angle);
  }

  if (millis() - Task_2 > 30) {
    Task_2 = millis();
    envio();
  }
}

void envio() {
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    //Serial.println("Sent with success");
    //Serial.println(myData.vel);
    //Serial.println(myData.adel);
    //Serial.println(myData.atra);
    //Serial.println(myData.boci);
    //Serial.println(myData.servo);
  } else {
    //Serial.println("Error sending the data");
  }
}
