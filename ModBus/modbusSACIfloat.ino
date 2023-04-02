#include <ModbusMaster.h>

//************************ Definimos pines puerto Serie2 ***********************//
#define RXD2 16
#define TXD2 17

// Crear un objeto ModbusMaster
ModbusMaster node;

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node.begin(1, Serial2);
}

void loop() {

  float v1 = readData(6, 2);
  float v2 = readData(8, 2);
  float v3 = readData(10, 2);

  float v13 = readData(16, 2);
  float v12 = readData(12, 2);
  float v23 = readData(14, 2);

  float i1 = readData(18, 2);
  float i2 = readData(20, 2);
  float i3 = readData(22, 2);

  float p1 = readData(26, 2);
  float p2 = readData(28, 2);
  float p3 = readData(30, 2);

  float pa = readData(32, 2);
  float fp = readData(56, 2);
  float hz = readData(58, 2);

  Serial.print(v1);
  Serial.print(" - ");
  Serial.print(v2);
  Serial.print(" - ");
  Serial.print(v3);
  Serial.print(" - ");
  Serial.print(v13);
  Serial.print(" - ");
  Serial.print(v12);
  Serial.print(" - ");
  Serial.print(v23);
  Serial.print(" - ");
  Serial.print(i1);
  Serial.print(" - ");
  Serial.print(i2);
  Serial.print(" - ");
  Serial.print(i3);
  Serial.print(" - ");
  Serial.print(p1);
  Serial.print(" - ");
  Serial.print(p2);
  Serial.print(" - ");
  Serial.print(p3);
  Serial.print(" - ");
  Serial.print(pa);
  Serial.print(" - ");
  Serial.print(fp);
  Serial.print(" - ");
  Serial.println(hz);  



  delay(2000);
}

float readData(int registerRead, int numberRegister) {

  uint8_t result;

  result = node.readHoldingRegisters(registerRead, numberRegister);

  if (result == node.ku8MBSuccess) {

    uint16_t a, b;
    uint32_t c;

    a = node.getResponseBuffer(0);
    b = node.getResponseBuffer(1);

    float value = modbusToFloat(a, b);

    //Serial.print("Valor float leído: ");
    //Serial.println(value);
    return value;

  } else {
    Serial.print("Error al leer el registro: ");
    Serial.println(result);

  }

}

float modbusToFloat(uint16_t val1, uint16_t val2) {

  union {
    uint32_t i;
    float f;
  } u;

  u.i = ((uint32_t)val1 << 16) | val2; // Combina los dos registros Modbus en un valor de 32 bits

  return u.f; // Retorna el valor de punto flotante resultante
}




