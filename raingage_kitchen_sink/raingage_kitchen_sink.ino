//annotated 23 June 2014
//first, define constants.  Unlike variables, these values shouldn't 
//  change in the program
#define RAIN_GAUGE_PIN 2  //the pin used to read the reed switch
#define RAIN_GAUGE_INT 0  //the declaration of which interrupt.  Pin 2 = INT 0 
#define RAIN_FACTOR 0.01  //amount of rainfall per count / tip of the bucket
// How often we want to calculate rain
#define MSECS_CALC_RAIN 5000  

//volatile is reserved for variables that may be changed outside of 
//  the program scope where they appear, so they are loaded into RAM
//  variables in Interrupt Service Routines (ISRs) should be volatile.
// These variables are global because they are declared outside of the
//  main program body.
volatile unsigned long rain_count=0;
unsigned long rain_last=0;
unsigned long nextCalcRain=0;
double unit_rain = 0 ;
unsigned long time = 0;
unsigned long rain2 = 0;

// make a string for assembling the data to log:
String dataString = "";

//from datalogger example
#include <SD.h>
//datecalc for datalogger shield
#include <Wire.h>
#include "RTClib.h"
#include <SoftwareSerial.h>
#include <String.h>

RTC_DS1307 rtc;

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 10; 
SoftwareSerial mySerial(7,8);

void setup()
{
  Serial.begin(9600);
  pinMode(RAIN_GAUGE_PIN, INPUT);
  digitalWrite(RAIN_GAUGE_PIN,HIGH);  // Turn on the internal Pull Up Resistor
  attachInterrupt(RAIN_GAUGE_INT,rainGageClick,FALLING);

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
  mySerial.begin(19200);
  delay(500);
  dataString = "Program reset at: ";
  recordRainandTime();

}

//loop just checks the time, determines whether to calculate rainfall,
// and then does the math and prints the results.
void loop()
{
  //millis() and micros() count time since the arduino was last restarted
   time = millis();  //get the time in milliseconds since last restart

   if (time >= nextCalcRain) {  //has more than 5 seconds occurred since we last calculated rainfall?
      getUnitRain();  //call getUnitRain() and calculate rainfall total over this time (5 seconds)
      //at this point, if we're here in the if statement, we jump down to the getUnitRain subroutine
      //  we then return to this point after that routine finishes executing.
      nextCalcRain = time + MSECS_CALC_RAIN;  //calculated when to measure rainfall again: current time + 5 seconds
         
      if (rain2 > 0) { //only print if some rainfall was actually logged
        recordRainandTime();
      }
   }
}

//This routine takes the number of clicks and multiplies it by 
//  the tipper volume to get a rain volume.
//double getUnitRain() I removed the return in favor of a 
//  global - fwb4june14
void getUnitRain()
{
// for checking the rain count Serial.print(rain_count); //debugging statements, uncomment to use
//  Serial.print("\n");                                  //add a carriage return, linefeed
  unsigned long reading=rain_count; //make a copy of rain count (number of clicks) in case it changes.  reading is local in scope
  rain_count=0;                      //zero the number of clicks for the next time period
//  at this point, rain_count could be modified by our ISR and we would be safe
  unit_rain=reading*RAIN_FACTOR;  //unit_rain is # clicks * rain depth per click.  I had a double mis-declaration here in the original
  rain2 = int(unit_rain * 100);  //rain2 converts unit_rain to an integer to tack onto the string to print. 
/*  return unit_rain;*/  //program used to return unit_rain here.  I changed it to global.
  Serial.print("Rain Volume: ");  //print a text header:
  Serial.print(unit_rain);        //print the rain depth
  Serial.print(" inches\n");      //print in inches
//  dataString = "";            //setup a variable to hold for printing
//  dataString.concat(time);    //object.operator(variable).  We just switched to a dataString object
//  dataString.concat(",");     //add a comma. You are adding text so you have to wrap it in quotes (dbl or sgl)
//  dataString.concat(rain2);   //add the rain depth converted to integer
//  dataString.concat(",");     //add another comma
//  dataString += ("hundredths of an inch");  //add a text note to print telling us its inches * 100
//  unit_rain = 0;               //reset the rain depth counter.  it's stored in a string to print from loop()
}

void rainGageClick()
{
  //declare a local variable to tell us the time since the last time we checked
    unsigned long thisTime=micros()-rain_last;  //by definition, micros() is an unsigned long
    rain_last=micros(); //set the rain_last time check to the current time
    if(thisTime>500)  //if its been more than 500 microseconds since the interrupt was called, log a click
    {                 // there are 1,000 microseconds in a millisecond, 1000 milliseconds in a second, so the interrupt is fast!
      rain_count++;  //count the number of clicks.  This is a volatile.
    }
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
    dataString.concat(rain2);
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




