#include <SPI.h>          //Library for using SPI Communication 
#include <mcp2515.h>      //Library for using CAN Communication
#include <Wire.h>         //I2c
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64
#define reset_oled -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, reset_oled);
struct can_frame canMsg;
MCP2515 mcp2515(7);
int globx=0,globy=0;



void setup() {
  SPI.begin();
  //Serial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));}
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); 
  mcp2515.setNormalMode();
  display.clearDisplay();
  display.setTextSize(2);                // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);   // Draw white text
  //display.invertDisplay(true);
  display.setCursor(20,20);  
  display.println("CAN BUS");
  display.setCursor(15,40);
  display.println("simulator");
  display.display();
  display.clearDisplay();
  delay(1000);
 // display.drawBitmap(0, 0, myBitmap, 128, 64, SSD1306_WHITE);
  //display.display();
 // long int k=strtol("thi sis text",NULL,16);
  //Serial.println(k);
}

void loop() {
   
   while(true)
   {
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) 
   
      {
      display.setCursor(20,20);
      display.clearDisplay();
      display.println("incoming can message");
      display.display();
      String output="";
      for(int i=0;i<canMsg.can_dlc;i++)
          {
             char letter= (char) canMsg.data[i];
              output=output+letter;
          }
      display.clearDisplay();
      display.setCursor(10, 0);
      display.setTextSize(2);
      display.println(output);
      display.display();
      }
      
   }
}
