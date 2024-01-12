#include <ArduinoBLE.h>

#define led 2
#define touch 3

const char *touchServiceUuid = "564b008d-5e18-4e7e-9213-c44d52c89589";
const char *touchCharacteristicReadUuid = "564b008d-5e18-4e7e-9213-c44d52c89591";

const char *ledServiceUuid = "19b20000-e8f2-537e-4f6c-d104768a1218";
const char *ledCharacteristicWriteUuid = "19b20001-e8f2-537e-4f6c-d104768a1219";

BLEService touchService(touchServiceUuid);
BLEStringCharacteristic touchCharacteristicRead(touchCharacteristicReadUuid, BLENotify, 16);

BLEService ledService(ledServiceUuid);
BLEStringCharacteristic ledCharacteristicWrite(ledCharacteristicWriteUuid, BLEWrite, 1);


void setup() {
  pinMode(led, OUTPUT);
  pinMode(touch, INPUT);

  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  if (!BLE.begin()) {
    Serial.println("- Starting Bluetooth® Low Energy module failed!");
    while (1)
      ;
  }

  BLE.setDeviceName("Esp-32");
  BLE.setLocalName("Esp-32");


  // LED Service
  ledService.addCharacteristic(ledCharacteristicWrite);
  BLE.addService(ledService);


  // Touch Service
  touchService.addCharacteristic(touchCharacteristicRead);
  BLE.addService(touchService);

  BLE.advertise();

  Serial.println("Services defined! Now you can interact with both services.");
}

void loop() {

  BLEDevice central = BLE.central();
  if (central) {
    Serial.println("* Connected to central device!");
    Serial.print("* Device MAC address: ");
    Serial.println(central.address());
    Serial.println(" ");

    while (central.connected()) {
      // LED Characteristic Write
      if (ledCharacteristicWrite.written()) {
        String newLEDState = ledCharacteristicWrite.value();
        int ledValue = newLEDState.toInt();
        if (ledValue == 1) {
          digitalWrite(led, HIGH);
        } else {
          digitalWrite(led, LOW);
        }
        Serial.println("Received LED state: " + newLEDState);
      }

      int readTouch = digitalRead(touch);
      touchCharacteristicRead.setValue(String(readTouch));
    }

    Serial.println("* Disconnected from central device!");
  }
}
