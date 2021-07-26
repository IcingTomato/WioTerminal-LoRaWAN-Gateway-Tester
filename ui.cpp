#include <Arduino.h>
#include "config.h"
#include "fonts.h"
#include "testeur.h"
#include "ui.h"
#include "gps.h"
#include "E5_Module.h"
#include "SqQueue.h"

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
#define SELECTED_TOTAL  3

ui_t ui;
int index_Rssi = 0;
bool hasAction = false;

void initScreen() {
  lcd.init();
  lcd.setRotation(1);
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
#endif
  lcd.fillScreen(TFT_BLACK);
  ui.selected_mode = MODE_MANUAL;
  ui.selected_menu = SELECTED_NONE;
  ui.displayed_state = UKN_STAT;
  ui.previous_display = DISPLAY_MAX;
  ui.hasClick = false;
  ui.refreshPower = true;
  ui.refreshSf = true;
  ui.refreshTotal = true;
  ui.refreshMode = true;
  ui.refreshLastFrame = true;
  ui.hasRefreshed = false;

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
        E5_Module_Data.SendNumber = 0;
        E5_Module_Data.RecvNumber = 0;
        E5_Module_Data.rssi = 0;
        E5_Module_Data.snr  = 0;
        ClearQueue(&SqQueueRssi);
        ClearQueue(&SqQueueSnr);
        ui.refreshLastFrame = true;
        ui.hasRefreshed = true;     
    }
}
void Button_Detection(void){
  hasAction = true;
  bool configHasChanged = true;
  uint8_t prev_select = ui.selected_menu;
  
  if (digitalRead(WIO_KEY_C) == LOW) {
    configHasChanged = false;
    ui.selected_menu = ( prev_select == SELECTED_POWER )?SELECTED_NONE:SELECTED_POWER;
  } else if (digitalRead(WIO_KEY_B) == LOW) {
    configHasChanged = false;
    ui.selected_menu = ( prev_select == SELECTED_SF )?SELECTED_NONE:SELECTED_SF;
  } else if (digitalRead(WIO_KEY_A) == LOW) {
    configHasChanged = false;
    ui.selected_menu = ( prev_select == SELECTED_TOTAL )?SELECTED_NONE:SELECTED_TOTAL;
  } else if ((digitalRead(WIO_5S_UP) == LOW) || (digitalRead(WIO_5S_DOWN) == LOW)) {
    int wio_5s_Up = digitalRead(WIO_5S_UP);
    switch ( ui.selected_menu ) {
      case SELECTED_POWER:
          ui.refreshPower = true;
          if(wio_5s_Up == LOW){
              tst_setPower(state.cPwr+2);
          }
          else{
              tst_setPower(state.cPwr-2);   
          }
          E5_Module_Data.Pwr = state.cPwr;
          SqQueueFillData(&SqQueueAtCmd,AT_POWER);
          break;
      case SELECTED_SF:
          ui.refreshSf = true;
          if(wio_5s_Up == LOW){
            tst_setSf(state.cSf+1);
          }
          else{
            tst_setSf(state.cSf-1);
          }
          E5_Module_Data.Sf = (e_Lora_Sf)(state.cSf);
          SqQueueFillData(&SqQueueAtCmd,AT_SF);   
         break;
      case SELECTED_TOTAL:
          ui.refreshTotal = true;
         if(wio_5s_Up == LOW){
            if(state.cTotal>=990)
            {
                state.cTotal = 990;
            }
            else
            {
                state.cTotal += 10;
            }  
         }
         else{
            if(state.cTotal<=10)
            {
                state.cTotal = 10;
            }
            else
            {
                state.cTotal -= 10;
            }           
         }       
         break;
       case SELECTED_NONE:
          if(ui.selected_display == DISPLAY_DEVICE_INFO)
          {
              if(E5_Module_Data.Moudlue_is_exist == true)
              {
                if(wio_5s_Up == LOW){
                    tst_setRegion(state.cRegion+1);
                }
                else{
                    tst_setRegion(state.cRegion-1);
                }
                ClearQueue(&SqQueueAtCmd);
                E5_Module_Data.Region = (e_Lora_Regional)state.cRegion;
                SqQueueFillData(&SqQueueAtCmd,AT_REGION);
                if(E5_Module_Data.Pwr != state.cPwr)
                {
                    ui.refreshPower = true;                
                    E5_Module_Data.Pwr = state.cPwr;
                    SqQueueFillData(&SqQueueAtCmd,AT_POWER);
                }
                if((E5_Module_Data.Sf != state.cSf) || (state.cRegion == US915) || (state.cRegion == US915HYBRID))
                {
                    ui.refreshSf = true;
                    E5_Module_Data.Sf  = (e_Lora_Sf)state.cSf;
                    SqQueueFillData(&SqQueueAtCmd,AT_SF);
                }            
                ui.hasRefreshed = true;
                if(ui.selected_mode != MODE_MANUAL) // Forced in MODE_MANUAL 
                {
                    ui.selected_mode = MODE_MANUAL;
                    ui.refreshMode = true;
                }
              }
          }
          else
          {  
              if(wio_5s_Up == LOW){
                  ui.selected_mode = (ui.selected_mode+MODE_MAX+1)%MODE_MAX; 
              }
              else{
                  ui.selected_mode = (ui.selected_mode+MODE_MAX-1)%MODE_MAX; 
              }
              ui.refreshMode = true;
          }             
      default:
         break;   
    }  
  } else if (digitalRead(WIO_5S_RIGHT) == LOW) {
    ui.selected_display = (ui.selected_display+1)%DISPLAY_MAX;
  } else if (digitalRead(WIO_5S_LEFT) == LOW) {
    ui.selected_display = (ui.selected_display+DISPLAY_MAX-1)%DISPLAY_MAX;
  } else if (digitalRead(WIO_5S_PRESS) == LOW) {
    if (( ui.selected_mode == MODE_MANUAL) && (ui.selected_display != DISPLAY_DEVICE_INFO)) {
      ui.hasClick = true;
    }
    else if(ui.selected_mode == MODE_CLEAR_DATA)
    {      
        Clear_Data();
        Clear_Data_Flag = true;        
    }
  }
  else{
    hasAction = false;
    configHasChanged = false;
  }
  
  if ( prev_select != ui.selected_menu) {
    if ( prev_select == SELECTED_POWER || ui.selected_menu == SELECTED_POWER ) {
      refreshPower();
    }
    if ( prev_select == SELECTED_SF || ui.selected_menu == SELECTED_SF ) {
      refreshSf();
    }
    if ( prev_select == SELECTED_TOTAL || ui.selected_menu == SELECTED_TOTAL ) {
      refreshTotal();
    }
  }
    
  if ( configHasChanged ) {
    storeConfig();
  } 
  // avoid re-entreing
  //if ( hasAction ) delay(10);//(300); 
}
void refresUI() {
  if(ui.refreshPower){
    ui.refreshPower = false;
    refreshPower();
  }
  if(ui.refreshSf){
    ui.refreshSf = false;
    refreshSf();
  }
  if(ui.refreshTotal){
    ui.refreshTotal = false;
    refreshTotal();
  }
  if(ui.refreshMode)
  {
    ui.refreshMode = false;
    refreshMode();
  }
  
  // refresh the Join state part
  refreshState();

  // refresh the graph history part
  if ((ui.hasRefreshed) || (ui.previous_display != ui.selected_display)){ 
    ui.hasRefreshed = false;
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

  } 
  if ( ui.refreshLastFrame == true ) 
  {
    ui.refreshLastFrame = false;
    refreshLastFrame();
  } 
  if(ui.selected_display == DISPLAY_GPS_INFO)
  {
      refreshGpsInfo();  
  } 
}

