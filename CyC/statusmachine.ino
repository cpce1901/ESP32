#define RXD2 16
#define TXD2 17

int pulsador = 25;
int led = 27;
unsigned long work_1 = 0;
unsigned long work_2 = 0;
String receivedData = "";
bool bobinastatus = false;

void IRAM_ATTR encenderTodas() {
  
}

void setup() {
  Serial.begin(9600);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  pinMode(pulsador, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(pulsador), encenderTodas, RISING);
}

void loop() {
  if (millis() - work_1 >= 100) {
    if (Serial2.available()) {
      receivedData = Serial2.readStringUntil(':');
      if (receivedData.length() > 0) {
        int distancia = processReceivedData();
        int bobina = map(distancia, 0, 2000, 100, 0);        
        if (bobina <= 10) {
          bobinastatus = false;
        } else {
          bobinastatus = true;
        }
        receivedData = "";
      }
    }
    work_1 = millis();
  }

  if (millis() - work_2 >= 1000) {
    Serial.println(bobinastatus);
    work_2 = millis();
    if (bobinastatus) {
      Serial.println("Bobina cargada");
    } else {
      Serial.println("Bobina bajo el nivel 10%, Bobina acabada o extraida");
    }
  }
}

int processReceivedData() {
  int endPos = receivedData.indexOf("mm");
  if (endPos != -1) {
    String numericValues = receivedData.substring(1, endPos);
    return numericValues.toInt();
  }
  return 0;
}
