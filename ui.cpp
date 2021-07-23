/**
 * This file is part of Wio LoRaWan Field Tester.
 *
 *   Wio LoRaWan Field Tester is free software created by Paul Pinault aka disk91. 
 *   You can redistribute it and/or modify it under the terms of the 
 *   GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   any later version.
 *
 *  Wio LoRaWan Field Tester is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Wio LoRaWan Field Tester.  If not, see <https://www.gnu.org/licenses/>.
 */  
#include <Arduino.h>
#include "config.h"
#include "fonts.h"
#include "testeur.h"
//#include <TFT_eSPI.h>
//#include <LovyanGFX.hpp>
//#include <LGFX_AUTODETECT.hpp>
#include "ui.h"
#include "gps.h"
#include "E5_Module.h"
#include "SqQueue.h"

//TFT_eSPI tft;
//TFT_elcdite lcd = TFT_elcdite(&tft);

LGFX lcd;                 
LGFX_Sprite sprite(&lcd);


#define X_OFFSET 2
#define Y_OFFSET 0
#define X_SIZE   80
#define Y_SIZE   20
#define R_SIZE    4 
#define BOX_SPACING 2
//                    RRRRRGGGGGGBBBBB
#define TFT_GRAY    0b1010010100010000
#define TFT_GRAY10  0b0100001100001000
#define TFT_GRAY20  0b0010000110000100



#define HIST_X_OFFSET 2
#define HIST_Y_OFFSET 75
#define HIST_X_SIZE 315
#define HIST_X_TXTSIZE X_SIZE-3
#define HIST_Y_SIZE 160
#define HIST_X_BAR_OFFSET 50
#define HIST_X_BAR_SPACE 2

#define MAX_SNR 40
#define MAX_RETRY 8
#define MAX_HS 20

#define SELECTED_NONE   0
#define SELECTED_POWER  1
#define SELECTED_SF     2
#define SELECTED_RETRY  3

ui_t ui;
int index_Rssi = 0;

void initScreen() {
//  tft.begin();
  lcd.init();
  lcd.setRotation(1);
//lcd.setBrightness(128);
//lcd.setColorDepth(24);
  lcd.fillScreen(TFT_BLACK);

  // Totally unusefull so totally mandatory
  #ifdef WITH_SPLASH 
    lcd.drawRoundRect((320-200)/2,200,200,10,5,TFT_WHITE);
    for ( int i = 10 ; i < 100 ; i+=4 ) {
      lcd.fillRoundRect((320-200)/2+2,202,((204*i)/100),6,3,TFT_WHITE);
      // #ifdef WITH_SPLASH_HELIUM
      //   draw_splash_helium(HELIUM_XCENTER, (240-100)/2, i);
      // #endif
      #ifdef WITH_SPLASH_TTN  
        draw_splash_ttn(TTN_XCENTER, (240-85)/2, i);
      #endif
  }
//  delay(1500);
#endif
Init_E5_Mode();
  lcd.fillScreen(TFT_BLACK);

  if ( ! readConfig() ) 
  {
//    Serial.println("initScreen Defaulr para");
    ui.selected_display = DISPLAY_RSSI_HIST;//DISPLAY_DEVICE_INFO;//DISPLAY_RSSI_HIST;    
    storeConfig();
  }
  ui.selected_mode = MODE_MANUAL;
  ui.selected_menu = SELECTED_NONE;
  ui.displayed_state = UKN_STAT;
  ui.transmit_v = 255;
  ui.previous_display = DISPLAY_MAX;
  ui.lastWrId = MAXBUFFER;
  ui.hasClick = false;
  ui.alertMode = false;
  
  // draw mask
tst_setRegion(state.cRegion);
//  refreshPower(); 
//  refreshSf();
  //refreshRetry();
  refreshTotal();
//  refreshState();
  refreshMode();
  refreshLastFrame();
  switch ( ui.selected_display ) {
    case DISPLAY_RSSI_HIST:
      refreshRssiHist();
      break;
    case DISPLAY_SNR_HIST:
      refreshSnrHist();
      break;
    case DISPLAY_DEVICE_INFO:
      refreshDeviceInfo();
      break;
  }
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP); 
}

/**
 * Call on regular basis by the main loop
 * check the button status to update the user interface
 * 
 */
