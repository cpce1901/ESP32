#include <esp_now.h>
#include <WiFi.h>

#define adelante 5
#define atras 18
#define bocina 16
#define start 17
#define pot 34



unsigned long Task_1 = 0;
unsigned long Task_2 = 0;
unsigned long Task_3 = 0;
unsigned long Task_4 = 0;
unsigned long Task_5 = 0;
unsigned long Task_6 = 0;
unsigned long Task_7 = 0;

// Direccion de recepcion
uint8_t broadcastAddress[] = {0x30, 0xAE, 0xA4, 0x98, 0x23, 0x6C}; //MAC PLACA RECEPTORA

// Estructura de datos para enviar
typedef struct struct_message {
  bool vel;
  int servo;
  bool adel;
  bool atra;
  bool boci;
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup()
{
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

  pinMode(adelante, INPUT);
  pinMode(atras, INPUT);
  pinMode(bocina, INPUT);
  pinMode(start, INPUT);
  pinMode(pot, INPUT);
}

void loop() {

  if (millis() - Task_1 > 40 ) {
    Task_1 = millis();
    bool Lec_start = digitalRead(start);
    myData.vel = Lec_start;
    //Serial.print(Lec_start);

  }

  if (millis() - Task_2 > 100 ) {
    Task_2 = millis();
    bool Lec_adelante = digitalRead(adelante);
    myData.adel = Lec_adelante;
    //Serial.print(Lec_adelante);
  }

  if (millis() - Task_3 > 110 ) {
    Task_3 = millis();
    bool Lec_atras = digitalRead(atras);
    myData.atra = Lec_atras;
    //Serial.print(Lec_atras);
  }

  if (millis() - Task_4 > 120 ) {
    Task_4 = millis();
    bool Lec_bocina = digitalRead(bocina);
    myData.boci = Lec_bocina;
    //Serial.print(Lec_bocina);

  }

  if (millis() - Task_5 > 120 ) {
    Task_5 = millis();
    int envio_servo = analogRead(pot);
    //long int envio_servo = map(Lec_pot, 0, 4095, 0, 800000);
    myData.servo = envio_servo;   
    Serial.println(envio_servo);

  }

  if (millis() - Task_6 > 50   ) {
    Task_6 = millis();
    envio();
  }
}

void envio() {
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    //Serial.println("Sent with success");
    //Serial.println(myData.vel);
    //Serial.println(myData.adel);
    //Serial.println(myData.atra);
    //Serial.println(myData.boci);
    //Serial.println(myData.servo);
  }
  else {
    //Serial.println("Error sending the data");
  }

}
