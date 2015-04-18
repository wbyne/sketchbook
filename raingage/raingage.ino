//annotated 23 June 2014
//first, define constants.  Unlike variables, these values shouldn't 
//  change in the program
#define RAIN_GAUGE_PIN 2  //the pin used to read the reed switch
#define RAIN_GAUGE_INT 0  //the declaration of which interrupt.  Pin 2 = INT 0 
#define RAIN_FACTOR 0.01  //amount of rainfall per count / tip of the bucket
// How often we want to calculate rain
#define MSECS_CALC_RAIN 60000   //every 5 minuteS
#define GPRS_VCC_IN A2  //the pin used to read the cell boards vcc input

//volatile is reserved for variables that may be changed outside of 
//  the program scope where they appear, so they are loaded into RAM
//  variables in Interrupt Service Routines (ISRs) should be volatile.
// These variables are global because they are declared outside of the
//  main program body.
volatile unsigned long rain_count=0;
unsigned long rain_last=0;
unsigned long nextCalcRain=0;
unsigned long time = 0;
unsigned int rain = 0;
unsigned int rain2 = 0;
unsigned int rain3 = 0;
unsigned long DateTimelong = 0;
unsigned long DateTimelong2 = 0;
unsigned long DateTimelong3 = 0;
unsigned int freeramresults = 0;
unsigned int ReportCounter = 0;
int GPRS_value = 0;

#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <SoftwareSerial.h>
#include <SPI.h> //had to include 22mar b/c wire lib couldn't find it

RTC_DS1307 rtc; //set up the rtc object for calling now, isrunning, begin, etc.
// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 10; 
// gprs setup from seeed website v1.0
SoftwareSerial GPRSSerial(7, 8);

void setup()
{
  Serial.begin(9600);
      pinMode(RAIN_GAUGE_PIN, INPUT);
      digitalWrite(RAIN_GAUGE_PIN,HIGH);  // Turn on the internal Pull Up Resistor
      attachInterrupt(RAIN_GAUGE_INT,rainGageClick,FALLING);
      pinMode(GPRS_VCC_IN, INPUT);
      //startup the pins for the gprs shield
      GPRSSerial.begin(19200);               // the GPRS baud rate   
      delay(500);
      #ifdef AVR
        Wire.begin();
      #else
        Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
      #endif
      rtc.begin(); //I don't think this does anything other than return 1, which you also check with isrunning.  Probably should 
                   // flag one and change the DateTimelong to 1000000000.
      pinMode(chipSelect, OUTPUT); //changed from 10 to variable 13aug14
        // see if the card is present and can be initialized:
      if (!SD.begin(chipSelect)) {
    //    Serial.println("Card failed, or not present");
        // don't do anything more:
        while(1) {
          Serial.println(F("ChipSelect Failed"));
          digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
          delay(2500);               // wait for a second
          digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
          delay(500);               // wait for a second
        }
      }
      GPRS_value = analogRead(GPRS_VCC_IN);
      Serial.print(GPRS_value);
      Serial.println(" should be off");
      freeramresults = freeRam();
      Serial.println(freeramresults);
      rain=999; //show that the program has reset.
      recordRainandTime();
      PowerUpOrDown(1); //power on the cell board
      delay(500); //just giving the file a chance to close, not sure if its atomic
      SendTextEmail(); //email the results
      delay(5000);  //a delay is required or the program will not be able to execute the next modem command fwb-1aug14
      SendTextMessage(); //text the results
      rain=0; //reset after 
      PowerUpOrDown(0); //power down the cell board
      freeramresults = freeRam();
      Serial.println(freeramresults);
}

//loop just checks the time, determines whether to calculate rainfall,
// and then does the math and prints the results.
void loop()
{
  //millis() and micros() count time since the arduino was last restarted
   time = millis();  //get the time in milliseconds since last restart

   if ((time - nextCalcRain) >= MSECS_CALC_RAIN) {  //has more than 60 seconds occurred since we last calculated rainfall?
//recommended by arduino TimingRollover and baldengineer.com (how do you reset millis)
// This works because they're both unsigned, and the math is done differently.
      getUnitRain();  //call getUnitRain() and calculate rainfall total over this time interval
      //at this point, if we're here in the if statement, we jump down to the getUnitRain subroutine
      //  we then return to this point after that routine finishes executing.
      nextCalcRain = time;   //calculated when to measure rainfall again: current time + 5 seconds
      if (rain > 0) { //only print if some rainfall was actually logged
        recordRainandTime();
        ReportCounter = ReportCounter + 1;
        
        if (ReportCounter >= 3) {
          PowerUpOrDown(1); //power up the cell board
          delay(500); //just giving the file a chance to close, not sure if its atomic
          SendTextEmail(); //email the results
          delay(5000);  //a delay is required or the program will not be able to execute the next modem command fwb-1aug14
          SendTextMessage(); //text the results
          PowerUpOrDown(0); //power down the cell board
          //reset counters
          ReportCounter = 0;          
          rain3=0;
          DateTimelong3=0;
          rain2=0;
          DateTimelong2=0;
          rain=0;
          DateTimelong=0;
        }

        rain3=rain2;
        DateTimelong3=DateTimelong2;
        rain2=rain;
        DateTimelong2=DateTimelong;
        freeramresults = freeRam();
        Serial.println(freeramresults);

      }
   }
}