bool Clear_Data_Flag = false;
void Clear_Data()
{
    if((E5_Module_Data.SendNumber != 0) || (E5_Module_Data.RecvNumber != 0) ||
       (E5_Module_Data.rssi != 0) || (E5_Module_Data.snr != 0) || 
       (QueueEmpty(SqQueueRssi) != true) || (QueueEmpty(SqQueueRssi) != true))
    {
//        Serial.println("Clear Data");
        E5_Module_Data.SendNumber = 0;
        E5_Module_Data.RecvNumber = 0;
        E5_Module_Data.rssi = 0;
        E5_Module_Data.snr  = 0;
        ClearQueue(&SqQueueRssi);
        ClearQueue(&SqQueueSnr);
        state.hasRefreshed = true;    
    }
}
void Button_Detection(void){
  bool hasAction = false;
  bool forceRefresh = false;
  bool configHasChanged = false;
  uint8_t prev_select = ui.selected_menu;
  
  if (digitalRead(WIO_KEY_C) == LOW) {
    ui.selected_menu = ( prev_select == SELECTED_POWER )?SELECTED_NONE:SELECTED_POWER;
  } else if (digitalRead(WIO_KEY_B) == LOW) {
    ui.selected_menu = ( prev_select == SELECTED_SF )?SELECTED_NONE:SELECTED_SF;
  } else if (digitalRead(WIO_KEY_A) == LOW) {
    ui.selected_menu = ( prev_select == SELECTED_RETRY )?SELECTED_NONE:SELECTED_RETRY;
  } else if (digitalRead(WIO_5S_UP) == LOW) {
    switch ( ui.selected_menu ) {
      case SELECTED_POWER:
         tst_setPower(state.cPwr+2);
         E5_Module_Data.Pwr = state.cPwr;
         E5_Module_AT_Cmd("POWER");
         forceRefresh = true;
         configHasChanged = true;
         break;
      case SELECTED_SF:
         tst_setSf(state.cSf+1);
         E5_Module_Data.Sf = (e_Lora_Regional)(state.cSf);
         E5_Module_AT_Cmd("DR");  
         configHasChanged = true;
         forceRefresh = true;
         break;
      case SELECTED_RETRY:
         //tst_setRetry(state.cRetry+1);
         if(state.cTotal>=990)
         {
             state.cTotal = 990;
         }
         else
         {
             state.cTotal += 10;
         }         
         configHasChanged = true;
         forceRefresh = true;
         break;
       case SELECTED_NONE:
          if(ui.selected_display == DISPLAY_DEVICE_INFO)
          {
              tst_setRegion(state.cRegion+1);
              E5_Module_Data.Sf = (e_Lora_Regional)(state.cRegion);
              E5_Module_AT_Cmd("DR");  
              refreshDeviceInfo();
          }
          else
          {  
              ui.selected_mode = (ui.selected_mode+MODE_MAX+1)%MODE_MAX; 
              refreshMode();
          }             
          configHasChanged = true;
          hasAction = true;
      default:
         break;   
    }  
    hasAction=true;
  } else if (digitalRead(WIO_5S_DOWN) == LOW) {
    switch ( ui.selected_menu ) {
      case SELECTED_POWER:
         tst_setPower(state.cPwr-2);
         E5_Module_Data.Pwr = state.cPwr;
         E5_Module_AT_Cmd("POWER");
         configHasChanged = true;
         forceRefresh = true;
         break;
      case SELECTED_SF:
         tst_setSf(state.cSf-1);
         E5_Module_Data.Sf = (e_Lora_Regional)(state.cSf);
         E5_Module_AT_Cmd("DR"); 
         configHasChanged = true;
         forceRefresh = true;
         break;
      case SELECTED_RETRY:
         //tst_setRetry(state.cRetry-1);
         if(state.cTotal<=10)
         {
             state.cTotal = 10;
         }
         else
         {
             state.cTotal -= 10;
         }
         configHasChanged = true;
         forceRefresh = true;
         break;
      case SELECTED_NONE:
          if(ui.selected_display == DISPLAY_DEVICE_INFO)
          {
              tst_setRegion(state.cRegion-1);
              E5_Module_Data.Sf = (e_Lora_Regional)(state.cRegion);
              E5_Module_AT_Cmd("DR");           
              //Serial.print("Set Region --\r\n");
              refreshDeviceInfo();
          }
          else
          {  
              ui.selected_mode = (ui.selected_mode+MODE_MAX-1)%MODE_MAX; 
              refreshMode();
          } 
          //ui.selected_mode = (ui.selected_mode+MODE_MAX-1)%MODE_MAX;              
          configHasChanged = true;
          //refreshMode(true);
          hasAction = true;
      default:
         break;   
    }  
    hasAction=true;
  } else if (digitalRead(WIO_5S_RIGHT) == LOW) {
    configHasChanged = true;
    ui.selected_display = (ui.selected_display+1)%DISPLAY_MAX;
    hasAction=true;
//    Serial.print("ui.selected_display = ");
//    Serial.println(ui.selected_display);
//    Serial.println("WIO_5S_RIGHT");
  } else if (digitalRead(WIO_5S_LEFT) == LOW) {
    configHasChanged = true;
    ui.selected_display = (ui.selected_display+DISPLAY_MAX-1)%DISPLAY_MAX;
    hasAction=true;
//    Serial.print("ui.selected_display = ");
//    Serial.println(ui.selected_display);
//    Serial.println("WIO_5S_LEFT");
  } else if (digitalRead(WIO_5S_PRESS) == LOW) {
    if (( ui.selected_mode == MODE_MANUAL)) {
      ui.hasClick = true;
      hasAction = true;
    }
    else if(ui.selected_mode == MODE_CLEAR_DATA)
    {   
        //Serial.println("MODE_CLEAR_DATA");    
        hasAction = true;
        Clear_Data();
        if(Module_Is_Busy() == true)
        {
           //Serial.println("MODE_CLEAR_DATA...E5 Module IS BUSY"); 
           Clear_Data_Flag = true;
        }          
    }
  }
  if ( prev_select != ui.selected_menu || forceRefresh ) {
    if ( prev_select == SELECTED_POWER || ui.selected_menu == SELECTED_POWER ) {
      refreshPower();
    }
    if ( prev_select == SELECTED_SF || ui.selected_menu == SELECTED_SF ) {
      refreshSf();
    }
    if ( prev_select == SELECTED_RETRY || ui.selected_menu == SELECTED_RETRY ) {
      //refreshRetry();
      refreshTotal();
    }
    hasAction = true;
  }
  if ( configHasChanged ) {
    storeConfig();
  } 
  // avoid re-entreing
  if ( hasAction ) delay(300);  
}
void refresUI() {
  if((Clear_Data_Flag == true) && (Module_Is_Busy() == false))
  {
      //Serial.println("Second Clear Data.....");
      Clear_Data_Flag = false; 
      Clear_Data(); 
  }
  #ifdef WITH_LIPO
   if ( refreshLiPo() ) {
    lcd.fillScreen(TFT_BLACK);
    refreshPower(); 
    refreshSf();
    //refreshRetry();
    refreshTotal();
    refreshState();
    refreshMode();
    refreshLastFrame();
    ui.previous_display = DISPLAY_UNKNONW;
    state.hasRefreshed = true;
   }
   if ( ui.alertMode ) return;
  #endif

  Button_Detection();

  // refresh the Join state part
  refreshState();

  // refresh the GPS part
  #ifdef WITH_GPS
  refreshGps();
  #endif

  
  // refresh the graph history part
//  static bool lora_state = E5_Module_Data.Moudlue_Is_Ok;
  if ( state.hasRefreshed == true
    || ui.previous_display != ui.selected_display){ 
    //|| lora_state != E5_Module_Data.Moudlue_Is_Ok) {
    //lora_state = E5_Module_Data.Moudlue_Is_Ok;
    ui.lastWrId = state.writePtr;
    switch ( ui.selected_display ) {
      case DISPLAY_RSSI_HIST:
        //Serial.println("refreshRssiHist");
        refreshRssiHist();
        break;
      case DISPLAY_SNR_HIST:
        //Serial.println("refreshSnrHist");
        refreshSnrHist();
        break;
      //case DISPLAY_RETRY_HIST:
      //  refreshRetryHist();
      //  break;
      case DISPLAY_DEVICE_INFO:
        refreshDeviceInfo();
        break;
//      case DISPLAY_GPS_INFO:
//        refreshGpsInfo();
//        break;
      // case DISPLAY_TXRSSI:
      //   refreshTxRssi();
      //   break;
      // case DISPLAY_TXHS:
      //   refreshTxHs();
      //   break;
    }
    if ( state.hasRefreshed == true ) 
    {
      refreshLastFrame();
    }
    state.hasRefreshed = false;
  } 
//  Serial.print("ui.selected_display ：");
//  Serial.println(ui.selected_display);
  if(ui.selected_display == DISPLAY_GPS_INFO)
  {
      refreshGpsInfo();  
  } 
}

