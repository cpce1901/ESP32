#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BT_EMERG 23
#define BT_START 22
#define BT_STOP 21

#define SV101 26
#define SV102 27
#define SV103 32
#define SV104 12

#define LT100 34  // Analógico
#define LT101 19  // Digital
#define LT102 18  // Digital
#define LT103 5   // Digital
#define LSW104 35

#define B1 25

#define SCL 16  // Pin de reloj para I2C
#define SDA 17  // Pin de datos para I2C

LiquidCrystal_I2C lcd(0x27, 16, 2);

// State Enumeration
enum SystemState {
  DETENIDO,
  EMERGENCIA,
  START,
  LLENADO,
  LLENANDO,
  END
};

SystemState currentState = DETENIDO;
float minTank = 30.0;
float normalTank = 50.0;
bool liquidEnabled = false;

// Tareas
TaskHandle_t TaskMonitorTank;
TaskHandle_t TaskHandleButtons;
TaskHandle_t TaskWork;
TaskHandle_t TaskLCDDisplay;

void TaskMonitorTankCode(void *pvParameters);
void TaskHandleButtonsCode(void *pvParameters);
void TaskWorkCode(void *pvParameters);
void TaskLCDDisplayCode(void *pvParameters);

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);

  // Configure Button Pins
  pinMode(BT_EMERG, INPUT);
  pinMode(BT_START, INPUT);
  pinMode(BT_STOP, INPUT);

  // Configure Level and Control Pins
  pinMode(LT100, INPUT);
  pinMode(LT101, INPUT);
  pinMode(LT102, INPUT);
  pinMode(LT103, INPUT);
  pinMode(LSW104, INPUT);

  pinMode(SV101, OUTPUT);
  pinMode(SV102, OUTPUT);
  pinMode(SV103, OUTPUT);
  pinMode(SV104, OUTPUT);
  pinMode(B1, OUTPUT);

  // Initialize all outputs to LOW
  digitalWrite(SV101, LOW);
  digitalWrite(SV102, LOW);
  digitalWrite(SV103, LOW);
  digitalWrite(SV104, LOW);
  digitalWrite(B1, LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(2000);
  lcd.clear();

  // Create tasks
  xTaskCreatePinnedToCore(
    TaskMonitorTankCode,  // Task function
    "MonitorTank",        // Name of the task
    4096,                 // Stack size
    NULL,                 // Task parameters
    1,                    // Priority
    &TaskMonitorTank,     // Task handle
    1                     // Core to run the task
  );

  xTaskCreatePinnedToCore(
    TaskHandleButtonsCode,  // Task function
    "HandleButtons",        // Name of the task
    4096,                   // Stack size
    NULL,                   // Task parameters
    1,                      // Priority
    &TaskHandleButtons,     // Task handle
    1                       // Core to run the task
  );

  xTaskCreatePinnedToCore(
    TaskWorkCode,  // Task function
    "Work",        // Name of the task
    4096,          // Stack size
    NULL,          // Task parameters
    1,             // Priority
    &TaskWork,     // Task handle
    1              // Core to run the task
  );

  xTaskCreatePinnedToCore(
    TaskLCDDisplayCode,  // Task function
    "LCDDisplay",        // Name of the task
    4096,                // Stack size
    NULL,                // Task parameters
    1,                   // Priority
    &TaskLCDDisplay,     // Task handle
    1                    // Core to run the task
  );

  Serial.println("Setup complete. System ready.");
}

void loop() {
}

void TaskMonitorTankCode(void *pvParameters) {
  for (;;) {
    int tankLevel = analogRead(LT100);                       // Leer nivel del tanque (0-4095)
    float tankPercentage = map(tankLevel, 0, 4095, 0, 100);  // Convertir a porcentaje

    // Mostrar nivel del tanque en la primera fila
    lcd.setCursor(0, 0);
    lcd.print("LT100: ");
    lcd.print(tankPercentage, 1);  // Mostrar con un decimal
    lcd.print("%   ");             // Espacios extra para borrar caracteres sobrantes

    // Comprobar si el nivel del tanque está dentro del rango de habilitación del líquido
    if (tankPercentage > normalTank) {
      liquidEnabled = true;  // Activar cuando el nivel es mayor al 50%
    } else if (tankPercentage < minTank) {
      liquidEnabled = false;  // Desactivar cuando el nivel es menor al 30%
    }



    vTaskDelay(500 / portTICK_PERIOD_MS);  // Esperar 500 ms
  }
}

