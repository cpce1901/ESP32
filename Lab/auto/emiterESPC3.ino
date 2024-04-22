#include <esp_now.h>
#include <WiFi.h>

#define ANALOG_1_X 2
#define ANALOG_1_Y 1

unsigned long Task_1 = 0;
unsigned long Task_2 = 0;
unsigned long Task_3 = 0;
unsigned long Task_4 = 0;
unsigned long Task_5 = 0;
unsigned long Task_6 = 0;
unsigned long Task_7 = 0;

// Direccion de recepcion
uint8_t broadcastAddress[] = { 0x34, 0xB4, 0x72, 0x4D, 0xBA, 0xD0 };  //MAC PLACA RECEPTORA

typedef struct struct_message {
  int vel;
  bool adel;
  bool atra;
  bool izqu;
  bool dere;

} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;


  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  if (millis() - Task_1 > 10) {
    Task_1 = millis();
    int read_input_y = analogRead(ANALOG_1_Y);
    // 0 = 600
    if (read_input_y > 1000) {
      // Hacia adelante
      int vel = map(read_input_y, 1000, 4000, 0, 255);
      myData.vel = vel;
      myData.adel = true;
      esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    } else if (read_input_y < 600) {
      // Hacia atras
      int vel = map(read_input_y, 600, 10, 0, 255);
      myData.vel = vel;
      myData.atra = true;
      esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    } else {
      myData.vel = 0;
      myData.adel = false;
      myData.atra = false;
      esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    }
  }

  if (millis() - Task_2 > 15) {
    Task_2 = millis();
    int read_input_x = analogRead(ANALOG_1_X);
    Serial.println(read_input_x);
    // 0 = 600
    if (read_input_x < 2000) {
      myData.izqu = true;
      esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    } else if (read_input_x > 2500) {
      myData.dere = true;
      esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    } else {
      myData.izqu = false;
      myData.dere = false;
      esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    }
  }
}