/*void refreshModeBacklight(bool state) {
  if(state == true){
      lcd.fillRoundRect(X_OFFSET+3*X_SIZE,Y_OFFSET,X_SIZE-5,Y_SIZE,R_SIZE,TFT_WHITE); 
  }
  else{
      lcd.fillRoundRect(X_OFFSET+3*X_SIZE,Y_OFFSET,X_SIZE-5,Y_SIZE,R_SIZE,TFT_GRAY);  
  }
}*/
/**
 * Select the way the messages are sent
 * On user action
 * Automatically
 */
void refreshMode() { 
  //lcd.createlcdite(320, 240);
  static bool state_tmp = false;
  int xOffset = X_OFFSET+3*X_SIZE;
  int yOffset = Y_OFFSET;
  lcd.fillRoundRect(xOffset,yOffset,X_SIZE-5,Y_SIZE,R_SIZE,TFT_WHITE);
  lcd.setTextColor(TFT_BLACK);    
  lcd.setFont(FS9);     // Select the orginal small TomThumb font
  switch ( ui.selected_mode ) {
    case MODE_MANUAL:
      lcd.drawString("Manual",xOffset+5,yOffset+3);  
      break;
    /*case MODE_AUTO_5MIN:
      lcd.drawString("Auto 5m",xOffset+5,yOffset+3);  
      break;*/
    case MODE_CLEAR_DATA:
      lcd.drawString("Clr Data",xOffset+5,yOffset+3);  
      break;
    case MODE_AUTO_1MIN:
      lcd.drawString("Auto 1m",xOffset+5,yOffset+3);  
      break;
    case MODE_MAX_RATE:
      lcd.drawString("Max rate",xOffset+5,yOffset+3);  
      break;
  }
  //lcd.pushlcdite(0, 0);  
}

/**
 * Update the last frame information on top of the screen
 */
void refreshLastFrame() 
{
  //lcd.createlcdite(320, 240);
  int xOffset = X_OFFSET;
  int yOffset = Y_OFFSET+Y_SIZE+2;
  lcd.fillRect(xOffset,yOffset,3*X_SIZE,Y_SIZE,TFT_BLACK);
  lcd.drawRoundRect(xOffset,yOffset,3*X_SIZE,Y_SIZE,R_SIZE,TFT_WHITE);
  lcd.fillRect(xOffset+X_SIZE-3,yOffset+Y_SIZE,3*X_SIZE,Y_SIZE,TFT_BLACK);
  lcd.drawRoundRect(xOffset+X_SIZE-3,yOffset+Y_SIZE,3*X_SIZE,Y_SIZE,R_SIZE,TFT_WHITE);
  int idx = getLastIndexWritten();
  lcd.setFont(FS9);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  char tmp[100];
  if(E5_Module_Data.rssi == 0 && E5_Module_Data.snr == 0)
  {
      lcd.setTextColor(TFT_RED, TFT_BLACK);
      sprintf(tmp,"NO DATA");      
  }
  else
  {
      lcd.setTextColor(TFT_GREEN, TFT_BLACK);
      sprintf(tmp,"RSSI:%-4ddBm   SNR:%-4ddB",E5_Module_Data.rssi,E5_Module_Data.snr);
  }
  lcd.drawString(tmp,xOffset+3,yOffset+2);     
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  sprintf(tmp,"Send:%-3d Recv:%-3d PER:%-3d%%",E5_Module_Data.SendNumber,E5_Module_Data.RecvNumber,((E5_Module_Data.SendNumber-E5_Module_Data.RecvNumber)*100)/state.cTotal);
  lcd.drawString(tmp,xOffset+X_SIZE+2,yOffset+Y_SIZE+2); 
  //lcd.pushlcdite(0, 0);  
}

/**
 * Update the current state display to see on the corner right / top 
 * the current tester action
 */
