//annotated 23 June 2014
//first, define constants.  Unlike variables, these values shouldn't 
//  change in the program
#define RAIN_GAUGE_PIN 2  //the pin used to read the reed switch
#define RAIN_GAUGE_INT 0  //the declaration of which interrupt.  Pin 2 = INT 0 
#define RAIN_FACTOR 0.01  //amount of rainfall per count / tip of the bucket
// How often we want to calculate rain
#define MSECS_CALC_RAIN 10000   //every 1 minute

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
int freeramresults = 0;
// make a string for assembling the data to log:
String dataString = "";

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

#include <SoftwareSerial.h>
// checking to see if necessary  #include <String.h>
SoftwareSerial mySerial(7, 8);
//SoftwareSerial Serial(0,1); 
void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
    pinMode(RAIN_GAUGE_PIN, INPUT);
  digitalWrite(RAIN_GAUGE_PIN,HIGH);  // Turn on the internal Pull Up Resistor
  attachInterrupt(RAIN_GAUGE_INT,rainGageClick,FALLING);

  mySerial.begin(19200);               // the GPRS baud rate   
  delay(500);
  }

//  Serial.println("Debug 1: mySerial and Serial started");
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
//  Serial.println("Debug 2: RTC passed");
//  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
//  Serial.println("Debug 3: SD card initialized.");
//  powerUpOrDown();  //turn on the shield
//  Serial.println("Debug 4: Shield powered up");
  freeramresults = freeRam();
  Serial.println(freeramresults);
  dataString = "PRA:12345678";
  recordRainandTime();
  freeramresults = freeRam();
  Serial.println(freeramresults);
//  Serial.flush();
//  Serial.println("W");
  //Serial.println("Debug 5: recordRainandTime rain once");
}

void loop()
{
  //millis() and micros() count time since the arduino was last restarted
   time = millis();  //get the time in milliseconds since last restart

   if (time >= nextCalcRain) {  //has more than 60 seconds occurred since we last calculated rainfall?
//      powerUpOrDown();  //turn on the shield
      getUnitRain();  //call getUnitRain() and calculate rainfall total over this time interval
      //at this point, if we're here in the if statement, we jump down to the getUnitRain subroutine
      //  we then return to this point after that routine finishes executing.
      nextCalcRain = time + MSECS_CALC_RAIN;  //calculated when to measure rainfall again: current time + 5 seconds
         
      if (rain2 > 0) { //only print if some rainfall was actually logged
        recordRainandTime();
      }
//      powerUpOrDown();  //turn off the shield
   }
}

//This routine takes the number of clicks and multiplies it by 
//  the tipper volume to get a rain volume.
//double getUnitRain() I removed the return in favor of a 
//  global - fwb4june14
int getUnitRain()
{
// for checking the rain count Serial.print(rain_count); //debugging statements, uncomment to use
//  Serial.print("\n");                                  //add a carriage return, linefeed
  unsigned long reading=rain_count; //make a copy of rain count (number of clicks) in case it changes.  reading is local in scope
  rain_count=0;                      //zero the number of clicks for the next time period
//  at this point, rain_count could be modified by our ISR and we would be safe
//  unit_rain=reading*RAIN_FACTOR;  //unit_rain is # clicks * rain depth per click.  I had a double mis-declaration here in the original
  rain2 = int((reading*RAIN_FACTOR) * 100);  //rain2 converts unit_rain to an integer to tack onto the string to print. 
/*  return unit_rain;*/  //program used to return unit_rain here.  I changed it to global.
//  return rain2;
  Serial.print("Rain Volume: ");  //print a text header:
  Serial.print(unit_rain);        //print the rain depth
  Serial.print(" inches\n");      //print in inches
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

    File dataFile = SD.open("spamhaus.txt", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);//print to the file
      Serial.println(dataString);  // print to the serial port too:
      dataFile.close();            // then close the file
    }  
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening spamhaus.txt");
    } 
    delay(500); //just giving the file a chance to close, not sure if its atomic
    SendTextEmail(); //email the results
    delay(5000);  //a delay is required or the program will not be able to execute the next modem command fwb-1aug14
    SendTextMessage(); //text the results
    dataString = ""; //reset the global variable
}

void SendTextEmail()
{
  mySerial.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
//  mySerial.println("AT + CMGS = \"+17065139486\"");//send sms message, be careful need to add a country code before the cellphone number
  mySerial.println("AT + CMGS = \"111\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  mySerial.print("rainfall@watergeorgia.com Rainfall:");//the content of the message
  delay(100);
  mySerial.print("\r");
  delay(100);
  mySerial.print("\r");
  delay(100);
  mySerial.print(dataString);//the content of the message
  delay(100);
  mySerial.print("\r");
  delay(100);
//  mySerial.println("Test Message");//the content of the message
//  delay(100);
  mySerial.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  mySerial.println();
  Serial.println(dataString);
  Serial.println("Finished Email");
}

///SendTextMessage()
///this function is to send a sms message
void SendTextMessage()
{
  mySerial.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  mySerial.println("AT + CMGS = \"+17064145535\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  mySerial.println(dataString);//the content of the message
  delay(100);
  mySerial.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  mySerial.println();
}

void powerUpOrDown()
{
  delay(5000); //added by me to give the board time to do its thing
  pinMode(9, OUTPUT); 
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
}

int freeRam ()
    {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }




