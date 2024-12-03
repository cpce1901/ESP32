#define BLYNK_TEMPLATE_ID "TMPL217OYDmzW"
#define BLYNK_TEMPLATE_NAME "Llenadora MAY"
#define BLYNK_AUTH_TOKEN "uTzzH7Ao2Pmh1Cfu9h-qADSojWHdnrDS"
#define BLYNK_PRINT Serial

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define BT_EMERG 23
#define BT_START 22
#define BT_STOP 21
#define BT_RESET 4

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
BlynkTimer timer;

char ssid[] = "USS_FIT";
char pass[] = "ussfit2019";

// State Enumeration
enum SystemState {
  DETENIDO,
  EMERGENCIA,
  START,
  LLENADO,
  LLENANDO,
  END
};

// Control Flags Structure
struct ControlFlags {
  bool physicalStart = false;
  bool physicalStop = false;
  bool physicalEmergency = false;
  bool physicalReset = false;

  bool remoteStart = false;
  bool remoteStop = false;
  bool remoteEmergency = false;
  bool remoteReset = false;
  bool remoteResetCounter = false;
};

SystemState currentState = DETENIDO;
ControlFlags controlFlags;
float minTank = 30.0;
float normalTank = 50.0;
bool liquidEnabled = false;
float tankPercentage = 0.0;
String currentStateStatus = "";

// KPIs
int countBottle = 3;
int bottleL = 20;
bool cycleComplete = false;
unsigned long totalCycles = 0;


// Tareas
TaskHandle_t TaskMonitorTank;
TaskHandle_t TaskHandleButtons;
TaskHandle_t TaskWork;
TaskHandle_t TaskLCDDisplay;


void TaskMonitorTankCode(void *pvParameters);
void TaskHandleButtonsCode(void *pvParameters);
void TaskWorkCode(void *pvParameters);
void TaskLCDDisplayCode(void *pvParameters);

unsigned long previousTotalCycles = 0;
String previousStateStatus = "";
bool previousLiquidEnabled = false;

void dataSend() {
  // Verifica si el total de ciclos cambió
  if (totalCycles != previousTotalCycles) {
    int totalBottlePeerCycle = totalCycles * countBottle;
    int totalLiterPeerCycle = totalBottlePeerCycle * bottleL;
    Blynk.virtualWrite(V8, totalBottlePeerCycle);
    Blynk.virtualWrite(V9, totalLiterPeerCycle);
    previousTotalCycles = totalCycles;  // Actualiza el valor anterior
  }


  // Verifica si el estado actual cambió
  if (currentStateStatus != previousStateStatus) {
    Blynk.virtualWrite(V2, currentStateStatus);
    previousStateStatus = currentStateStatus;  // Actualiza el valor anterior
  }

  // Verifica si el estado de liquidEnabled cambió
  if (liquidEnabled != previousLiquidEnabled) {
    Blynk.virtualWrite(V1, !liquidEnabled);
    previousLiquidEnabled = liquidEnabled;  // Actualiza el valor anterior
  }

  // La variable del porcentaje del tanque siempre se envía, ya que es un valor continuo
  Blynk.virtualWrite(V0, tankPercentage);
}

// Blynk Virtual Pin Handlers with Mutex-like Behavior
BLYNK_WRITE(V4) {  // START
  controlFlags.remoteStart = param.asInt();
}

BLYNK_WRITE(V5) {  // STOP
  controlFlags.remoteStop = param.asInt();
}

BLYNK_WRITE(V6) {  // EMERGENCY
  controlFlags.remoteEmergency = param.asInt();
}

BLYNK_WRITE(V7) {  // RESET
  controlFlags.remoteReset = param.asInt();
}

BLYNK_WRITE(V10) {  // RESET COUNTER
  controlFlags.remoteResetCounter = param.asInt();
}


void setup() {
  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Wire.begin(SDA, SCL);

  // Configure Button Pins
  pinMode(BT_EMERG, INPUT);
  pinMode(BT_START, INPUT);
  pinMode(BT_STOP, INPUT);
  pinMode(BT_RESET, INPUT);

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
  lcd.print(WiFi.localIP());
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

  timer.setInterval(5000L, dataSend);
}