void refreshState() {
  //lcd.createlcdite(320, 240);
  int xOffset = X_OFFSET+3*X_SIZE;
  int yOffset = Y_OFFSET+Y_SIZE+2;
  if ( ui.displayed_state != E5_Module_Data.State ) {
    ui.displayed_state = E5_Module_Data.State;
    lcd.fillRect(xOffset,yOffset,X_SIZE-BOX_SPACING,Y_SIZE,TFT_BLACK);
    lcd.setTextSize(1);
    switch ( ui.displayed_state ) {
      case NOT_JOINED:
        lcd.setTextColor(TFT_RED, TFT_BLACK);
        lcd.drawString("Disc",xOffset+3,yOffset+2);   
        break;
      case JOIN_FAILED:
        lcd.setTextColor(TFT_RED, TFT_BLACK);
        lcd.drawString("Fail",xOffset+3,yOffset+2);   
        break;
      case JOINED:
        lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        lcd.drawString("Conn",xOffset+3,yOffset+2);      
        break;
      case JOINING:
        ui.transmit_v = 255; 
        lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
        lcd.drawString("Join",xOffset+3,yOffset+2); 
        break;
      case IN_TX:
        lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        lcd.drawString("Tx",xOffset+3,yOffset+2); 
        break;
      case IN_RPT:
        lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
        lcd.drawString("Tx",xOffset+3,yOffset+2); 
        break;
      case DWNLINK:
        lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        lcd.drawString("Dwn",xOffset+3,yOffset+2);   
        break;
      case DWNLINK_FAILED:
        lcd.setTextColor(TFT_RED, TFT_BLACK);
        lcd.drawString("Dwn",xOffset+3,yOffset+2);   
        break;        
    }
  }
  //lcd.pushlcdite(0, 0);
}

void refreshRegion(){
  //lcd.createlcdite(320, 240);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setFont(FS9); 
  lcd.fillRoundRect(12+80,95-2,60,18,4,TFT_WHITE);//TFT_GRAY // TFT_WHITE
  lcd.drawString("EU868",12+84,95);  
  //lcd.pushlcdite(0, 0);
}
void refreshPower() {
  //lcd.createlcdite(320, 240);
//  //lcd.createlcdite(100, 30);
  uint16_t color = (ui.selected_menu == SELECTED_POWER)?TFT_WHITE:TFT_GRAY;
  lcd.fillRoundRect(X_OFFSET,Y_OFFSET,X_SIZE-BOX_SPACING,Y_SIZE,R_SIZE,color);
  lcd.setTextColor(TFT_BLACK);
  lcd.setFont(FS9);     // Select the orginal small TomThumb font
//  lcd.setFont(&fonts::FreeMono9pt7b);
//  lcd.setFont(&fonts::FreeMonoBoldOblique9pt7b);
//lcd.setFont(&fonts::FreeSansOblique9pt7b);
  char sPower[10];
  sprintf(sPower,"+%02d dBm",state.cPwr); 
  lcd.drawString(sPower,X_OFFSET+5,Y_OFFSET+3);
  //lcd.pushlcdite(0, 0);
}

void refreshSf() {
  //lcd.createlcdite(320, 240);
//  //lcd.createlcdite(100, 30);
  int xOffset = X_OFFSET+1*X_SIZE;
  uint16_t color = (ui.selected_menu == SELECTED_SF)?TFT_WHITE:TFT_GRAY;
  lcd.fillRoundRect(xOffset,Y_OFFSET,X_SIZE-BOX_SPACING,Y_SIZE,R_SIZE,color);
  lcd.setTextColor(TFT_BLACK);
  lcd.setFont(FS9);//);     // Select the orginal small TomThumb font
//  lcd.setTextFont(2);
//  lcd.setTextSize(1);
//  lcd.setFont(&fonts::FreeMonoBold9pt7b);√
//lcd.setFont(&fonts::FreeSans9pt7b);√
//lcd.setFont(&fonts::FreeSansBoldOblique9pt7b);
  char sSf[10];
  if((state.cRegion == US915) || (state.cRegion == US915HYBRID)){
     sprintf(sSf,"SF   %02d",state.cSf-2); 
  }
  else{
     sprintf(sSf,"SF   %02d",state.cSf);     
  }
  lcd.drawString(sSf,xOffset+5,Y_OFFSET+3);
  //lcd.pushlcdite(0, 0);
//  lcd.print(sSf);
}

//void refreshRetry() {
//  //lcd.createlcdite(320, 240);
//  int xOffset = X_OFFSET+2*X_SIZE;
//  uint16_t color = (ui.selected_menu == SELECTED_RETRY)?TFT_WHITE:TFT_GRAY;
//  lcd.fillRoundRect(xOffset,Y_OFFSET,X_SIZE-BOX_SPACING,Y_SIZE,R_SIZE,color);
//  lcd.setTextColor(TFT_BLACK);
//  lcd.setFont(FS9);     // Select the orginal small TomThumb font
//  char sRetry[10];
//  sprintf(sRetry,"Retry  %01d",state.cRetry); 
//  lcd.drawString(sRetry,xOffset+5,Y_OFFSET+3);
//  //lcd.pushlcdite(0, 0);
//}

void refreshTotal(){
  //lcd.createlcdite(320, 240);
//  //lcd.createlcdite(100, 30);
  int xOffset = X_OFFSET+2*X_SIZE;
  uint16_t color = (ui.selected_menu == SELECTED_RETRY)?TFT_WHITE:TFT_GRAY;
  lcd.fillRoundRect(xOffset,Y_OFFSET,X_SIZE-BOX_SPACING,Y_SIZE,R_SIZE,color);
  lcd.setTextColor(TFT_BLACK);
  lcd.setFont(FS9);     // Select the orginal small TomThumb font
//  lcd.setFont(&fonts::FreeMonoOblique9pt7b);
//  lcd.setFont(&fonts::FreeSansBold9pt7b);
//  lcd.setFont(&fonts::FreeSerif9pt7b); √√
  char sRetry[10];
  sprintf(sRetry,"Total %3d",state.cTotal); 
  //Serial.print("state.cTotal = "); 
  //Serial.println(state.cTotal);
  lcd.drawString(sRetry,xOffset+5,Y_OFFSET+3);
  //lcd.pushlcdite(0, 0);
}




