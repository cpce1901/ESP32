#include <ModbusMaster.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

//************************ Definimos pines puerto Serie2 ***********************//
#define RXD2 16
#define TXD2 17

// Crear un objeto ModbusMaster
ModbusMaster node;


void setup() {

  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node.begin(1, Serial2);

  // Inicializar la conexión WiFi
  //WiFi.begin("DemoColegio2.4", "democolegio2.4");
  WiFi.begin("Jessica2.4", "167832873");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red WiFi...");
  }

  Serial.println("Conexión WiFi establecida");
}

void loop() {
  float total_kw = readTwoRegisterPower(0, 0.1);
  float v1 = readOneRegisterData(2, 0.01);  // Direccion de registro, multiplicador
  float i1 = readTwoRegisterPower(3, 1) * 0.001;
  float p1 = readTwoRegisterPower(5, 0.1);
  float hz = readOneRegisterData(11, 0.01);  // Direccion de registro, multiplicador

  Serial.print("Consumo: ");
  Serial.println(total_kw);

  Serial.print("Voltaje: ");
  Serial.println(v1);

  Serial.print("Corriente: ");
  Serial.println(i1);

  Serial.print("Potencia: ");
  Serial.println(p1);

  Serial.print("Frecuencia: ");
  Serial.println(hz);

  delay(5000);
}

float readOneRegisterData(int registerRead, float plus) {
  uint8_t result;
  result = node.readHoldingRegisters(registerRead, 1);

  if (result == node.ku8MBSuccess) {
    uint16_t a = node.getResponseBuffer(0);
    float value = a * plus;
    return value;
  }
}

float readTwoRegisterData(int registerRead, float plus) {
  uint8_t result;
  result = node.readHoldingRegisters(registerRead, 2);

  if (result == node.ku8MBSuccess) {
    uint16_t a, b;

    a = node.getResponseBuffer(0);
    b = node.getResponseBuffer(1);

    String c = String(a) + String(b);

    int c_int = c.toInt();

    float value = c_int * plus;
    return value;

  } else {
    Serial.print("Error al leer el registro: ");
    Serial.println(result);
    return 0;
  }
}

// Para leer debe rescatar 2 registros el primero multiplicarlo por el factor y el segundo asigna positivo o negativo
float readTwoRegisterPower(int registerRead, float plus) {
  uint8_t result;
  result = node.readHoldingRegisters(registerRead, 2);

  if (result == node.ku8MBSuccess) {
    int a, b;

    a = node.getResponseBuffer(0) * plus;
    b = node.getResponseBuffer(1);

    long c = (b << 16) | a;

    float value = c;
    return value;

  } else {
    Serial.print("Error al leer el registro: ");
    Serial.println(result);
    return 0;
  }
}
