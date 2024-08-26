#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2901.h>
#include <WS2812FX.h>

#define SERVICE_UUID "00001815-0000-1000-8000-00805F9B34FB"
#define SEND_DATA_CHAR_UUID "00002AE8-0000-1000-8000-00805F9B34FB"
#define INT_DATA_CHAR_UUID "00002AEA-0000-1000-8000-00805F9B34FB"
#define STRING_DATA_CHAR_UUID "00002A00-0000-1000-8000-00805F9B34FB"
#define FLOAT_DATA_CHAR_UUID "00002AF2-0000-1000-8000-00805F9B34FB"

#define LED_PIN 10
#define LED_COUNT 58

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

BLECharacteristic *pSendCharacteristic = NULL;
BLECharacteristic *pIntCharacteristic = NULL;
BLECharacteristic *pFloatCharacteristic = NULL;
BLECharacteristic *pStringCharacteristic = NULL;

BLEServer *pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

uint32_t value_send = 0;
int value_int = 10;
float value_float = 15.4;
String value_string = "Hello";

bool isRainbowMode = false;
int currentBrightness = 30;
int currentColorValue = 0;

TaskHandle_t Task1;
TaskHandle_t Task2;

uint32_t interpolateColor(uint8_t value) {
  uint8_t r1 = 0xFF, g1 = 0xFF, b1 = 0xFF;  // Blanco frío
  uint8_t r2 = 0xFF, g2 = 0xD7, b2 = 0x00;  // Blanco cálido
  uint8_t r = (r1 * (100 - value) + r2 * value) / 100;
  uint8_t g = (g1 * (100 - value) + g2 * value) / 100;
  uint8_t b = (b1 * (100 - value) + b2 * value) / 100;
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Conectado");
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    value_send = 0;
    Serial.println("Des-Conectado");
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    String rxValue = pCharacteristic->getValue().c_str();
    Serial.println("Nuevo valor recibido en característica:");
    Serial.println(pCharacteristic->getUUID().toString().c_str());
    Serial.print("Valor: ");
    Serial.println(rxValue.c_str());

    // Parse the received message
    if (rxValue.startsWith("{") && rxValue.endsWith("}")) {
      int colonPos = rxValue.indexOf(':');
      if (colonPos != -1) {
        String key = rxValue.substring(1, colonPos);
        int value = rxValue.substring(colonPos + 1, rxValue.length() - 1).toInt();

        if (key == "color") {
          if (value >= 0 && value <= 100) {
            currentColorValue = value;
            if (!isRainbowMode) {
              ws2812fx.setColor(interpolateColor(currentColorValue));
            }
          }
        } else if (key == "brillo") {
          if (value >= 0 && value <= 100) {
            currentBrightness = map(value, 0, 100, 0, 255);
            ws2812fx.setBrightness(currentBrightness);
          }
        } else if (key == "arco") {
          if (value == 1) {
            isRainbowMode = true;
            ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
            ws2812fx.setSpeed(1000); // Ajusta este valor para cambiar la velocidad del arcoíris
          } else if (value == 0) {
            isRainbowMode = false;
            ws2812fx.setMode(FX_MODE_STATIC);
            ws2812fx.setColor(interpolateColor(currentColorValue));
          }
        }
      }
    }

    // Update the string characteristic
    value_string = rxValue.c_str();
    pStringCharacteristic->setValue(value_string.c_str());
    pStringCharacteristic->notify();
    Serial.println("String Notify");
  }
};

void bluetoothTask(void * pvParameters) {
  for(;;) {
    if (deviceConnected) {
      pSendCharacteristic->setValue(String(value_send).c_str());
      pSendCharacteristic->notify();
      Serial.print(value_send);
      Serial.print(" | ");
      Serial.println(value_string);
      value_send++;
    }

    if (!deviceConnected && oldDeviceConnected) {
      delay(500);
      pServer->startAdvertising();
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected) {
      oldDeviceConnected = deviceConnected;
    }

    delay(500);
  }
}

void lightTask(void * pvParameters) {
  for(;;) {
    ws2812fx.service();
    delay(10);
  }
}

void setup() {
  Serial.begin(115200);

  ws2812fx.init();
  ws2812fx.setBrightness(currentBrightness);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(interpolateColor(currentColorValue));
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

  Serial.println("LED ready.");

  BLEDevice::init("ESP32-Test");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // CREAMOS CARACTERISTICAS
  pSendCharacteristic = pService->createCharacteristic(
    SEND_DATA_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);

  pIntCharacteristic = pService->createCharacteristic(
    INT_DATA_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);

  pFloatCharacteristic = pService->createCharacteristic(
    FLOAT_DATA_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);

  pStringCharacteristic = pService->createCharacteristic(
    STRING_DATA_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);

  pSendCharacteristic->addDescriptor(new BLE2902());
  pIntCharacteristic->addDescriptor(new BLE2902());
  pFloatCharacteristic->addDescriptor(new BLE2902());
  pStringCharacteristic->addDescriptor(new BLE2902());

  pStringCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

  // Crear tareas en diferentes núcleos
  xTaskCreatePinnedToCore(
    bluetoothTask,   /* Función de la tarea */
    "BluetoothTask", /* Nombre de la tarea */
    10000,           /* Tamaño del stack */
    NULL,            /* Parámetro de la tarea */
    1,               /* Prioridad de la tarea */
    &Task1,          /* Handle de la tarea */
    0);              /* Núcleo donde se ejecutará (0) */

  xTaskCreatePinnedToCore(
    lightTask,       /* Función de la tarea */
    "LightTask",     /* Nombre de la tarea */
    10000,           /* Tamaño del stack */
    NULL,            /* Parámetro de la tarea */
    1,               /* Prioridad de la tarea */
    &Task2,          /* Handle de la tarea */
    1);              /* Núcleo donde se ejecutará (1) */
}

void loop() {
  // El loop principal queda vacío ya que las tareas se manejan en sus propios núcleos
}