void refreshRssiHist() {
  //lcd.createlcdite(320, 240);
  // No need to refresh everytime
  if ( ui.previous_display != ui.selected_display ) {
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET-18,HIST_X_TXTSIZE,18,TFT_BLACK);
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,TFT_BLACK);
    lcd.setFont(FM9);    
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Rx Rssi",HIST_X_OFFSET,HIST_Y_OFFSET-18);
    lcd.drawRoundRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,R_SIZE,TFT_WHITE);
    ui.previous_display = ui.selected_display;
  }

  // clean the bar
  int xSz = (HIST_X_SIZE - (HIST_X_OFFSET+HIST_X_BAR_OFFSET + MAXBUFFER*HIST_X_BAR_SPACE)) / MAXBUFFER;
  int xOffset = HIST_X_OFFSET+HIST_X_SIZE-xSz-HIST_X_BAR_SPACE;
  for ( int i = 0 ; i < MAXBUFFER ; i++ ) {
     lcd.fillRect(xOffset,HIST_Y_OFFSET+1,xSz,154,TFT_BLACK);
     xOffset -= xSz + HIST_X_BAR_SPACE;
  }
  // Redraw lines
  lcd.drawLine(HIST_X_OFFSET+2,HIST_Y_OFFSET+10,HIST_X_SIZE-2,HIST_Y_OFFSET+10,TFT_GRAY);  
  for ( int i = 20 ; i < HIST_Y_SIZE ; i+=20 ) {
    if ( i % 40 == 0 ) {
      char sTmp[10];
      sprintf(sTmp,"-%d",i); 
      lcd.setFont(FF25);    
      lcd.setTextColor(TFT_GRAY);
      lcd.drawString(sTmp,HIST_X_OFFSET+5,HIST_Y_OFFSET-5+i);
    }
    lcd.drawLine(HIST_X_OFFSET+2,HIST_Y_OFFSET+10+i,HIST_X_SIZE-2,HIST_Y_OFFSET+10+i,TFT_GRAY20);
  }
  xOffset = HIST_X_OFFSET+HIST_X_SIZE-xSz-HIST_X_BAR_SPACE;
  int i = SqQueueRssi.front;
  int length = QueueLength(SqQueueRssi);
  if(length <= 0)
  {
     return; 
  }
  xOffset -= (xSz + HIST_X_BAR_SPACE)*(length-1);
  while(i!=SqQueueRssi.rear)
  {
  int rssi = SqQueueRssi.data[i];//data[i];
  i=(i+1)%MAXSIZE;
  uint16_t color = TFT_GREEN;
  if ( rssi > 8 ) rssi = 8; // avoid drawing over the graph.
  if ( rssi < -125 ) color = TFT_RED;
  else if (rssi < -100 ) color = TFT_ORANGE;
  else if (rssi < -80 ) color = TFT_DARKGREEN;
  if ( rssi < 0 ) {
    lcd.fillRect(xOffset,HIST_Y_OFFSET+10,xSz,-rssi,color);
  } else {
    lcd.fillRect(xOffset,HIST_Y_OFFSET+10-rssi,xSz,rssi,color);         
  }
  xOffset += xSz + HIST_X_BAR_SPACE;
  }  
  //lcd.pushlcdite(0, 0);  
}
void refreshSnrHist() {
  //lcd.createlcdite(320, 240);
    // No need to refresh everytime
    if ( ui.previous_display != ui.selected_display ) {
      lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET-18,HIST_X_TXTSIZE,18,TFT_BLACK);
      lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,TFT_BLACK);
      lcd.setFont(FM9);    
      lcd.setTextColor(TFT_WHITE);
      lcd.drawString("Rx Snr",HIST_X_OFFSET,HIST_Y_OFFSET-18);
      lcd.drawRoundRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,R_SIZE,TFT_WHITE);
      ui.previous_display = ui.selected_display;
    }
  
    // clean the bar
    int xSz = (HIST_X_SIZE - (HIST_X_OFFSET+HIST_X_BAR_OFFSET + MAXBUFFER*HIST_X_BAR_SPACE)) / MAXBUFFER;
    int xOffset = HIST_X_OFFSET+HIST_X_SIZE-xSz-HIST_X_BAR_SPACE;
    for ( int i = 0 ; i < MAXBUFFER ; i++ ) {
       lcd.fillRect(xOffset,HIST_Y_OFFSET+2,xSz,HIST_Y_SIZE-4,TFT_BLACK);
       xOffset -= xSz + HIST_X_BAR_SPACE;
    }
    // Redraw lines
    int yOffset = HIST_Y_OFFSET+HIST_Y_SIZE-10;
    int yStep10 = ((HIST_Y_OFFSET+HIST_Y_SIZE-10) - ( HIST_Y_OFFSET - 5 )) / (MAX_SNR/10);  // step for 10 SNR
    for ( int i = 10 ; i < MAX_SNR ; i+= 10 ) {
      int y = yOffset-(yStep10*i)/10;
      if ( i % 10 == 0 ) {
        char sTmp[10];
        if(i == 10)
        {
            sprintf(sTmp,"-10");
        }
        else if(i == 20)
        {
            sprintf(sTmp,"0"); 
        }
        else if(i == 30)
        {
            sprintf(sTmp,"10");   
        } 
        lcd.setFont(FF25);    
        lcd.setTextColor(TFT_GRAY);
        lcd.drawString(sTmp,HIST_X_OFFSET+5,y-15);
      }
      lcd.drawLine(HIST_X_OFFSET+2,y,HIST_X_SIZE-2,y,TFT_GRAY20);
    }
    lcd.drawLine(HIST_X_OFFSET+2,yOffset-(yStep10*20)/10,HIST_X_SIZE-2,yOffset-(yStep10*20)/10,TFT_GRAY);   
    xOffset = HIST_X_OFFSET+HIST_X_SIZE-xSz-HIST_X_BAR_SPACE;
    yOffset -= (yStep10*20)/10;
    int i = SqQueueSnr.front;
    int length = QueueLength(SqQueueSnr);
    if(length <= 0)
    {
       return; 
    }
    xOffset -= (xSz + HIST_X_BAR_SPACE)*(length-1);
    while(i!=SqQueueRssi.rear)
    {
        int snr = SqQueueSnr.data[i];//data[i];
        i=(i+1)%MAXSIZE;
        uint16_t color = TFT_GREEN;
        if ( snr < 0 ) color = TFT_RED;
        else if (snr < 10 ) color = TFT_ORANGE;
        else if (snr < 20 ) color = TFT_DARKGREEN;
        if(snr >= 20)
        {
            snr = 20;  
        }else if(snr <= -20)
        {
            snr = -20;  
        }               
        if(snr<0)
        {
           lcd.fillRect(xOffset,yOffset,xSz,- (snr*yStep10)/10,color);  
        }
        else
        {
            lcd.fillRect(xOffset,yOffset-(snr*yStep10)/10,xSz,(snr*yStep10)/10,color);
        }
        xOffset += xSz + HIST_X_BAR_SPACE;
    }
   //lcd.pushlcdite(0, 0); 
}  