/**
 * Select the way the messages are sent
 * On user action
 * Automatically
 */
void refreshMode() { 
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
}

/**
 * Update the last frame information on top of the screen
 */
void refreshLastFrame() 
{
  int xOffset = X_OFFSET;
  int yOffset = Y_OFFSET+Y_SIZE+2;
  lcd.fillRect(xOffset,yOffset,3*X_SIZE,Y_SIZE,TFT_BLACK);
  lcd.drawRoundRect(xOffset,yOffset,3*X_SIZE,Y_SIZE,R_SIZE,TFT_WHITE);
  lcd.fillRect(xOffset+X_SIZE-3,yOffset+Y_SIZE,3*X_SIZE,Y_SIZE,TFT_BLACK);
  lcd.drawRoundRect(xOffset+X_SIZE-3,yOffset+Y_SIZE,3*X_SIZE,Y_SIZE,R_SIZE,TFT_WHITE);
//  int idx = getLastIndexWritten();
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
}

/**
 * Update the current state display to see on the corner right / top 
 * the current tester action
 */
void refreshState() {
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
}

void refreshPower() {
  uint16_t color = (ui.selected_menu == SELECTED_POWER)?TFT_WHITE:TFT_GRAY;
  lcd.fillRoundRect(X_OFFSET,Y_OFFSET,X_SIZE-BOX_SPACING,Y_SIZE,R_SIZE,color);
  lcd.setTextColor(TFT_BLACK);
  lcd.setFont(FS9);     // Select the orginal small TomThumb font
  char sPower[10];
  sprintf(sPower,"+%02d dBm",state.cPwr); 
  lcd.drawString(sPower,X_OFFSET+5,Y_OFFSET+3);
}

