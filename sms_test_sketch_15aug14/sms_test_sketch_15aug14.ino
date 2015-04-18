/*Note: this code is a demo for how to using GPRS Shield to send SMS message and dial a voice call.

The microcontrollers Digital Pin 7 and hence allow unhindered communication with GPRS Shield using SoftSerial Library. 
IDE: Arduino 1.0 or later */
 
 
#include <SoftwareSerial.h>
#include <String.h>
 
SoftwareSerial mySerial(7,8);
 
void setup()
{
  mySerial.begin(19200);               // the GPRS baud rate   
  Serial.begin(19200);    // the GPRS baud rate 
  delay(500);
}
 
void loop()
{
  //after start up the program, you can using terminal to connect the serial of gprs shield,
  //if you input 't' in the terminal, the program will execute SendTextMessage(), it will show how to send a sms message,
  //if input 'd' in the terminal, it will execute DialVoiceCall(), etc.
 
  if (Serial.available())
    switch(Serial.read())
   {
     case 't':
       SendTextMessage();
       break;
     case 'l':
       listSMS();
       break;
   } 
  if (mySerial.available())
    Serial.write(mySerial.read());
}
 
///SendTextMessage()
///this function is to send a sms message
void SendTextMessage()
{
  mySerial.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  mySerial.println("AT + CMGS = \"+17065139486\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  mySerial.println("CATS!!!!! ?");//the content of the message
  delay(100);
  mySerial.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  mySerial.println();
}
 

void ShowSerialData()
{
  while(mySerial.available()!=0)
    Serial.write(mySerial.read());
}

void listSMS()
{
//    Read a SMS in index # 1;
//  mySerial.println("AT+CMGR=2\r");

//Read list of SMS messages in memory;
   mySerial.println("AT+CMGL=\"ALL\"");
}