void refreshDeviceInfo()
{
  //lcd.createlcdite(320, 240);
  int Length = 60;
  static bool lora_state = false; 
  if ( lora_state != E5_Module_Data.Moudlue_Is_Ok || ui.previous_display != ui.selected_display ){
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET-18,HIST_X_TXTSIZE,18,TFT_BLACK);
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,TFT_BLACK);
    lcd.setFont(FM9);    
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Device",HIST_X_OFFSET,HIST_Y_OFFSET-18);
    lcd.drawRoundRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,R_SIZE,TFT_WHITE);
    //state = E5_Module_Data.Moudlue_Is_Ok;
  }
//E5_Module_Data.Moudlue_Is_Ok = true;
  if(lora_state != E5_Module_Data.Moudlue_Is_Ok || ui.previous_display != ui.selected_display)
  {
    // clean the bar
    int xSz = (HIST_X_SIZE - (HIST_X_OFFSET+HIST_X_BAR_OFFSET + MAXBUFFER*HIST_X_BAR_SPACE)) / MAXBUFFER;
    int xOffset = HIST_X_OFFSET+HIST_X_SIZE-xSz-HIST_X_BAR_SPACE;
    for ( int i = 0 ; i < MAXBUFFER ; i++ ) {
       lcd.fillRect(xOffset,HIST_Y_OFFSET+2,xSz,HIST_Y_SIZE-4,TFT_BLACK);
       xOffset -= xSz + HIST_X_BAR_SPACE;
    }
    lora_state = E5_Module_Data.Moudlue_Is_Ok;
  if(E5_Module_Data.Moudlue_Is_Ok == true)
  {
      xOffset = HIST_X_OFFSET + 10;
      int yOffset = HIST_Y_OFFSET + 2;
      lcd.setFont(FM9);    
      lcd.setTextColor(TFT_BLUE);
      lcd.drawString("LoRaWAN",xOffset,yOffset);
      yOffset += 18;    
//      lcd.setFont(FM9);    
      lcd.setTextColor(TFT_WHITE);
      lcd.drawString("Region:",xOffset,yOffset);
//      lcd.setTextColor(TFT_BLACK);
//      lcd.setFont(FS9); 
//      lcd.fillRoundRect(xOffset+80,yOffset-2,60,18,4,TFT_WHITE);//TFT_GRAY // TFT_WHITE
//      lcd.drawString("EU868",xOffset+84,yOffset);
      lcd.setFont(FM9);
      lcd.setTextColor(TFT_WHITE);
      char str[100] = {0};
      char str1[50] = {0};
      strcpy(str,"DevEui:");
      strcat(str,"\"");
      strcat(str,E5_Module_Data.DevEui);
      strcat(str,"\"");
      lcd.drawString(str,xOffset,yOffset+18);  
      memset(str, 0, sizeof(str));
      strcpy(str,"AppEui:");
      strcat(str,"\"");
      strcat(str,E5_Module_Data.AppEui);
      strcat(str,"\"");
      lcd.drawString(str,xOffset,yOffset+36);   
      memset(str, 0, sizeof(str));
//UpdateGpsData(E5_Module_Data.AppKey);
      strcpy(str,"AppKey:");
      strcat(str,"\"");
      memcpy(str1,E5_Module_Data.AppKey,16);
      strcat(str,str1);
      lcd.drawString(str,xOffset,yOffset+54);  
      memset(str, 0, sizeof(str));
      strcpy(str, "        ");
      strcat(str,&E5_Module_Data.AppKey[16]);
      strcat(str,"\""); 
      lcd.drawString(str,xOffset,yOffset+72); 
//      lcd.setFont(FSSO9);    
      lcd.setTextColor(TFT_BLUE); 
      lcd.drawString("Firmware Version",xOffset,yOffset+90);
//      lcd.setFont(FM9);    
      lcd.setTextColor(TFT_WHITE); 
      memset(str, 0, sizeof(str)); 
      strcpy(str,"Lora  :");
      strcat(str,"\""); 
      strcat(str,"V");
      strcat(str,E5_Module_Data.Version);
      strcat(str,"\""); 
      lcd.drawString(str,xOffset,yOffset+108);
  } 
  else
  {
      xOffset = HIST_X_OFFSET + 10;
      lcd.setFont(FSSO9);    
      lcd.setTextColor(TFT_RED);
      lcd.drawString("LoRaWAN No Find",xOffset,HIST_Y_OFFSET+20);          
  }
  } 
  ui.previous_display = ui.selected_display;
  if(E5_Module_Data.Moudlue_Is_Ok == true)
  {
  lcd.setTextColor(TFT_BLACK);
  lcd.setFont(FS9); 
  lcd.fillRoundRect(HIST_X_OFFSET + 10+80,HIST_Y_OFFSET +18 + 2-2,130,18,4,TFT_BLACK);//TFT_GRAY // TFT_WHITE
  if(state.cRegion == US915HYBRID)
  {
      Length = 130;
  }
//if(Status == true){
      lcd.fillRoundRect(HIST_X_OFFSET + 10+80,HIST_Y_OFFSET +18 + 2-2,Length,18,4,TFT_WHITE);//TFT_GRAY // TFT_WHITE
//}
//else{
//      lcd.fillRoundRect(HIST_X_OFFSET + 10+80,HIST_Y_OFFSET +18 + 2-2,Length,18,4,TFT_GRAY);//TFT_GRAY // TFT_WHITE
//       
//}
//Serial.print("state.cRegion = ");
//Serial.println(state.cRegion);
switch(state.cRegion)
{
    case EU868: 
      lcd.drawString("EU868",HIST_X_OFFSET + 10+84,HIST_Y_OFFSET +18 + 2);
    break;
    case US915:
      lcd.drawString("US915",HIST_X_OFFSET + 10+84,HIST_Y_OFFSET +18 + 2);
    break;
    case US915HYBRID:
      lcd.drawString("US915HYBRID",HIST_X_OFFSET + 10+84,HIST_Y_OFFSET +18 + 2);
    break;
    case AU915:
      lcd.drawString("AU915",HIST_X_OFFSET + 10+84,HIST_Y_OFFSET +18 + 2);
    break;
    case AS923:
      lcd.drawString("AS923",HIST_X_OFFSET + 10+84,HIST_Y_OFFSET +18 + 2);
    break;
    case KR920:
      lcd.drawString("KR920",HIST_X_OFFSET + 10+84,HIST_Y_OFFSET +18 + 2);
    break;
    case IN865:
      lcd.drawString("IN865",HIST_X_OFFSET + 10+84,HIST_Y_OFFSET +18 + 2);
    break;
  default:
    break;
} 
  }
  //lcd.pushlcdite(0, 0);  
}
//String N_date, N_time,N_lat,N_lng,N_satellites,N_meters;
//String P_date, P_time,P_lat,P_lng,P_satellites,P_meters;
void refreshGpsInfo(){
//  //lcd.createlcdite(320, 240);
  int xOffset,yOffset;
  if ( ui.previous_display != ui.selected_display )   
  {
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET-18,HIST_X_TXTSIZE,18,TFT_BLACK);
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,TFT_BLACK);
    lcd.setFont(FM9);    
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Gps",HIST_X_OFFSET,HIST_Y_OFFSET-18);

    xOffset = HIST_X_OFFSET+10;
    yOffset = HIST_Y_OFFSET+10;
    lcd.drawString("Date: ",xOffset,yOffset);
    lcd.drawString(N_date,xOffset+65,yOffset);
    yOffset += 18;
    lcd.drawString("Time: ",xOffset,yOffset);
    lcd.drawString(N_time,xOffset+65,yOffset);
    yOffset += 18;
    lcd.drawString("LAT: ",xOffset,yOffset);
    lcd.drawString(N_lat,xOffset+65,yOffset);      
    yOffset += 18;
    lcd.drawString("LONG: ",xOffset,yOffset);
    lcd.drawString(N_lng,xOffset+65,yOffset); 
    yOffset += 18;
    lcd.drawString("ALT: ",xOffset,yOffset);
    lcd.drawString(N_meters,xOffset+65,yOffset); 
    yOffset += 18;
