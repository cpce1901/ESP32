
#define RXD2 23
#define TXD2 22

unsigned long Task_1 = 0;
unsigned long Task_2 = 0;
unsigned long Task_3 = 0;

String dato = "";
String envio = "";

void setup() {
  Serial.begin(9600);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); //SERIAL_8N1 8-bit No parity 1 Stop bit
}

void loop() {


  if (millis() - Task_1 > 100 ) {
    Task_1 = millis();

    if (Serial2.available() > 0) {
      String leer = Serial2.readStringUntil('\n');
      if (leer.startsWith("d:")) {
        for (int i = 0; i < leer.length(); i++) {
          if (isDigit(leer[i])) {
            dato += (String(leer[i]));
          }
        }
        if (millis() - Task_2 > 130 ) {
          Task_2 = millis();
          envio = dato;
        }
        dato = "";
      }
    }
  }

  Serial.println(envio);
}
