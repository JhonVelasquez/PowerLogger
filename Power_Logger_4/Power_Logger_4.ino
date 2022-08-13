#include <Adafruit_SH1106.h>
#include <Adafruit_GFX.h>
#include <BufferedPrint.h>
#include <FreeStack.h>
#include <MinimumSerial.h>

#include <Wire.h>
#include <SPI.h>


#define display_RESET -1
Adafruit_SH1106 display(display_RESET);

//Analog pins
#define voltageSensorPin 1
#define currentSensorShuntPin 3 
#define potPin 7
#define currentSensorACS712Pin 2 
//Digital pins
#define currentSensorSelectionPin 3
#define ledShuntPin 5
#define ledShuntPinPORTD 5
#define ledACSPin 4
#define recordPin 2

//VOLTAGE SENSOR
#define rMayorV 333500 //u 330k
#define rMenorV 99100 //u 100k
float loadvoltage = 0;

//CURRENT SENSOR 
float current_mA = 0;
//CURRENT SENSOR - SHUNT
#define rMayorI 50500 // 51 k
#define rMenorI 1004 // 1k + 20
#define opampGain 50.2988047809 //u
float currentShunt_mA = 0;
float rShunt=0.071; //u
byte firstRun=0;
int potPinFirstValue=0;
//CURRENT SENSOR  - ACS712
#define ACS712Sensibility 0.066 //u
float currentACS712_mA = 0;



//TIMING
unsigned long currentMillis = millis();
unsigned long previousMillisSampling = 0;
#define interval  100
unsigned long previousMillisCurrentLeds= 0;

//General
byte ledShuntState=0;
byte sdCardPresence=0;
float energy = 0;

void setup() {

  Serial.begin(115200);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(500);
  display.clearDisplay();
  
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
  currentMillis=millis();
  if (currentMillis - previousMillisSampling >= interval)
  {
    previousMillisSampling = currentMillis;
    updateSensorsValues();
    if(!digitalRead(recordPin))recordData();
    
    displaydata();
  }

  if (currentMillis - previousMillisCurrentLeds >= 400)
  {      
    if (!digitalRead(currentSensorSelectionPin)){ //Shunt
      /*if (currentShunt_mA<=500){
        digitalWrite(ledShuntPin,HIGH);
      }else{ 
        digitalWrite(ledShuntPin,!ledShuntState);
      }*/
      if (currentShunt_mA<=500){
        digitalWrite(ledShuntPin,HIGH);
      }else{ 
        digitalWrite(ledShuntPin,!bitRead(PORTD,ledShuntPinPORTD));        
      }
      digitalWrite(ledACSPin,LOW);
    }
    else { //ACS712
      digitalWrite(ledShuntPin,LOW);
      digitalWrite(ledACSPin,HIGH);    
    }
  }
  actualizarPotShunt();
  delay(1);
  
}

void actualizarPotShunt(){
    long var=0;
    var=analogRead(potPin);
    if(firstRun==0){
      potPinFirstValue=analogRead(potPin);
    }
    firstRun=1;
    if((var-potPinFirstValue)>7){
      //rShunt=+0.001*float(int(var/potPinFirstValue));
      rShunt=rShunt+0.001;
      potPinFirstValue=var; 
    }else if((var-potPinFirstValue)<-7){
      //rShunt=-0.001*float(int(var/potPinFirstValue));
      rShunt=rShunt-0.001;
      potPinFirstValue=var;
    }
    Serial.print(potPinFirstValue);
    Serial.print(" - ");
    Serial.print(var);
    Serial.print(" - ");
    Serial.println(rShunt*1000);      
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
