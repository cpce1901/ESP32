#include <esp_now.h>
#include <WiFi.h>
#include <Servo.h>


#define servoPin 26
#define IN3 32
#define IN4 33
#define ENA2 27
#define Bocina 23

Servo servoMotor;

unsigned long Task_1 = 0;
unsigned long Task_2 = 0;
unsigned long Task_3 = 0;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  bool vel;
  int servo;
  bool adel;
  bool atra;
  bool boci;

} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
}

void setup() {

  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode (ENA2, OUTPUT);
  pinMode (Bocina, OUTPUT);

  servoMotor.attach(servoPin);  // attaches the servo on ESP32 pin


  servoMotor.write(10);
  delay(1000);
  servoMotor.write(50);
  delay(1000);
  servoMotor.write(35);
  delay(1000);


}

void loop() {


  bool start = myData.vel;
  unsigned int escala = map(myData.servo, 0, 4095, 0, 100000);
  unsigned int direccion = map(escala, 0, 100000, 15, 55);
  bool front = myData.adel;
  bool back = myData.atra;
  bool bocina = myData.boci;

  //Serial.println(front);
  //Serial.println(back);
  //Serial.println(direccion);
  //Serial.println(start);
  Serial.println(bocina);


  if (millis() - Task_1 > 50 ) {
    Task_1 = millis();

    if (front and start) {
      adelante();
    }
    else if (back and start) {
      
      atras();
    }
    else {
      nada();
    }
  }

  if (millis() - Task_2 > 40 ) {
    Task_2 = millis();
    if (direccion > 35 and direccion < 45 ) {
      direccion = 33;
    }

    servoMotor.write(direccion);

  }

  if (millis() - Task_3 > 60 ) {
    Task_3 = millis();

    if (!bocina) {
      digitalWrite(Bocina, HIGH);
    }
    else {
      digitalWrite(Bocina, LOW);
    }
  }
}

void nada ()
{
  //Direccion motor B
  digitalWrite (IN3, LOW);
  digitalWrite (IN4, LOW);
}

void adelante () {
  //Direccion motor B
  digitalWrite (IN3, HIGH);
  digitalWrite (IN4, LOW);
}

void atras () {

  //Direccion motor B
  digitalWrite (IN3, LOW);
  digitalWrite (IN4, HIGH);

}
