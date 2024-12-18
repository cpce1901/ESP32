#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

// Pin para el servo
const int pinServo = 18;

// Estructura de datos recibidos
typedef struct struct_message {
    int id;      // ID del dispositivo o acción
    int angle; // Ángulo para el servo
} struct_message;

struct_message myData;

Servo miServo; // Objeto del servo

// Callback para manejar los datos recibidos
void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes recibidos: ");
  Serial.println(len);

  Serial.print("ID del mensaje: ");
  Serial.println(myData.id);

  Serial.print("Ángulo recibido: ");
  Serial.println(myData.angle);

  // Mover el servo si el ID coincide
  if (myData.id == 1) { // ID 1 para controlar el servo
    int angle = myData.angle;
    if (angle >= 0 && angle <= 180) {
      miServo.write(angle);
      Serial.print("Servo movido a: ");
      Serial.println(angle);
    } else {
      Serial.println("Ángulo fuera de rango (0-180).");
    }
  }
}

void setup() {
  // Configurar monitor serie
  Serial.begin(115200);

  // Configurar el ESP32 como estación
  WiFi.mode(WIFI_STA);

  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  // Registrar el callback para recibir datos
  esp_now_register_recv_cb(OnDataRecv);

  // Configurar el pin del servo
  miServo.attach(pinServo);
  miServo.write(65); // Posición inicial del servo
  Serial.println("Servo inicializado a 90°.");
}

void loop() {
  // No se necesita lógica en el loop principal
}
