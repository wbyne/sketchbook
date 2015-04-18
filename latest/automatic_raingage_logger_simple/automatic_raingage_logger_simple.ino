#define MSECS_CALC_RAIN 5000 //5 seconds
#define DRAINTIME 5500
#define EMPTYTIME 10000

int analogPin1 = 0;
int analogPin2 = 1;
int analogPin3 = 2;
int analogPin4 = 3;
int waterLevel1 = 0;
int waterLevel2 = 0;
int waterLevel3 = 0;
int waterLevel4 = 0;
int hasBeenRecorded1 = 0;
int hasBeenRecorded2 = 0;
int hasBeenRecorded3 = 0;

unsigned long nextCalcRain=0;
unsigned long time = 0;
unsigned int rainfall = 0;

//from datalogger example
#include <SD.h>
//datecalc for datalogger shield
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;


// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 10; 
 // make a string for assembling the data to log:
String dataString = "";
  
  
void setup () {
  Serial.begin(9600);
  pinMode(8,OUTPUT); //for powering the mosfet
  pinMode(6,INPUT_PULLUP);
  digitalWrite(6,HIGH);
  #ifdef AVR
  Wire.begin();
  #else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif
  rtc.begin();
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  //from datalogger example
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
    // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  dataString = "Program reset at: ";
  recordRainandTime();
}

void loop () {
  //millis() and micros() count time since the arduino was last restarted
   time = millis();  //get the time in milliseconds since last restart, will reset to 0 after 50 days

   if (time >= nextCalcRain) {  //has more than 5 seconds occurred since we last calculated rainfall?
      getRainDepth();  //call getRainDepth() and calculate rainfall total over this time (5 seconds)
      //at this point, if we're here in the if statement, we jump down to the getUnitRain subroutine
      //  we then return to this point after that routine finishes executing.
      nextCalcRain = time + MSECS_CALC_RAIN;  //calculated when to measure rainfall again: current time + 5 seconds
  }
}

void getRainDepth() {
  waterLevel1 = analogRead(analogPin1);  
  waterLevel2 = analogRead(analogPin2);  
  waterLevel3 = analogRead(analogPin3);  
  waterLevel4 = analogRead(analogPin4);  
  Serial.print(waterLevel1);
    Serial.print(" : ");
  Serial.print(waterLevel2);
    Serial.print(" : ");
  Serial.print(waterLevel3);
    Serial.print(" : ");
  Serial.println(waterLevel4);
  if (waterLevel4 > 35) {
    //record time and level
    rainfall = rainfall + 25;
    Serial.print("Waterlevel 4: ");    
    Serial.println(rainfall);    
    recordRainandTime();
    drainGage();
    hasBeenRecorded1 = 0;
    hasBeenRecorded2 = 0;
    hasBeenRecorded3 = 0;
  }
  else if (waterLevel3 > 35) {
      //record time and level
      if (hasBeenRecorded3 == 0) {
        rainfall = rainfall + 25;
        Serial.print("Waterlevel 3: ");    
        Serial.println(rainfall);    
        recordRainandTime();
        hasBeenRecorded3 = 1;
      }
  }
  else if (waterLevel2 > 35) {
      //record time and level
      if (hasBeenRecorded2 == 0) {
        rainfall = rainfall + 25;
        Serial.print("Waterlevel 2: ");    
        Serial.println(rainfall);    
        recordRainandTime();
        hasBeenRecorded2 = 1;
      }
  }
  else if (waterLevel1 > 35) {
      //record time and level
      if (hasBeenRecorded1 == 0) {
        rainfall = rainfall + 25;
        Serial.print("Waterlevel 1: ");    
        Serial.println(rainfall);    
        recordRainandTime();
        hasBeenRecorded1 = 1;
      }
    }
}

void drainGage() {
  digitalWrite (8, HIGH);
  Serial.println("OPENING DRAIN VALVE");
  delay(DRAINTIME); //run the motor for 5.5 seconds to rotate the valve open
  digitalWrite(8,LOW);
  Serial.println("WAITING TO DRAIN");
  delay(EMPTYTIME); //wait ten seconds for it to drain
  digitalWrite (8, HIGH);
  Serial.println("CLOSING VALVE");
  delay(DRAINTIME); //run the motor for 5.5 seconds to rotate the valve closed
  digitalWrite(8,LOW);
}

void recordRainandTime() {
    DateTime mytime = rtc.now();
    dataString.concat(mytime.year());
    dataString.concat("/");
    dataString.concat(mytime.month());
    dataString.concat("/");
    dataString.concat(mytime.day());
    dataString.concat("_");
    dataString.concat(mytime.hour());
    dataString.concat(":");
    dataString.concat(mytime.minute());
    dataString.concat(":");
    dataString.concat(mytime.second());
//    dataString.concat(time);
    dataString.concat(":");
    dataString.concat(rainfall);
    dataString.concat(",");
    dataString += (" hundredths of an inch");
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("rainfall.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
 
      // print to the serial port too:
      Serial.println(dataString);
    }  
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening rainfall.txt");
    } 
    dataFile.close();
    dataString = "";
}