//    spr.drawString("Satellites",xOffset,yOffset,GFXFF);
    lcd.drawString("Satellites: ",xOffset,yOffset);
    lcd.drawString(N_satellites,xOffset+135,yOffset);
                        
    lcd.drawRoundRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,R_SIZE,TFT_WHITE);
    ui.previous_display = ui.selected_display;
  }

/*  // clean the bar
  int xSz = (HIST_X_SIZE - (HIST_X_OFFSET+HIST_X_BAR_OFFSET + MAXBUFFER*HIST_X_BAR_SPACE)) / MAXBUFFER;
  //int 
  xOffset = HIST_X_OFFSET+HIST_X_SIZE-xSz-HIST_X_BAR_SPACE;
  for ( int i = 0 ; i < MAXBUFFER ; i++ ) {
     spr.fillRect(xOffset,HIST_Y_OFFSET+2,xSz,HIST_Y_SIZE-4,TFT_BLACK);
     xOffset -= xSz + HIST_X_BAR_SPACE;
  }*/
  lcd.setFont(FM9);    
  lcd.setTextColor(TFT_WHITE);
  xOffset = HIST_X_OFFSET+10+50;
  yOffset = HIST_Y_OFFSET+10;
  if(N_date != P_date){
      lcd.fillRect(xOffset,yOffset,150,18,TFT_BLACK);
      lcd.drawString(N_date,xOffset+15,yOffset);
      P_date = N_date;  
  }
  yOffset += 18;

  if(N_time != P_time){

      lcd.fillRect(xOffset,yOffset,150,18,TFT_BLACK);
      lcd.drawString(N_time,xOffset+15,yOffset);
      P_time = N_time; 
  }
  yOffset += 18;
  if(N_lat != P_lat){
      lcd.fillRect(xOffset,yOffset,150,18,TFT_BLACK);
      lcd.drawString(N_lat,xOffset+15,yOffset); 
      P_lat = N_lat;
  }
  yOffset += 18;
  if(N_lng != P_lng){
      lcd.fillRect(xOffset,yOffset,150,18,TFT_BLACK);
      lcd.drawString(N_lng,xOffset+15,yOffset); 
      P_lng = N_lng; 
  }
  yOffset += 18;
  if(N_meters != P_meters){
      lcd.fillRect(xOffset,yOffset,150,18,TFT_BLACK);
      lcd.drawString(N_meters,xOffset+15,yOffset); 
      P_meters = N_meters; 
  }
  xOffset += 70;
  yOffset += 18;
  if(N_satellites != P_satellites){
      lcd.fillRect(xOffset,yOffset,100,18,TFT_BLACK);
      lcd.drawString(N_satellites,xOffset+15,yOffset);
      P_satellites = N_satellites; 
  }
  //lcd.pushlcdite(0, 0);  