void refreshSf() {
  int xOffset = X_OFFSET+1*X_SIZE;
  uint16_t color = (ui.selected_menu == SELECTED_SF)?TFT_WHITE:TFT_GRAY;
  lcd.fillRoundRect(xOffset,Y_OFFSET,X_SIZE-BOX_SPACING,Y_SIZE,R_SIZE,color);
  lcd.setTextColor(TFT_BLACK);
  lcd.setFont(FS9);//);     // Select the orginal small TomThumb font
  char sSf[10];
  if((state.cRegion == US915) || (state.cRegion == US915HYBRID)){
     sprintf(sSf,"SF   %02d",state.cSf-2); 
  }
  else{
     sprintf(sSf,"SF   %02d",state.cSf);     
  }
  lcd.drawString(sSf,xOffset+5,Y_OFFSET+3);
}

void refreshTotal(){
  int xOffset = X_OFFSET+2*X_SIZE;
  uint16_t color = (ui.selected_menu == SELECTED_TOTAL)?TFT_WHITE:TFT_GRAY;
  lcd.fillRoundRect(xOffset,Y_OFFSET,X_SIZE-BOX_SPACING,Y_SIZE,R_SIZE,color);
  lcd.setTextColor(TFT_BLACK);
  lcd.setFont(FS9);     // Select the orginal small TomThumb font
  char sRetry[10];
  sprintf(sRetry,"Total %3d",state.cTotal); 
  lcd.drawString(sRetry,xOffset+5,Y_OFFSET+3);
}




void refreshRssiHist() {
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
}
void refreshSnrHist() {

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
}  

void refreshDeviceInfo()
{
  int Length = 60;
  int xOffset;
  static bool lora_state = false; 
  if (ui.previous_display != ui.selected_display ){
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET-18,HIST_X_TXTSIZE,18,TFT_BLACK);
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,TFT_BLACK);
    lcd.setFont(FM9);    
    lcd.setTextColor(TFT_WHITE);
    lcd.drawString("Device",HIST_X_OFFSET,HIST_Y_OFFSET-18);
    lcd.drawRoundRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,R_SIZE,TFT_WHITE);
  }
  if((lora_state != E5_Module_Data.Moudlue_is_exist) || (ui.refreshDeviceInfo) || (ui.previous_display != ui.selected_display))
  {
    ui.refreshDeviceInfo = false;
    lcd.fillRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,TFT_BLACK);
    lcd.drawRoundRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,R_SIZE,TFT_WHITE);
    lora_state = E5_Module_Data.Moudlue_is_exist;
    if(E5_Module_Data.Moudlue_is_exist == true)
    {
        xOffset = HIST_X_OFFSET + 10;
        int yOffset = HIST_Y_OFFSET + 2;
        lcd.setFont(FM9);    
        lcd.setTextColor(TFT_BLUE);
        lcd.drawString("LoRaWAN",xOffset,yOffset);
        yOffset += 18;       
        lcd.setTextColor(TFT_WHITE);
        lcd.drawString("Region:",xOffset,yOffset);
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
        lcd.setTextColor(TFT_BLUE); 
        lcd.drawString("Firmware Version",xOffset,yOffset+90);    
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
  if(E5_Module_Data.Moudlue_is_exist == true)
  {
    lcd.setTextColor(TFT_BLACK);
    lcd.setFont(FS9); 
    lcd.fillRoundRect(HIST_X_OFFSET + 10+80,HIST_Y_OFFSET +18 + 2-2,130,18,4,TFT_BLACK);//TFT_GRAY // TFT_WHITE
    if(state.cRegion == US915HYBRID)
    {
        Length = 130;
    }
    lcd.fillRoundRect(HIST_X_OFFSET + 10+80,HIST_Y_OFFSET +18 + 2-2,Length,18,4,TFT_WHITE);//TFT_GRAY // TFT_WHITE
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
}

void refreshGpsInfo(){
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
    lcd.drawString("Satellites: ",xOffset,yOffset);
    lcd.drawString(N_satellites,xOffset+135,yOffset);
                        
    lcd.drawRoundRect(HIST_X_OFFSET,HIST_Y_OFFSET,HIST_X_SIZE,HIST_Y_SIZE,R_SIZE,TFT_WHITE);
    ui.previous_display = ui.selected_display;
  }

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
}
