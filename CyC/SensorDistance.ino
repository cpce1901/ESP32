#include <SPIFFS.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define RXD2 16
#define TXD2 17

#define button_1 25
#define button_2 27
#define out_1 32
#define out_2 4

String dataSerial2 = "";  // Datos recibidos en el puerto serie 2
int min_d = 300;          // Valor por defecto de distancia minimo
int max_d = 1000;         // Valor por defecto de distancia maxima
String estado_on = "";
String estado_off = "";
int distance = 0;
int distance_min = 0;
int distance_max = 0;
bool error = false;


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

  // Iniciamos la lectura de la memoria interna
  if (!SPIFFS.begin(true)) {
    Serial.println("A ocurrido un error al montar SPIFFS");
    return;
  }

  // Listamos los archivos existentes en memoria
  listFilles();
  readJson();

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.println(F("Made in CyC by CpCe"));
  display.display();
  delay(2000);

  pinMode(button_1, INPUT);
  pinMode(button_2, INPUT);
  pinMode(out_1, OUTPUT);
  pinMode(out_2, OUTPUT);
}

void loop() {

  if (Serial2.available()) {
    char c = Serial2.read();
    if (c == ':') {
      distance = processReceivedData();
      //Serial.println(distance);
      inicio(distance, min_d, max_d, estado_on, estado_off);
      dataSerial2 = "";
    } else {
      dataSerial2 += c;
    }
  }

  // Verificar el estado del rango, si esta dentro de el o no lo esta
  if (millis() - work_1 >= 100) {
    work_1 = millis();
    if (distance <= min_d || distance >= max_d) {
      digitalWrite(out_1, 0);  // Deshabilita el encendido
      estado_on = "Off";
      digitalWrite(out_2, 1);  // Habilita el apagado
      estado_off = "On";
    } else {
      digitalWrite(out_1, 1);  // Habilita el encendido
      estado_on = "On";
      digitalWrite(out_2, 0);  // Deshabilita el apagado
      estado_off = "Off";
    }
  }

  if (millis() - work_2 >= 400) {
    work_2 = millis();

    bool state_1 = digitalRead(button_1);  //Lectura de botones
    bool state_2 = digitalRead(button_2);  //Lectura de botones

    if (state_1 && !state_2) {
      resta();
    }
    if (state_2 && !state_1) {
      suma();
    }
  }
}

void inicio(int dato, int mini, int maxi, String on, String off) {

  float result = dato * 0.001;
  display.clearDisplay();

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.println(dato);

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 20);             // Start at top-left corner
  display.println(mini);

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 40);             // Start at top-left corner
  display.println(maxi);

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(40, 20);            // Start at top-left corner
  display.println("Encendido: ");

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(110, 20);           // Start at top-left corner
  display.println(on);

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(40, 40);            // Start at top-left corner
  display.println("Apagado: ");

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(110, 40);           // Start at top-left corner
  display.println(off);
  display.display();
}

void resta() {
  displayProgressBar(100, 100, 1);
  while (true) {

    int capture_min;

    if (Serial2.available()) {
      char c = Serial2.read();
      if (c == ':') {
        distance_min = processReceivedData();

        display.clearDisplay();

        display.setTextSize(1);               // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);  // Draw white text
        display.setCursor(80, 0);             // Start at top-left corner
        display.println(distance_min);

        display.setTextSize(1);               // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);  // Draw white text
        display.setCursor(30, 20);            // Start at top-left corner
        display.println("Captura minima");

        display.setTextSize(1);               // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);  // Draw white text
        display.setCursor(40, 40);            // Start at top-left corner
        display.println(capture_min);

        display.display();

        dataSerial2 = "";
      } else {
        dataSerial2 += c;
      }
    }

    if (millis() - work_3 >= 300) {
      work_3 = millis();
      bool state_1 = digitalRead(button_1);  //Lectura de botones

      if (state_1) {
        capture_min = distance_min;
        min_d = capture_min;
        recDataJson();
      } else {
        capture_min = min_d;
      }

      if (min_d > max_d) {
        error = true;
        error_min("El valor minimo no puede ser mayor al valor maximo");
      }
    }

    if (millis() - work_4 >= 320) {
      work_4 = millis();
      bool state_2 = digitalRead(button_2);  //Lectura de botones

      if (state_2) {
        delay(500);
        break;
      }
    }
  }
}

void suma() {
  displayProgressBar(100, 100, 1);
  while (true) {
    int capture_max;
    if (Serial2.available()) {
      char c = Serial2.read();
      if (c == ':') {
        distance_max = processReceivedData();

        display.clearDisplay();
        display.setTextSize(1);               // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);  // Draw white text
        display.setCursor(80, 0);             // Start at top-left corner
        display.println(distance_max);

        display.setTextSize(1);               // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);  // Draw white text
        display.setCursor(30, 20);            // Start at top-left corner
        display.println("Captura maxima");

        display.setTextSize(1);               // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);  // Draw white text
        display.setCursor(40, 40);            // Start at top-left corner
        display.println(capture_max);

        display.display();

        dataSerial2 = "";
      } else {
        dataSerial2 += c;
      }
    }

    if (millis() - work_5 >= 300) {
      work_5 = millis();
      bool state_1 = digitalRead(button_1);  //Lectura de botones
      if (state_1) {
        capture_max = distance_max;
        max_d = capture_max;
        recDataJson();
      } else {
        capture_max = max_d;
      }

      if (max_d < min_d) {
        error = true;
        error_max("El valor maximo no puede ser menor al valor minimo");
      }
    }

    if (millis() - work_6 >= 320) {
      work_6 = millis();
      bool state_2 = digitalRead(button_2);  //Lectura de botones

      if (state_2) {
        delay(500);
        break;
      }
    }
  }
}

void error_min(String e) {

  display.clearDisplay();

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.println(e);

  display.display();

  delay(3500);
  min_d = 0;
}

void error_max(String e) {

  display.clearDisplay();

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.println(e);

  display.display();

  delay(3500);
  max_d = 9999;
}

void displayProgressBar(int progress, int totalProgress, int delayTime) {
  // Borramos la pantalla OLED
  display.clearDisplay();

  // Bucle "for" para actualizar el progreso de la barra
  for (int i = 0; i <= progress; i++) {
    // Calculamos el ancho de la barra en función del progreso
    int barWidth = map(i, 0, totalProgress, 0, display.width() - 2);

    // Dibujamos la barra de carga en la pantalla
    display.drawRect(1, 12, display.width() - 2, 8, WHITE);
    display.fillRect(1, 12, barWidth, 8, WHITE);
    display.display();

    // Pequeña pausa para visualizar el progreso
    delay(delayTime);
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

void listFilles() {
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.print("ARCHIVO: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }
  root.close();
  file.close();
}

void readJson() {

  File file = SPIFFS.open("/datos.json");
  if (file) {
    StaticJsonDocument<255> doc;
    DeserializationError error = deserializeJson(doc, file);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    } else {
      min_d = doc["min_d"];
      max_d = doc["max_d"];

      Serial.println("min_d: " + min_d);
      Serial.println("max_d: " + max_d);
    }
    file.close();
  }
}

void recDataJson() {

  File outfile = SPIFFS.open("/datos.json", "w");
  StaticJsonDocument<255> object;
  object["min_d"] = min_d;
  object["max_d"] = max_d;

  if (serializeJson(object, outfile) == 0) {
    Serial.println("Error al grabar archivo");
  } else {
    Serial.println("Archivo Grabado");
  }
  outfile.close();
}
