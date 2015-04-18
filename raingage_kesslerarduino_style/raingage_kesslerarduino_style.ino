#define RAIN_GAUGE_PIN 2
#define RAIN_GAUGE_INT 0
#define RAIN_FACTOR 0.10
// How often we want to calculate rain
#define MSECS_CALC_RAIN 5000

volatile unsigned long rain_count=0;
volatile unsigned long rain_last=0;
volatile unsigned long nextCalcRain=0;
volatile unsigned long unit_rain = 0 ;
volatile unsigned long time = 0;
volatile unsigned long rain2 = 0;

//from datalogger example
#include <SD.h>

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 4;
 // make a string for assembling the data to log:
String dataString = "";
  
void setup()
{
  Serial.begin(9600);
  pinMode(RAIN_GAUGE_PIN, INPUT);
  digitalWrite(RAIN_GAUGE_PIN,HIGH);  // Turn on the internal Pull Up Resistor
  attachInterrupt(RAIN_GAUGE_INT,rainGageClick,FALLING);
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

}


void loop()
{
   time = millis();

   if (time >= nextCalcRain) {
      getUnitRain();
      nextCalcRain = time + MSECS_CALC_RAIN;

  if (rain2 > 0) { //some rainfall was actually logged
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
      Serial.println("error opening datalog.txt");
    } 
    dataFile.close();
  }


   }

  
}

   
//double getUnitRain() I removed the return in favor of a volatile - fwb4june14
void getUnitRain()
{
 // for checking the rain count Serial.print(rain_count);
//  Serial.print("\n");
  unsigned long reading=rain_count;
  rain_count=0;
  double unit_rain=reading*RAIN_FACTOR;
  rain2 = int(unit_rain * 100);
/*  return unit_rain;*/
  Serial.print("Rain Volume: ");
  Serial.print(unit_rain);
  Serial.print(" inches\n");
  dataString = "";
  dataString.concat(time);
  dataString.concat(",");
  dataString.concat(rain2);
  dataString.concat(",");
  dataString += ("100*inches\n");
  unit_rain = 0;    
}

void rainGageClick()
{
    long thisTime=micros()-rain_last;
    rain_last=micros();
    if(thisTime>500)
    {
      rain_count++;
    }
}