/*//  if(E5_Module_Data.Moudlue_Is_Ok == true)
  {
      xOffset = HIST_X_OFFSET + 10;
      yOffset = HIST_Y_OFFSET+2; 
//      lcd.setFont(FSSO9);    
//      lcd.setTextColor(TFT_BLUE);
      lcd.drawString("LoRaWAN",xOffset,HIST_Y_OFFSET+2);    
//      lcd.setFont(FM9);    
//      lcd.setTextColor(TFT_WHITE);

  } 
*/
/*  else
  {
      xOffset = HIST_X_OFFSET + 10;
      lcd.setFont(FSSO9);    
      lcd.setTextColor(TFT_RED);
      lcd.drawString("LoRaWAN No Find",xOffset,HIST_Y_OFFSET+20);          
  } 
*/  
}
void refreshRetryHist() {

  // No need to refresh everytime
  if ( ui.previous_display != ui.selected_display ) {
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET-18,HIST_X_TXTSIZE,18,TFT_BLACK);
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,TFT_BLACK);
    lcd.setFont(FM9);    
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Retry",HIST_X_OFFSET,HIST_Y_OFFSET-18);
    lcd.drawRoundRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,R_SIZE,TFT_WHITE);
    ui.previous_display = ui.selected_display;
  }

  // clean the bar
  int xSz = (HIST_X_SIZE - (HIST_X_OFFSET+HIST_X_BAR_OFFSET + MAXBUFFER*HIST_X_BAR_SPACE)) / MAXBUFFER;
  int xOffset = HIST_X_OFFSET+HIST_X_SIZE-xSz-HIST_X_BAR_SPACE;
  for ( int i = 0 ; i < MAXBUFFER ; i++ ) {
     lcd.fillRect(xOffset,HIST_Y_OFFSET+2,xSz,HIST_Y_SIZE-4,TFT_BLACK);
     xOffset -= xSz + HIST_X_BAR_SPACE;
  }
  // Redraw lines
  int yOffset = HIST_Y_OFFSET+HIST_Y_SIZE-10;
  int yStep = ((HIST_Y_OFFSET+HIST_Y_SIZE-10) - ( HIST_Y_OFFSET - 5 )) / (MAX_RETRY);  
  lcd.drawLine(HIST_X_OFFSET+2,yOffset,HIST_X_SIZE-2,yOffset,TFT_GRAY);  
  for ( int i = 1 ; i < MAX_RETRY ; i+= 1 ) {
    int y = yOffset-(yStep*i);
    if ( i % 2 == 0 ) {
      char sTmp[10];
      sprintf(sTmp,"%d",i); 
      lcd.setFont(FF25);    
      lcd.setTextColor(TFT_GRAY);
      lcd.drawString(sTmp,HIST_X_OFFSET+5,y-15);
    }
    lcd.drawLine(HIST_X_OFFSET+2,y,HIST_X_SIZE-2,y,TFT_GRAY20);
  }
  xOffset = HIST_X_OFFSET+HIST_X_SIZE-xSz-HIST_X_BAR_SPACE;
  // for ( int i = 0 ; i < state.elements ; i++ ) {
    //  int idx = getIndexInBuffer(state.elements-(i+1));
    //  if ( idx != MAXBUFFER && state.retry[idx] != LOSTFRAME ) {
        int retry = 3;
        if ( retry == 0 ) {
          lcd.drawLine(xOffset+1,yOffset+3,xOffset+xSz-2,yOffset-3,TFT_GREEN);
          lcd.drawLine(xOffset+1,yOffset-3,xOffset+xSz-2,yOffset+3,TFT_GREEN);        
        } else {
          uint16_t color = TFT_RED;
          if ( retry == 1 ) color = TFT_DARKGREEN;
          else if (retry == 2 ) color = TFT_ORANGE;
          lcd.fillRect(xOffset,yOffset-(retry*yStep),xSz,(retry*yStep),color);
        }
    //  } else {
    //     lcd.drawLine(xOffset+1,yOffset+3,xOffset+xSz-2,yOffset-3,TFT_RED);
    //     lcd.drawLine(xOffset+1,yOffset-3,xOffset+xSz-2,yOffset+3,TFT_RED);        
    //  }
     xOffset -= xSz + HIST_X_BAR_SPACE;
  // }
  
}