void TaskHandleButtonsCode(void *pvParameters) {
  for (;;) {
    // Leer el estado de los botones
    bool emergPressed = digitalRead(BT_EMERG);
    bool startPressed = digitalRead(BT_START);
    bool stopPressed = digitalRead(BT_STOP);

    // Si liquidEnabled es false, el sistema debe estar en DETENIDO
    if (!liquidEnabled) {
      currentState = DETENIDO;
    } 
    // Si liquidEnabled es true, proceder con los botones
    else {
      if (!emergPressed) {
        currentState = EMERGENCIA;
      } else if (stopPressed) {
        currentState = DETENIDO;
      } else if (startPressed) {
        currentState = START;
      }
    }

    vTaskDelay(200 / portTICK_PERIOD_MS);  // Esperar 200 ms antes de revisar nuevamente
  }
}

void TaskWorkCode(void *pvParameters) {
  for (;;) {
    // Determina el mensaje según el estado
    switch (currentState) {
      case EMERGENCIA:
        // Apagar todas las salidas del sistema
        digitalWrite(SV101, LOW);
        digitalWrite(SV102, LOW);
        digitalWrite(SV103, LOW);
        digitalWrite(SV104, LOW);
        digitalWrite(B1, LOW);
        break;

      case DETENIDO:
        // Apagar todas las salidas del sistema
        digitalWrite(SV101, LOW);
        digitalWrite(SV102, LOW);
        digitalWrite(SV103, LOW);
        digitalWrite(SV104, LOW);
        digitalWrite(B1, LOW);
        break;

      case START:
        // Activar SV104
        digitalWrite(SV104, HIGH);
        if (digitalRead(LSW104)) {
          currentState = LLENADO;
          vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        break;
      case LLENADO:
        digitalWrite(SV101, HIGH);
        digitalWrite(SV102, HIGH);
        digitalWrite(SV103, HIGH);
        currentState = LLENANDO;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        break;

      case LLENANDO:
        digitalWrite(B1, HIGH);

        // Apagar las válvulas si los sensores de nivel indican que deben apagarse
        if (digitalRead(LT101)) {
          digitalWrite(SV101, LOW);
        }
        if (digitalRead(LT102)) {
          digitalWrite(SV102, LOW);
        }
        if (digitalRead(LT103)) {
          digitalWrite(SV103, LOW);
        }

        if (digitalRead(LT101) && digitalRead(LT102) && digitalRead(LT103)) {
          currentState = END;
          vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        break;

      case END:
        digitalWrite(B1, LOW);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        digitalWrite(SV104, LOW);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        currentState = DETENIDO;
        break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Esperar 100 ms
  }
}

void TaskLCDDisplayCode(void *pvParameters) {
  for (;;) {
    lcd.setCursor(0, 1);
    if (liquidEnabled) {
      // Mostrar los estados de proceso
      switch (currentState) {
        case EMERGENCIA:
          lcd.print("Emergencia      ");
          break;
        case DETENIDO:
          lcd.print("Detenido        ");
          break;
        case START:
          lcd.print("Iniciado        ");
          break;
        case LLENADO:
          lcd.print("Inicio Llenado  ");
          break;
        case LLENANDO:
          lcd.print("Llenando...     ");
          break;
        case END:
          lcd.print("Fin Llenado     ");
          break;
      }
    } else {
      // Mostrar alerta de bajo nivel
      lcd.print("Low Level Alert! ");
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);  // Esperar 500 ms
  }
}
