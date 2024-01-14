#define RXD2 16
#define TXD2 17

// Variables globales de tiempo
unsigned long work_1 = 0;
unsigned long work_2 = 0;
unsigned long work_3 = 0;
unsigned long work_4 = 0;
unsigned long work_5 = 0;
unsigned long work_6 = 0;
unsigned long work_7 = 0;
unsigned long work_8 = 0;


void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
}

void loop() {

  if (Serial2.available()) {
    char c = Serial2.read();
    if (c == ':') {
      distance = processReceivedData();
      Serial.println(distance);
      dataSerial2 = "";
    } else {
      dataSerial2 += c;
    }
  }
}

int processReceivedData() {
  int endPos = dataSerial2.indexOf("mm");
  if (endPos != -1) {
    String numericValues = dataSerial2.substring(1, endPos);
    return numericValues.toInt();
  }
  return 0;
}
