#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include "E5_Module.h"
#include "gps.h"
#include "ui.h"
#include "testeur.h"
#include "SqQueue.h"

TFT_eSprite spr = TFT_eSprite(&tft);  // Sprite

void setup(void) {
  Serial.begin(115200);
//  while (!Serial);
//  pinMode(LED_BUILTIN, OUTPUT);
//  digitalWrite(LED_BUILTIN, HIGH);

  init_softSerial();
    
  Serial1.begin(9600);
  while (!Serial1);
  Serial.print("E5 LORAWAN TEST\r\n");
//  Init_E5_Mode();
  initState();
  initScreen();
  Init_E5_Mode();
  InitQueue(&SqQueueRssi);
  InitQueue(&SqQueueSnr);
}
void loop(void)
{
  static long cTime = 5 * 60 * 1000;//0;
  static long batUpdateTime = 0;

  long sTime = millis();
  bool fireMessage = false;
  GetGpsInfoPolling();
  GetSerialDataPolling();
  UpdateGpsInfo();
  refresUI();
  switch ( ui.selected_mode ) {
    case MODE_MANUAL:
      if ( ui.hasClick)// && canLoRaSend() ) 
      { 
        Serial.println("MANUAL Send");
        fireMessage = true;
        ui.hasClick = false;
      }
      break;
    case MODE_AUTO_1MIN:
      if ( cTime >= ( 1 * 60 * 1000 ) ) 
      {
          cTime = 0;
          Serial.println("AUTO_1MIN Send");
          fireMessage = true;
      }
      break;
    case MODE_MAX_RATE:
      if(Module_Is_Busy() == false)
      {
          fireMessage = true;
      }
      break;
  }  
  if(fireMessage == true)
  {
      E5_Module_SendCmsgHexData();
  }
  delay(10);
  long duration = millis() - sTime;
  if ( duration < 0 ) duration = 10;
  cTime += duration;
}
