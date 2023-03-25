#define RXD2 16
#define TXD2 17
 
void setup() {

  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  
}
 
void loop() {

  if (Serial2.available()) {
    Serial.write(Serial2.read());
  }
  
  if (Serial.available()) {
    Serial2.write(Serial.read());
  }
   
}
