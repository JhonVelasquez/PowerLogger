#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include "SdFat.h"
SdFat32 SD;

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

unsigned long previousMillis = 0;
unsigned long interval = 100;
const int chipSelect = 10;
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float energy = 0;
File32 TimeFile;
File32 VoltFile;
File32 CurFile;


void setup() {
  SD.begin(chipSelect);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    ina219values();

    TimeFile = SD.open("TIME2.txt", FILE_WRITE);
    if (TimeFile) {
      TimeFile.println(currentMillis);
      TimeFile.close();
    }

    VoltFile = SD.open("VOLT2.txt", FILE_WRITE);
    if (VoltFile) {
      VoltFile.println(loadvoltage);
      VoltFile.close();
    }

    CurFile = SD.open("CUR2.txt", FILE_WRITE);
    if (CurFile) {
      CurFile.println(current_mA);
      CurFile.close();
    }
    displaydata();
  }
}

void displaydata() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(loadvoltage);
  display.setCursor(35, 0);
  display.println("V");
  display.setCursor(50, 0);
  display.println(current_mA);
  display.setCursor(95, 0);
  display.println("mA");
  display.setCursor(0, 10);
  display.println(loadvoltage * current_mA);
  display.setCursor(65, 10);
  display.println("mW");
  display.setCursor(0, 20);
  display.println(energy);
  display.setCursor(65, 20);
  display.println("mWh");
  display.display();
}

void ina219values() {
  shuntvoltage = 2;
  busvoltage =3;
  current_mA = 4;
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  energy = energy + loadvoltage * current_mA / 3600;
}
