#include <SPI.h>          //Library for using SPI Communication 
#include <mcp2515.h>      //Library for using CAN Communication
#include "Arduino.h"
#include "heltec.h"       //OLED Driver at onboard dedicated pins (check Heltec-ESP32-WiFi documentation for more details)


struct can_frame canMsg;
MCP2515 mcp2515(2);

#define trig 16
#define echopin 17

void ExpedString(String message);
float ping_distance();
static unsigned long echoTime;
static float rangeCm;
String output ="the range in cm is: ";
////////test cases for various DLC's/////////
String test1="the8char"; //8 chars
String test2="abcde fghijklmno"; //16chars
String test3="The quick brown fox jumps over the lazy dog"; //random length 

void setup()                          //add initilisation error checking for mcp!
{
 
  pinMode(echopin, INPUT);
  digitalWrite(trig,LOW);// ready for pulse
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, false /*Serial disbaled*/);
  SPI.begin();              
  Heltec.display->clear();
  Heltec.display->drawString(0,0,"initiating canbus...");
  Heltec.display->display();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode();
  delay(3000);
  canMsg.can_id  = 0x038;       //ID set, leave it lower than reveiver ID for testing or manipulate as needed
  canMsg.can_dlc = 4;
  canMsg.data[0]=0x74;
  canMsg.data[1]=0x65;
  canMsg.data[2]=0x73;
  canMsg.data[3]=0x74;
  mcp2515.sendMessage(&canMsg);
}


void loop() 
{
  ExpedString(test1);
  delay(2000);
  ExpedString(test2);
  delay(1000);
  ExpedString(test3);
  delay(1000);
  Heltec.display->setFont(ArialMT_Plain_16);
  for(;;)
  {
     rangeCm=ping_distance();
     Heltec.display->clear();
     Heltec.display->drawStringMaxWidth(0,0,128,output+rangeCm);
     Heltec.display->display();
     delay(62);
  }
  
}

void ExpedString(String message)  // Function takes in string
{                                 // manipulates breakup in 8 bit chunks if needed
  String chunk="";                 // prepares CAN bus message and sends it.
  int fdlc= message.length();  
  int strindex=0;                 
  //next if block should proably be implemented as a case/switch statement 
  
  if(fdlc<=8)                     //fdlc of 8 or less case, adjust dlc to match and populate the frame as needed also takes care of 0 state
  {
    canMsg.can_dlc = fdlc;
    for(int k=0;k<fdlc;k++)
    {
      canMsg.data[k]=message[k];
      strindex++;
      chunk=chunk+message[k];
    }
    if (strindex!=0)//avoid empty strings 
    {
       Heltec.display->clear();
       Heltec.display->drawStringMaxWidth(0,0,128,chunk);
       Heltec.display->display();
      mcp2515.sendMessage(&canMsg);
      delay(1500);
      else if (fdlc==0)
      {
       Heltec.display->clear();
       Heltec.display->drawStringMaxWidth(0,0,128,"empty string supplied, no mesasge to be sent\n");
       Heltec.display->display();
      }
    }
  }
  else if(!(fdlc%8))              //fdlc is a multiple of 8 but not 8 case 
                                  //faster manipulation than "other" cases
  { 

    canMsg.can_dlc = 8;
    for(int i=fdlc/8;i>0;i--)
    {
      for(int z=0;z<8;z++)
      {
        canMsg.data[z]=message[strindex];
        strindex++;
        chunk=+message[strindex];
      }
      mcp2515.sendMessage(&canMsg);
      delay(1000);
    }
    
  }
  else                             //other cases
  {
    //int k=ceil(fdlc/8);
    //Serial.println("other case detected: decommposing main chunks found and remainder chars in the last chunk"); //maybe redesign to simply decompose into chunks and pass into chunk handler function 
    Heltec.display->clear();
    Heltec.display->drawStringMaxWidth(0,0,128,"other case detected: decommposing");
    Heltec.display->display();
    delay(1000);
    int l=fdlc%8;
    int counter=0;                 
    ;
    canMsg.can_dlc = 8;
    for(int i=0;i<floor(fdlc/8);i++) //find how many transimitions will have 8dlc
    {
      for(int z=0;z<8;z++)
      {
        canMsg.data[z]=message[strindex] ;
        chunk=chunk+message[strindex];
        strindex++;
      }
      Heltec.display->clear();
      Heltec.display->drawStringMaxWidth(0,0,128,chunk);
       Heltec.display->display();
      mcp2515.sendMessage(&canMsg);
      delay(1000);
    }
    canMsg.can_dlc = fdlc-(strindex);
    for(int i=0;strindex<fdlc;strindex++)// remaining bytes 
    {
      canMsg.data[i]=message[strindex];
      chunk=chunk+message[strindex];
      strindex++;
    }
    Heltec.display->clear();
    Heltec.display->drawStringMaxWidth(0,0,128,chunk);
    Heltec.display->display();
    mcp2515.sendMessage(&canMsg);
    delay(100);
  }
}

float ping_distance() // add imlpementation for transmission over CAN lines 
{
  float distanceCm;
  digitalWrite(trig,LOW) ;
  delayMicroseconds(2);
  digitalWrite(trig,HIGH) ;
  delayMicroseconds(10); // pulse 10 us on trigger pin
  echoTime = pulseIn(echopin, HIGH); //read echo resposonse in MICROseconds
  distanceCm=(echoTime*0.0340)/2;// multiply by the speed of sound in cm/us, divide by 2 for 1 way time 
  return distanceCm;
}
