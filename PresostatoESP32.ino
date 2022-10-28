// Librerias OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Objeto OLED
Adafruit_SSD1306 PANEL(128, 64, &Wire, -1);

// Definimos nuestras enytradas y salidas
#define Button_1 35
#define Button_2 32
#define Sensor 34
#define Rele 33

// Variables globales de tiempo
unsigned long Task_1 = 0;
unsigned long Task_2 = 0;
unsigned long Task_3 = 0;
unsigned long Task_4 = 0;
unsigned long Task_5 = 0;
unsigned long Task_6 = 0;
unsigned long Task_7 = 0;
unsigned long Task_8 = 0;

// Variables globales de programa
float Set_Down, Set_Up, Lec, Set_1, Set_2;
String Status;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  PANEL.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(Button_1, INPUT);
  pinMode(Button_2, INPUT);
  pinMode(Sensor, INPUT);
  pinMode(Rele, OUTPUT);

  // Inicializamos variables
  Status = "OFF";
  Set_Down = 1;
  Set_Up = 5;
}
void loop() {
  // Variables a ser enviadas
  Set_1 = Set_Down;
  Set_2 = Set_Up;

  ////////////////////////////////////////Pantalla PRINCIPAL/////////////////////////////////////////////////
  while (true)
  {
    bool Esta_1 = digitalRead(Button_1); //Lectura de botones
    bool Esta_2 = digitalRead(Button_2); //Lectura de botones
    bool Esta_Re;

    Inicio(Set_Down, Set_Up, Lec, Status); //LLamamos a la funcion para imprimir la pantalla OLED

    if (millis() - Task_1 > 500) {      //Lectura de sensor cada 500ms
      Task_1 = millis();
      Lec = (analogRead(Sensor) * 0.000805) / 0.275;            //Escalamiento de variable considerando 3.3V y 12Bar
      if (Lec >= Set_Down and Lec < Set_Up) {                   //Limites de encendido de rele
        Status = "ON";
        digitalWrite(Rele, HIGH);
        Esta_Re = true;                              //Marca de estado de rele
      }
      else {
        Status = "OFF";
        digitalWrite(Rele, LOW);
        Esta_Re = false;
      }
    }
    if (millis() - Task_2 > 20) {      //Lectura de boton de prueba solo cuando este fuera de rango
      Task_2 = millis();
      if (Esta_1 and !Esta_Re) {
        Status = "ON";
        digitalWrite(Rele, HIGH);
        Serial.println("Hola");
        Esta_Re = true;
      }
    }
    if (millis() - Task_3 > 200) {      //Lectura de boton para ingreso a set BP
      Task_3 = millis();
      if (Esta_2) {
        delay(300);                     //Tiempo muerto antes de salir del menu
        break;
      }
    }
  }

  ////////////////////////////////////////Pantalla SET PB/////////////////////////////////////////////////
  while (true) {
    String tp = "PB";
    bool Esta_1a = digitalRead(Button_1); //Lectura de botones en menu Set PB
    bool Esta_2b = digitalRead(Button_2);

    Set_1 = Set(Set_Down, tp);         //Llamamos a la funcion para imprimir en pantalla OLED el menu Set BP

    if (millis() - Task_4 > 200) {      //Lectura de boton para aumentar variable durante 200ms.
      Task_4 = millis();
      if (Esta_1a) {
        Set_Down = Set_Down + 0.5;      //Aumento de variable en 0.5bar
        if (Set_Down > 12) {            //Limitacion de variable a 12bar maximo
          Set_Down = 0;                 //Reinicio de variable 0bar
        }
      }
    }
    if (millis() - Task_5 > 200) {      //Lectura de boton para ingreso a set AP
      Task_5 = millis();
      if (Esta_2b) {
        delay(300);                     //Tiempo muerto antes de salir del menu
        break;
      }
    }
  }

  ////////////////////////////////////////Pantalla SET PA/////////////////////////////////////////////////
  while (true) {
    String tp = "PA";
    bool Esta_1aa = digitalRead(Button_1); //Lectura de botones en menu Set PA
    bool Esta_2bb = digitalRead(Button_2);

    Set_2 = Set(Set_Up, tp);         //Llamamos a la funcion para imprimir en pantalla OLED el menu Set PA

    if (millis() - Task_6 > 200) {      //Lectura de boton para aumentar variable durante 200ms.
      Task_6 = millis();
      if (Esta_1aa) {
        Set_Up = Set_Up + 0.5;        //Aumento de variable en 0.5bar
        if (Set_Up > 12) {            //Limitacion de variable a 12bar maximo
          Set_Up = 0;                 //Reinicio de variable 0bar
        }
      }
    }
    if (millis() - Task_7 > 200) {      //Lectura de boton para regreso a PANTALLA PRINCIPAL
      Task_7 = millis();
      if (Esta_2bb) {
        delay(300);                     //Tiempo muerto antes de salir del menu
        break;
      }
    }
  }
}

void Inicio(float Set_D, float Set_U, float Lectura, String Sta)
{
  PANEL.clearDisplay();
  PANEL.setTextColor(WHITE);
  PANEL.setCursor(35, 5);
  PANEL.setTextSize(1);
  PANEL.print("Set - Bar");

  PANEL.setTextColor(WHITE);
  PANEL.setCursor(60, 35);
  PANEL.setTextSize(1);
  PANEL.print("to");

  PANEL.setTextColor(WHITE);
  PANEL.setCursor(5, 30);
  PANEL.setTextSize(2);
  PANEL.print(Set_D, 1);

  PANEL.setTextColor(WHITE);
  PANEL.setCursor(80, 30);
  PANEL.setTextSize(2);
  PANEL.print(Set_U, 1);

  PANEL.setTextColor(WHITE);
  PANEL.setCursor(53, 55);
  PANEL.setTextSize(1);
  PANEL.print(Lectura, 1);

  PANEL.setTextColor(WHITE);
  PANEL.setCursor(100, 55);
  PANEL.setTextSize(1);
  PANEL.print(Sta);
  PANEL.display();
}

float Set(float seting, String Type)
{
  PANEL.clearDisplay();
  PANEL.setTextColor(WHITE);
  PANEL.setCursor(25, 5);
  PANEL.setTextSize(1);
  PANEL.print("Seleccione " + Type);

  PANEL.setTextColor(WHITE);
  PANEL.setCursor(42, 25);
  PANEL.setTextSize(2);
  PANEL.print(seting, 1);

  PANEL.setTextColor(WHITE);
  PANEL.setCursor(58, 55);
  PANEL.setTextSize(1);
  PANEL.print("Bar");
  PANEL.display();

  return seting;
}
