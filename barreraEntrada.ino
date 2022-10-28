const int button = 15;
const int sensor_arriba = 2;
const int sensor_abajo = 4;
const int barrera_sube = 17;
const int barrera_baja = 16;
int estado_botton, funcionamiento, estado_s_arriba, estado_s_abajo;
boolean parada = false;
const int canalPWM = 0;
const int canalPWM1 = 1;
const int F = 10000;
const int resolucion = 8;


void setup() {
  Serial.begin(115200);
  pinMode(button, INPUT);
  pinMode(sensor_arriba, INPUT);
  
  ledcSetup(canalPWM, F, resolucion);
  ledcSetup(canalPWM1, F, resolucion);
  ledcAttachPin(barrera_sube, canalPWM);
  ledcAttachPin(barrera_baja, canalPWM1);

}

void loop() {

  estado_botton = digitalRead(button);
  estado_s_arriba = digitalRead(sensor_arriba);
  estado_s_abajo = digitalRead(sensor_abajo);


  if (estado_botton == 1 and estado_s_arriba == 1) {
    ledcWrite(canalPWM, 120);
    ledcWrite(canalPWM1, 0);
    funcionamiento = 1;
    Serial.println("Bajando");
  }

  else if (estado_botton == 1 and estado_s_abajo == 1) {
    ledcWrite(canalPWM, 0);
    ledcWrite(canalPWM1, 120);
    funcionamiento = 2;
    Serial.println("Subiendo");
  }

  else if (estado_s_abajo == 1 and funcionamiento == 1) {
    ledcWrite(canalPWM, 0);
    ledcWrite(canalPWM1, 0);
    Serial.println("STOP, llegue ABAJO");

  }

  else if (estado_s_arriba == 1 and funcionamiento == 2) {
    ledcWrite(canalPWM, 0);
    ledcWrite(canalPWM1, 0);
    Serial.println("STOP, llegue ARRIBA");

  }

  else if (estado_botton == 1 and parada == true and funcionamiento == 1) {
    ledcWrite(canalPWM, 120);
    ledcWrite(canalPWM1, 0);
    Serial.println("Sigo hacia ABAJO");
    parada = false;
    delay(100);


  }

  else if (estado_botton == 1 and parada == true and funcionamiento == 2) {
    ledcWrite(canalPWM, 0);
    ledcWrite(canalPWM1, 120);
    Serial.println("Sigo hacia ARRIBA");
    parada = false;
    delay(100);


  }

  else if (estado_botton == 1 and estado_s_arriba == 0 and estado_s_abajo == 0) {
    ledcWrite(canalPWM, 0);
    ledcWrite(canalPWM1, 0);
    Serial.println("STOP, aun no llego");
    parada = true;
    delay(100);

  }

  Serial.print(estado_botton);
  Serial.print(" - ");
  Serial.print(estado_s_arriba);
  Serial.print(" - ");
  Serial.print(estado_s_abajo);
  Serial.print(" - ");
  Serial.print(funcionamiento);
  Serial.print(" - ");
  Serial.println(parada);
  delay(100);



}
