#include <Arduino.h>
#include "config.h"
#include "testeur.h"
#include "E5_Module.h"
#include "ui.h"

void initState() {
  if ( ! readConfig() ) 
  {
    tst_setRegion(EU868);
    tst_setPower(16);  
    tst_setSf(12);      
    state.cTotal  = 100;  
    ui.selected_display = DISPLAY_DEVICE_INFO;//DISPLAY_RSSI_HIST;//DISPLAY_DEVICE_INFO;//DISPLAY_RSSI_HIST;    
    storeConfig();  
  }
}

void tst_setPower(int8_t pwr) {
  if((state.cRegion == EU868) || (state.cRegion == AS923)){
     if(pwr>16)pwr = 16;
     if(pwr<2)pwr  = 2;
  }
  else if(state.cRegion == KR920){
     if(pwr>14)pwr = 14;    
  }
  else{
     if(pwr>22)pwr = 22;     
  }
  if(state.cRegion == IN865){
     if(pwr<10)pwr  = 10;    
  }
  else{
     if(pwr<0)pwr  = 0;      
  }
  state.cPwr = pwr;
}

void tst_setSf(uint8_t sf) {
  if ( sf > 12 ) sf = 12;
  if((state.cRegion == US915) || (state.cRegion == US915HYBRID)){ 
    if ( sf < 9 ) sf = 9;
//    if ( sf > 11 ) sf = 11;  // Region US915 and US915HYBRID only send 11 bytes in the case of DR0,but we need send 13 bytes,  
  }
  else{
    if ( sf < 7 ) sf = 7;
  }
  state.cSf = sf;
}

void tst_setRegion(uint8_t Region) {
  if ( Region >= MAX_REGIONAL ) Region = IN865;
  if ( Region <= MIN_REGIONAL ) Region = EU868;
  state.cRegion= Region;
  E5_Module_Data.State = NOT_JOINED;
  E5_Module_Data.Moudlue_is_join = false;
  Clear_Data();
  Clear_Data_Flag = true;
  tst_setPower(state.cPwr);
  tst_setSf(state.cSf);
}
