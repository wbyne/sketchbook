//poached from http://tronixstuff.com/2014/01/08/tutorial-arduino-and-sim900-gsm-modules/
// works but has an 18 character limit? fwb-22june14
#include <SoftwareSerial.h>
SoftwareSerial SIM900(7, 8);
 
char incoming_char=0;
 
void setup()
{
  Serial.begin(19200); // for serial monitor
  SIM900.begin(19200); // for GSM shield
  SIM900power();  // turn on shield
  delay(20000);  // give time to log on to network.
 
  SIM900.print("AT+CMGF=1\r");  // set SMS mode to text
  delay(100);
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  // blurt out contents of new SMS upon receipt to the GSM shield's serial out
  delay(100);
}
 
void SIM900power()
// software equivalent of pressing the GSM shield "power" button
{
//commented by me 22june14-fwb  digitalWrite(9, HIGH);
//  delay(1000);
//  digitalWrite(9, LOW);
//  delay(7000);
}
 
void loop()
{
//fwb added from developershome.com   SIM900.print("AT+CMGR=3\r");  // set SMS mode to text
//fwb   SIM900.print("AT+CMGL=\"ALL\r");
 // Now we simply display any text that the GSM shield sends out on the serial monitor
  if(SIM900.available() >0)
  {
    incoming_char=SIM900.read(); //Get the character from the cellular serial port.
    Serial.print(incoming_char); //Print the incoming character to the terminal.
  }
}
