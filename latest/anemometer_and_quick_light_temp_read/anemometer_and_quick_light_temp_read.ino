#define LIGHT_READ 5
#define TEMP_READ 3
#define WINDVANE_PIN 2
#define WV_INT 0
#define MSECS_TO_CALC 5000
//WIND_FACTOR should be rotations(clicks)/MSECS_TO_CALC = rot/time, 
//   or circumference/time = 2*pi*r / time.  
//The r should be measured to the center of the cup/ping-pong ball.
// in my case, this is 3": 3*3.14*2 = inches / MSECS_TO_CALC/1000=(secs)
//  inches per second to miles per hour = * 0.0568
//  3*3.14*2/12/5280*60*60 = 1.07
#define WIND_FACTOR 1.07 //change 
// the 
int temp = 0;
int temp2 = 0;
int light = 0;
float realtemp = 0.0;
volatile unsigned long wind_clicks = 0;
unsigned long nextCalc = 0;
unsigned long time = 0;
unsigned long wind_last = 0;
float windSpeed = 0.0;
int windSpeed2 = 0;
String dataString = "";

void setup () {
  Serial.begin(9600);
  Serial.println("Starting setup");
  pinMode (WINDVANE_PIN, INPUT);  //set the windvane to be an input
  digitalWrite(WINDVANE_PIN, HIGH); //pull-up the windvane input
  attachInterrupt(WV_INT,windVaneClick,FALLING);
  Serial.println("Done in Setup");
  }
  
void loop () {
    //millis() and micros() count time since the arduino was last restarted
   time = millis();  //get the time in milliseconds since last restart

   if (time >= nextCalc) {  //has more than 5 seconds occurred since we last calculated rainfall?
      getWindSpeed();  //call getUnitRain() and calculate rainfall total over this time (5 seconds)
      //at this point, if we're here in the if statement, we jump down to the getUnitRain subroutine
      //  we then return to this point after that routine finishes executing.
      calc_light_and_temp();
      nextCalc = time + MSECS_TO_CALC;  //calculated when to measure rainfall again: current time + 5 seconds
   }        
}

void calc_light_and_temp() {
    temp = analogRead(3);
//  temp2 = map(temp,20,409,0,342);
  Serial.println(temp);
  realtemp = ((temp - 20.) * (342. / 389.)) - 40.;
  Serial.println(realtemp);
  Serial.print("The Temperature is: ");
  Serial.println(realtemp);
  light = analogRead(5);
  Serial.print("The light level is :");
  Serial.println(light);
  delay(5000);
}

void getWindSpeed()
{
// for checking the rain count Serial.print(rain_count); //debugging statements, uncomment to use
//  Serial.print("\n");                                  //add a carriage return, linefeed
  unsigned long reading=wind_clicks; //make a copy of rain count (number of clicks) in case it changes.  reading is local in scope
  wind_clicks=0;                      //zero the number of clicks for the next time period
  windSpeed = float(reading)*WIND_FACTOR;  
  windSpeed2 = int(windSpeed * 100);  
  Serial.print("Wind Speed: ");  //print a text header:
  Serial.print(windSpeed);        //print the windSpeed
  Serial.print(" miles per hour\n");      //units
  dataString = "";            //setup a variable to hold for printing
  dataString.concat(time);    //object.operator(variable).  We just switched to a dataString object
  dataString.concat(",");     //add a comma. You are adding text so you have to wrap it in quotes (dbl or sgl)
  dataString.concat(windSpeed2);   //add the rain depth converted to integer
  dataString.concat(",");     //add another comma
  dataString += ("100*inches\n");  //add a text note to print telling us its inches * 100
  windSpeed = 0;               //reset the rain depth counter.  it's stored in a string to print from loop()
}

void windVaneClick()
{
  //declare a local variable to tell us the time since the last time we checked
    unsigned long thisTime=micros()-wind_last;  //by definition, micros() is an unsigned long
    wind_last=micros(); //set the rain_last time check to the current time
    if(thisTime>500)  //if its been more than 500 microseconds since the interrupt was called, log a click
    {                 // there are 1,000 microseconds in a millisecond, 1000 milliseconds in a second, so the interrupt is fast!
      wind_clicks++;  //count the number of clicks.  This is a volatile.
    }
}


