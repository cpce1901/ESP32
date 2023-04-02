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


const char* serverName = "http://64.226.77.98/api-add";

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  node.begin(1, Serial2);

  // Inicializar la conexión WiFi
  WiFi.begin("DemoColegio2.4", "democolegio2.4");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red WiFi...");
  }

  Serial.println("Conexión WiFi establecida");
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

  char out_v1[8];
  char out_v2[8];
  char out_v3[8];

  char out_v12[8];
  char out_v13[8];
  char out_v23[8];

  char out_i1[8];
  char out_i2[8];
  char out_i3[8];

  char out_p1[8];
  char out_p2[8];
  char out_p3[8];

  char out_hz[8];
  char out_pa[16];
  char out_fp[8];

  // Conversiones
  dtostrf(v1, 4, 1, out_v1);
  dtostrf(v2, 4, 1, out_v2);
  dtostrf(v3, 4, 1, out_v3);

  dtostrf(v12, 4, 1, out_v12);
  dtostrf(v13, 4, 1, out_v13);
  dtostrf(v23, 4, 1, out_v23);

  dtostrf(i1, 4, 1, out_i1);
  dtostrf(i2, 4, 1, out_i2);
  dtostrf(i3, 4, 1, out_i3);

  dtostrf(p1, 4, 1, out_p1);
  dtostrf(p2, 4, 1, out_p2);
  dtostrf(p3, 4, 1, out_p3);

  dtostrf(hz, 4, 2, out_hz);
  dtostrf(pa, 8, 2, out_pa);
  dtostrf(fp, 3, 2, out_fp);

  DynamicJsonDocument jsonDoc(1024); // Tamaño máximo del objeto JSON
  jsonDoc["sensor"] = 1;
  jsonDoc["v1"] = out_v1;
  jsonDoc["v2"] = out_v2;
  jsonDoc["v3"] = out_v3;

  jsonDoc["v13"] = out_v13;
  jsonDoc["v12"] = out_v23;
  jsonDoc["v23"] = out_v23;

  jsonDoc["i1"] = out_i1;
  jsonDoc["i2"] = out_i2;
  jsonDoc["i3"] = out_i3;

  jsonDoc["p1"] = out_p1;
  jsonDoc["p2"] = out_p2;
  jsonDoc["p3"] = out_p3;

  jsonDoc["pa"] = out_pa;
  jsonDoc["fp"] = out_fp;
  jsonDoc["hz"] = out_hz;

  String postData;
  serializeJson(jsonDoc, postData);

  //Serial.print("Json enviado: ");
  //serializeJson(postData);

  if (WiFi.status() == WL_CONNECTED) {

    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName);

    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(postData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }

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



