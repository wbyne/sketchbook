int first = 2;
int second = 3;
int third = 4;
int fourth = 5;
int val = 0;
int val2 =0;
int val3 = 0;
int val4 =0;

#define mosfet 8

#define MSECS_CALC_RAIN 5000

volatile unsigned long rain_count = 0;
double unit_rain = 0;
unsigned long time = 0;
unsigned long rain_last=0;
unsigned long nextCalcRain = 0;
unsigned long rain2 = 0;

void setup(){
  Serial.begin(9600);
  
  pinMode(first,INPUT_PULLUP);
  attachInterrupt(0, rainclosed, FALLING); 
  pinMode(second, OUTPUT);
  pinMode(third, OUTPUT);
  pinMode(fourth, OUTPUT);  
  pinMode(mosfet, OUTPUT);
}

void loop(){
  time = millis();
  
  if (time >= nextCalcRain) {
    getUnitRain(),
    nextCalcRain = time + MSECS_CALC_RAIN;
  }
}

void getUnitRain(){
 
  val=digitalRead(first);
  val2=digitalRead(second);
  val3=digitalRead(third);
  val4=digitalRead(fourth);
    if(val < 1){
    //if(rain_count < 5){
      Serial.println("1st water level");
      delay(5000);
    }
  else{
    delay(7000);
    Serial.println("Dump rain");
    Serial.println("");
    digitalWrite(mosfet, HIGH);
    delay(5000);
    digitalWrite(mosfet, LOW);
    delay(5000);
    digitalWrite(mosfet, HIGH);
    delay(5000);
    digitalWrite(mosfet, LOW);
    delay(5000);
  }
}

void rainclosed(){
  unsigned long thisTime=millis()-rain_last;
  rain_last=millis();
  if(thisTime>500){
    rain_count++;
  }
}
      



  
          



        
    