void loop() {
  Blynk.run();
  timer.run();
}

void TaskMonitorTankCode(void *pvParameters) {
  for (;;) {
    int tankLevel = analogRead(LT100);  // Leer nivel del tanque (0-4095)
    tankPercentage = (tankLevel / 4095.0) * 100.0;

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
    // Read physical button states
    controlFlags.physicalEmergency = !digitalRead(BT_EMERG);
    controlFlags.physicalStart = digitalRead(BT_START);
    controlFlags.physicalStop = digitalRead(BT_STOP);
    controlFlags.physicalReset = digitalRead(BT_RESET);

    if (controlFlags.remoteResetCounter) {
      totalCycles = 0;
      Blynk.virtualWrite(V8, 0);
      Blynk.virtualWrite(V9, 0);
    }

    // Priority-based State Management
    if (!liquidEnabled) {
      currentState = DETENIDO;
    } else {
      // Emergency has the highest priority
      if (controlFlags.physicalEmergency || controlFlags.remoteEmergency) {
        currentState = EMERGENCIA;
      }

      // Reset only works when in Emergency state
      if (currentState == EMERGENCIA && (controlFlags.physicalReset || controlFlags.remoteReset)) {
        currentState = DETENIDO;
        controlFlags.remoteEmergency = false;
      }

      // Only process start/stop if not in Emergency
      if (currentState != EMERGENCIA) {
        // Prioritize physical buttons over remote
        if (controlFlags.physicalStop || controlFlags.remoteStop) {
          currentState = DETENIDO;
        } else if (controlFlags.physicalStart || controlFlags.remoteStart) {
          currentState = START;
        }
      }
    }

    // Reset transient flags
    controlFlags.physicalStart = false;
    controlFlags.physicalStop = false;
    controlFlags.physicalReset = false;
    controlFlags.remoteStart = false;
    controlFlags.remoteStop = false;
    controlFlags.remoteReset = false;

    vTaskDelay(200 / portTICK_PERIOD_MS);
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
        cycleComplete = false;
        break;

      case DETENIDO:
        // Apagar todas las salidas del sistema
        digitalWrite(SV101, LOW);
        digitalWrite(SV102, LOW);
        digitalWrite(SV103, LOW);
        digitalWrite(SV104, LOW);
        digitalWrite(B1, LOW);
        cycleComplete = false;
        break;

      case START:
        // Activar SV104
        digitalWrite(SV104, HIGH);
        if (digitalRead(LSW104)) {
          currentState = LLENADO;
          vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        break;
      case LLENADO:
        digitalWrite(SV101, HIGH);
        digitalWrite(SV102, HIGH);
        digitalWrite(SV103, HIGH);
        currentState = LLENANDO;
        vTaskDelay(2000 / portTICK_PERIOD_MS);
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
          vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        break;

      case END:
        digitalWrite(B1, LOW);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        digitalWrite(SV104, LOW);
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        if (!cycleComplete) {
          totalCycles++;
          Serial.println(totalCycles);
          cycleComplete = true;
        }

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
          currentStateStatus = "EMERGENCIA";
          break;
        case DETENIDO:
          lcd.print("Detenido        ");
          currentStateStatus = "DETENIDO";
          break;
        case START:
          lcd.print("Iniciado        ");
          currentStateStatus = "INICIANDO...";
          break;
        case LLENADO:
          lcd.print("Inicio Llenado  ");
          currentStateStatus = "LLENADO...";
          break;
        case LLENANDO:
          lcd.print("Llenando...     ");
          currentStateStatus = "LLENANDO...";
          break;
        case END:
          lcd.print("Fin Llenado     ");
          currentStateStatus = "TERMINADO";
          break;
      }
    } else {
      // Mostrar alerta de bajo nivel
      lcd.print("Low Level Alert! ");
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);  // Esperar 500 ms
  }
}