//This routine takes the number of clicks and multiplies it by 
//  the tipper volume to get a rain volume.
void getUnitRain()
{
  unsigned long reading=rain_count; //make a copy of rain count (number of clicks) in case it changes.  reading is local in scope
  rain_count=0;                      //zero the number of clicks for the next time period
//  at this point, rain_count could be modified by our ISR and we would be safe
//  unit_rain=reading*RAIN_FACTOR;  //unit_rain is # clicks * rain depth per click.  I had a double mis-declaration here in the original
  rain = int((reading*RAIN_FACTOR) * 100);  //rainrain converts unit_rain to an integer to tack onto the string to print. 
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
    DateTimelong = mytime.month() *  100000000;
    DateTimelong = DateTimelong + mytime.day() *   1000000;
    unsigned long shortyear = mytime.year()%100;
    DateTimelong = DateTimelong + shortyear * 10000;
    DateTimelong = DateTimelong + mytime.hour() *      100;
    DateTimelong = DateTimelong + mytime.minute() *      1;
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    // supposedly this was changed in a later version so that multiple files can be open.
    File dataFile = SD.open("rainfall.txt", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.print(DateTimelong);//print to the file
      dataFile.println(rain);
      Serial.print(DateTimelong);  // print to the serial port too:
      Serial.println(rain);  // print to the serial port too:
      dataFile.close();            // then close the file
    }  
    // if the file isn't open, pop up an error:
    else {
      rain = rain+900; //let the program continue on but anything over 900 (and not 999) should raise an error that its not being written.
    } 
}

///SendTextEmail()
///this function is to send a sms message
void SendTextEmail()
{
  GPRSSerial.print(F("AT+CMGF=1\r"));    //Because we want to send the SMS in text mode
  delay(100);
//500 for t-mobile?  GPRSSerial.println("AT + CMGS = \"111\"");//send sms message, be careful need to add a country code before the cellphone number
  GPRSSerial.println(F("AT + CMGS = \"500\""));//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  GPRSSerial.println(F("rainfall@watergeorgia.com Rainfall "));//SPACES SEPARATE THE TO: and SUBJECT fields. If you don't add spaces, it'll jumble stuff up together. fwb-6Oct14
  delay(100);
  GPRSSerial.println(F("Rainfall occurring at:"));//the content of the message
  delay(100);
  GPRSSerial.print(DateTimelong3);//the content of the message
  delay(100);
  GPRSSerial.print(F(":"));//the content of the message
  delay(100);
  GPRSSerial.println(rain3);//the content of the message
  delay(100);
  GPRSSerial.print(DateTimelong2);//the content of the message
  delay(100);
  GPRSSerial.print(F(":"));//the content of the message
  delay(100);
  GPRSSerial.println(rain2);//the content of the message
  delay(100);
  GPRSSerial.print(DateTimelong);//the content of the message
  delay(100);
  GPRSSerial.print(F(":"));//the content of the message
  delay(100);
  GPRSSerial.println(rain);//the content of the message
  delay(100);
  GPRSSerial.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  GPRSSerial.println();
  Serial.println(F("Finished Email"));
}

///SendTextMessage()
///this function is to send a sms message
void SendTextMessage()
{
  GPRSSerial.print(F("AT+CMGF=1\r"));    //Because we want to send the SMS in text mode
  delay(100);
  GPRSSerial.println(F("AT + CMGS = \"+17064145535\""));//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  GPRSSerial.print(DateTimelong3);//the content of the message
  delay(100);
  GPRSSerial.print(F(":"));//the content of the message
  delay(100);
  GPRSSerial.println(rain3);//the content of the message
  delay(100);
  GPRSSerial.print(DateTimelong2);//the content of the message
  delay(100);
  GPRSSerial.print(F(":"));//the content of the message
  delay(100);
  GPRSSerial.println(rain2);//the content of the message
  delay(100);
  GPRSSerial.print(DateTimelong);//the content of the message
  delay(100);
  GPRSSerial.print(F(":"));//the content of the message
  delay(100);
  GPRSSerial.println(rain);//the content of the message
  delay(100);
  GPRSSerial.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  GPRSSerial.println();
}

void PowerUpOrDown(int on_off)
{
  GPRS_value = analogRead(GPRS_VCC_IN);
  Serial.print(GPRS_value);
  Serial.println (" 1");
  if (((GPRS_value == 0) && (on_off == 1)) || ((GPRS_value > 100) && (on_off == 0)))  { //cell is off and switch says turn it on or vice-versa
    delay(5000); //added by me to give the board time to do its thing
    pinMode(9, OUTPUT); 
    digitalWrite(9,LOW);
    delay(1000);
    digitalWrite(9,HIGH);
    delay(2000);
    GPRS_value = analogRead(GPRS_VCC_IN);
    Serial.print(GPRS_value);
    Serial.println (" 2");
    digitalWrite(9,LOW);
    delay(3000);
    GPRS_value = analogRead(GPRS_VCC_IN);
    Serial.print(GPRS_value);
    Serial.println(" 3");
  }
}

int freeRam ()
    {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }




