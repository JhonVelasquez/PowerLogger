#include <Adafruit_SH1106.h>
#include <Adafruit_GFX.h>
#include <BufferedPrint.h>
#include <FreeStack.h>
#include <MinimumSerial.h>

#include <Wire.h>
#include <SPI.h>
//#include <SD.h>
#include "SdFat.h"

//SdFat SD;
#define chipSelect  10

#define display_RESET -1
//Adafruit_SSD1306 display(display_RESET);
//Adafruit_SSD1306 display(128,32,&Wire,display_RESET);
Adafruit_SH1106 display(display_RESET);

unsigned long currentMillis = millis();
unsigned long previousMillisSampling = 0;
#define interval  100

//Voltage sensor
#define voltageSensorPin 1
#define rMayorV 330000
#define rMenorV 100000
float loadvoltage = 0;

//Current sensor - shunt
#define currentSensorShuntPin 3 //era 2
#define rMayorI 51000
#define rMenorI 1020
//#define rShunt 0.084
#define opampGain 52
float currentShunt_mA = 0;
//Shunt value
#define potPin 1
float rShunt=0.084;
byte firstRun=0;
int potPinFirstValue=0;

//Current sensor - ACS712
#define currentSensorACS712Pin 2 // era 1 
#define ACS712Sensibility 0.066
float currentACS712_mA = 0;

//Current sensor
float current_mA = 0;
#define currentSensorSelectionPin 3
#define ledShuntPin 5
#define ledACSPin 4
unsigned long previousMillisCurrentLeds= 0;
byte ledShuntState=0;

//General
#define recordPin 2 
byte sdCardPresence=0;
float energy = 0;
//File TimeFile;
//File VoltFile;
//File CurFile;

void setup() {
  
  //SD.begin(chipSelect);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();
  
  pinMode(recordPin, INPUT);           // set pin to input
  digitalWrite(recordPin, HIGH);       // turn on pullup resistors
  pinMode(currentSensorSelectionPin, INPUT);           // set pin to input
  digitalWrite(currentSensorSelectionPin, HIGH);       // turn on pullup resistors
  pinMode(ledShuntPin, OUTPUT);           
  pinMode(ledACSPin, OUTPUT);
  digitalWrite(ledShuntPin,LOW);
  digitalWrite(ledACSPin,LOW);            

  // see if the card is present and can be initialized:
  /*if (!SD.begin(chipSelect)) {
    sdCardPresence=0;
  }else{
    sdCardPresence=1;
  }*/  
}

void loop() {
  currentMillis=millis();
  if (currentMillis - previousMillisSampling >= interval)
  {
    previousMillisSampling = currentMillis;
    updateSensorsValues();
    if(!digitalRead(recordPin))recordData();
    actualizarPotShunt();
    displaydata();
  }

  if (currentMillis - previousMillisCurrentLeds >= 400)
  {      
    if (!digitalRead(currentSensorSelectionPin)){
      if (currentShunt_mA<=1400){
        digitalWrite(ledShuntPin,HIGH);
        digitalWrite(ledACSPin,LOW);
      }else{
        digitalWrite(ledShuntPin,!ledShuntState);
      }
    }
    else {
      digitalWrite(ledShuntPin,LOW);
      digitalWrite(ledACSPin,HIGH);    
    }
  }
  
}

void actualizarPotShunt(){
    long var=0;
    var=analogRead(potPin);
    if(firstRun==0){
      potPinFirstValue=analogRead(potPin);
    }
    if((var-potPinFirstValue)>=20){
      //rShunt=+0.001*float(int(var/potPinFirstValue));
      //rShunt=+0.001;
      potPinFirstValue=var; 
    }else if((var-potPinFirstValue)<=-20){
      //rShunt=-0.001*float(int(var/potPinFirstValue));
      //rShunt=-0.001;
      potPinFirstValue=var;
    }      
}

