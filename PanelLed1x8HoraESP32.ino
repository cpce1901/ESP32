#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_H

#define MAX_DEVICES 8
#define CLK_PIN 18
#define DATA_PIN 23
#define CS_PIN 5
MD_Parola Display = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const char* ssid = "Jessica2.4";
const char* password = "167832873";

String Time, hour, minute, seconds;
String Formatted_date;
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
int intensity = 15;  // Nivel de intensidad (0-15)

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting.");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  timeClient.begin();
  timeClient.setTimeOffset(-4 * 3600);

  Display.begin();
  Display.setIntensity(intensity);  // Ajusta la intensidad de los LED
  Display.displayClear();
}

void loop() {
  obtainTime();
}

void obtainTime() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  currentMillis = millis();

  if (currentMillis - previousMillis > interval) {
    previousMillis = millis();

    Formatted_date = timeClient.getFormattedDate();
    Serial.println(Formatted_date);

    hour = Formatted_date.substring(11, 13);
    minute = Formatted_date.substring(14, 16);
    seconds = Formatted_date.substring(17, 19);


    Time = hour + ":" + minute + ":" + seconds;
    Serial.println(Time);
    Display.setTextAlignment(PA_CENTER);
    Display.print(Time);
  }
}
