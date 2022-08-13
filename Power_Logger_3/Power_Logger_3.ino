#include <Adafruit_SH1106.h>

#include <BufferedPrint.h>
#include <FreeStack.h>
#include <MinimumSerial.h>

#include <Wire.h>
#include <SPI.h>
#include "SdFat.h"
//#include <SD.h>


#define chipSelect  10

#define display_RESET -1
//Adafruit_SSD1306 display(display_RESET);
//Adafruit_SSD1306 display(128,32,&Wire,display_RESET);


#define interval  100

//Voltage sensor
#define voltageSensorPin 3
#define rMayorV 330000
#define rMenorV 100000


//Current sensor - shunt
#define currentSensorShuntPin 2
#define rMayorI 51000
#define rMenorI 1020
//#define rShunt 0.084
#define opampGain 52

//Shunt value
#define potPin 7

//Current sensor - ACS712
#define currentSensorACS712Pin 1
#define ACS712Sensibility 0.066

//Current sensor
#define currentSensorSelectionPin 3
#define ledShuntPin 8
#define ledACSPin 9

//General
#define recordPin 2 
byte SDCardPresence=0;
//SDFat SD;


 
void setup() {

  /*
  if (!SD.begin(chipSelect)) {   
    SDCardPresence=0;
  }else{
    SDCardPresence=1;
  }*/
  
  pinMode(recordPin, INPUT);           // set pin to input
  digitalWrite(recordPin, HIGH);       // turn on pullup resistors
  pinMode(currentSensorSelectionPin, INPUT);           // set pin to input
  digitalWrite(currentSensorSelectionPin, HIGH);       // turn on pullup resistors
  pinMode(ledShuntPin, OUTPUT);           
  pinMode(ledACSPin, OUTPUT);
  digitalWrite(ledShuntPin,LOW);
  digitalWrite(ledACSPin,LOW);
  
}


void loop() {    
  Adafruit_SH1106 display(display_RESET);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  SdFat32 SD;
  File32 file;
  if (!SD.begin(chipSelect)) {   
    SDCardPresence=0;
  }else{
    SDCardPresence=1;
  }
  
  unsigned long currentMillis = millis();
  unsigned long previousMillisSampling = 0;
  //Voltage sensor
  float loadvoltage = 0;
  //Current sensor -shunt
  float currentShunt_mA = 0;
  //Shunt value
  float rShunt=0.084;
  byte firstRun=0;
  float potPinFirstValue=0;
  //Current sensor ACS
  float currentACS712_mA = 0; 
  //Current sensor
  unsigned long previousMillisCurrentLeds= 0;
  byte ledShuntState=0;
  float current_mA;
  //General
  
  float energy = 0;
  
  //File VoltFile;
  //File CurFile;
  long var=0;  
  while(1){
  currentMillis=millis();
  if (currentMillis - previousMillisSampling >= interval)
  {
    previousMillisSampling = currentMillis;
    //UPDATE_SENSORS_VALUES
        loadvoltage = ((rMayorV+rMenorV)/(rMenorV))*analogRead(voltageSensorPin)*5.0/1023.0;
        currentShunt_mA = (((analogRead(currentSensorShuntPin)*5.0/1023.0)/opampGain)/rShunt)*1000;
        currentACS712_mA= (analogRead(currentSensorACS712Pin)*(5.0 / 1023.0)-2.5)/ACS712Sensibility;
        if (digitalRead(currentSensorSelectionPin)) current_mA=currentShunt_mA;
        else current_mA=currentACS712_mA;
        energy = energy + loadvoltage * currentShunt_mA / 3600;
    //RECORD DATA
    if(digitalRead(recordPin)&&(SDCardPresence==1)&&0){
        /*file = SD.open("ee.txt", FILE_WRITE);
        if (file) {
          //file.println(currentMillis);
          //file.close();
          file.println(F("Hello2222"));
          file.close();
        }*/  
  
          /*
          file.open("SizeTest.txt", O_RDWR | O_CREAT | O_AT_END);
          file.println("Hello2");
          file.close();*/
          /*file.print((currentMillis));
          file.print(F(" "));
          file.print((loadvoltage));
          file.print(F(" "));
          file.println((current_mA));
          file.close();*/
          
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
    //ACTUALIZAR POTENTIOMETER VALUE OF SHUNT RESISTANCE
        var=analogRead(potPin);
        if(firstRun==0){
          potPinFirstValue=analogRead(potPin);
        }
        if((var-potPinFirstValue)>=20){
          //rShunt=+0.001*float(int(var/potPinFirstValue));
          rShunt=+0.001;
          potPinFirstValue=var; 
        }else if((var-potPinFirstValue)<=-20){
          //rShunt=-0.001*float(int(var/potPinFirstValue));
          rShunt=-0.001;
          potPinFirstValue=var;
        }   
    //DISPLAY DATA
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
      if (SDCardPresence) {
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
}