void displaydata() {
  display.clearDisplay();
  display.setTextColor(WHITE); 
  display.setTextSize(1);

  display.setCursor(0,0);
  display.println(F("rShunt:"));
  display.setCursor(45,0);
  display.println((rShunt*1000));
  display.setCursor(80,0);
  display.println(F("mOhm"));
  display.setCursor(110,0);
  if (sdCardPresence) {
    display.println(F("SD"));
  }else{
    display.println(F("--"));
  }
  display.setCursor(0, 20);
  display.println((loadvoltage));
  display.setCursor(35, 20);
  display.println(F("V"));
  display.setCursor(50, 20);
  display.println((current_mA));
  display.setCursor(95, 20);
  display.println(F("mA"));
  display.setCursor(110, 20);
  if (!digitalRead(currentSensorSelectionPin)) display.println(F("SHN"));
  else display.println(F("ACS"));
  display.setCursor(0, 30);
  display.println((loadvoltage * current_mA));
  display.setCursor(65, 30);
  display.println(F("mW"));
  display.setCursor(0, 40);
  display.println(energy);
  display.setCursor(65, 40);
  display.println(F("mWh"));
  if(!digitalRead(recordPin)){
    display.setCursor(0, 10);
    display.println(F("Recording..."));
  }
  display.display();
}

void updateSensorsValues() {
  loadvoltage = ((rMayorV+rMenorV)/(rMenorV))*analogRead(voltageSensorPin)*5.0/1023.0;
  currentShunt_mA = (((analogRead(currentSensorShuntPin)*5.0/1023.0)/opampGain)/rShunt)*1000;
  currentACS712_mA= ((analogRead(currentSensorACS712Pin)*(5.0 / 1023.0)-2.5)/ACS712Sensibility)*1000;
  if (!digitalRead(currentSensorSelectionPin)) current_mA=currentShunt_mA;
  else current_mA=currentACS712_mA;
  energy = energy + loadvoltage * currentShunt_mA / 3600;
}

void recordData(){

    /*
    TimeFile = SD.open("TIME.txt", FILE_WRITE);
    if (TimeFile) {
      TimeFile.println((currentMillis));
      TimeFile.close();
    }
    */
    /*
    VoltFile = SD.open("VOLT.txt", FILE_WRITE);
    if (VoltFile) {
      VoltFile.println(F(loadvoltage);
      VoltFile.close();
    }*/
    /*
    CurFile = SD.open("CUR.txt", FILE_WRITE);
    if (CurFile) {
      CurFile.println(F(current_mA);
      CurFile.close();
    }*/
}

/*
  Capitulo 47 de Arduino desde cero en Español.
  Programa que muestra texto en la zona amarilla de la pantalla OLED y en la zona
  cyan los segundos transcurridos desde iniciado el programa
  Requiere instalar librerias Adafruit GFX y Adafruit SSD1306

  Autor: bitwiseAr  





#include <Wire.h>     // libreria para bus I2C
#include <Adafruit_GFX.h>   // libreria para pantallas graficas
#include <Adafruit_SSD1306.h>   // libreria para controlador SSD1306
#include <Adafruit_SH1106.h>

#define ANCHO 128     // reemplaza ocurrencia de ANCHO por 128
#define ALTO 64       // reemplaza ocurrencia de ALTO por 64

#define OLED_RESET -1      // necesario por la libreria pero no usado
//Adafruit_SSD1306 display(ANCHO, ALTO, &Wire, OLED_RESET);  // crea objeto
Adafruit_SH1106 display(OLED_RESET);
void setup() {
  Wire.begin();         // inicializa bus I2C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // inicializa pantalla con direccion 0x3C
}
 
void loop() {
  display.clearDisplay();      // limpia pantalla
  display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
  display.setCursor(0, 0);     // ubica cursor en inicio de coordenadas 0,0
  display.setTextSize(1);      // establece tamano de texto en 1
  display.print("Hola, han pasado:");  // escribe en pantalla el texto
  display.setCursor (10, 30);    // ubica cursor en coordenas 10,30
  display.setTextSize(1);      // establece tamano de texto en 2
  display.print(millis() / 1000);    // escribe valor de millis() dividido por 1000
  display.print(" seg.");      // escribe texto
  display.display();     // muestra en pantalla todo lo establecido anteriormente
}

*/
