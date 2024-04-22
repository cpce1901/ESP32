#include <esp_now.h>
#include <WiFi.h>

// Definición de pines para el control del vehículo
#define ENA1 0
#define ENA2 1
#define IN1 2
#define IN2 9
#define IN3 3
#define IN4 10

// Estructura para almacenar los datos recibidos
typedef struct {
  int vel;
  bool adel;
  bool atra;
  bool izqu;
  bool dere;
} ControlData;

ControlData controlData;
bool dataReceived = false;

// Función de callback para manejar los datos recibidos
void OnDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len) {
  // Copiar los datos recibidos a la estructura controlData
  memcpy(&controlData, incomingData, sizeof(controlData));
  dataReceived = true;
}

void setup() {
  Serial.begin(115200);

  // Inicialización de WiFi en modo estación
  WiFi.mode(WIFI_STA);

  // Inicialización de ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Registro de la función de callback para manejar los datos recibidos
  esp_now_register_recv_cb(OnDataRecv);

  // Configuración de los pines del vehículo
  pinMode(ENA1, OUTPUT);
  pinMode(ENA2, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  if (dataReceived) {
    // Procesar los datos recibidos
    controlVehicle(controlData);
    dataReceived = false; // Reiniciar la bandera de datos recibidos
  }
}

// Función para controlar el vehículo según los datos recibidos
void controlVehicle(ControlData data) {
  int vel = data.vel;
  bool adel = data.adel;
  bool atra = data.atra;
  bool izqu = data.izqu;
  bool dere = data.dere;

  // Imprimir los datos recibidos (para propósitos de depuración)
  Serial.print("Velocidad: ");
  Serial.print(vel);
  Serial.print(", Adelante: ");
  Serial.print(adel);
  Serial.print(", Atrás: ");
  Serial.print(atra);
  Serial.print(", Izquierda: ");
  Serial.print(izqu);
  Serial.print(", Derecha: ");
  Serial.println(dere);

  // Controlar el vehículo según las instrucciones recibidas
  if (adel) {
    if (dere) {
      adelanteDerecha(vel);
    } else if (izqu) {
      adelanteIzquierda(vel);
    } else {
      adelante(vel);
    }
  } else if (atra) {
    if (dere) {
      
      atrasDerecha(vel);
    } else if (izqu) {
      atrasIzquierda(vel);
    } else {
      atras(vel);
    }
  } else {
    detener();
  }
}

// Función para detener el vehículo
void detener() {
  digitalWrite(ENA1, LOW);
  digitalWrite(ENA2, LOW);
}

// Funciones para controlar el movimiento del vehículo en diferentes direcciones
void adelante(int vel) {
  digitalWrite(ENA1, HIGH);
  digitalWrite(ENA2, HIGH);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void adelanteDerecha(int vel) {
  digitalWrite(ENA1, LOW);
  digitalWrite(ENA2, HIGH);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void adelanteIzquierda(int vel) {
  digitalWrite(ENA1, HIGH);
  digitalWrite(ENA2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void atras(int vel) {
  digitalWrite(ENA1, HIGH);
  digitalWrite(ENA2, HIGH);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void atrasDerecha(int vel) {
  digitalWrite(ENA1, LOW);
  digitalWrite(ENA2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void atrasIzquierda(int vel) {
  digitalWrite(ENA1, HIGH);
  digitalWrite(ENA2, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}
