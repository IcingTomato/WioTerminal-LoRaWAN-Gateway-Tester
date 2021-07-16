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
//#include <lmic.h>
#include "config.h"
#include "testeur.h"
#include "E5_Module.h"
#include "ui.h"

state_t state;
void initState() {
  if ( ! readConfig() ) 
  {
    //Serial.println("Defaulr para");
    tst_setPower(MAXPOWER);  
    tst_setSf(SLOWERSF);      
    tst_setRetry(3);
    tst_setRegion(EU868);
    state.cTotal  = 100;    
  }
//  state.cState = NOT_JOINED;
  state.readPtr = 0;
  state.writePtr = 0;
  state.elements = 0;
  state.hasRefreshed = false;
  state.batVoltage = 0;
}

void addInBuffer(int16_t rssi, int16_t snr, uint8_t retry, uint16_t seq, bool lost) {
  state.seq[state.writePtr] = seq;
  if ( ! lost ) { 
      state.rssi[state.writePtr] = rssi;
      state.snr[state.writePtr] = snr;
      state.retry[state.writePtr] = retry;
      state.hs[state.writePtr] = NODATA;
  } else {
      state.rssi[state.writePtr] = 0;
      state.snr[state.writePtr] = 0;
      state.retry[state.writePtr] = LOSTFRAME;
      state.hs[state.writePtr] = NODATA;
  }
  state.writePtr = (state.writePtr+1) & (MAXBUFFER-1);
  if ( state.writePtr == state.readPtr ) {
    state.readPtr = (state.readPtr+1) & (MAXBUFFER-1);  
  }
  if (state.elements < MAXBUFFER) state.elements++;
}

uint8_t getIndexInBuffer(int i) {
  int t = state.readPtr;
  for ( int k = 0 ; k < i ; k++ ) {
    if ( t == state.writePtr ) return MAXBUFFER;
    t = (t+1) & (MAXBUFFER-1);
  }
  return 100;
}

uint8_t getIndexBySeq(int seq) {
  int idx = state.readPtr;
  while ( idx != state.writePtr && state.seq[idx] != seq ) {
    idx = ( idx + 1 ) & (MAXBUFFER-1);
  }
  if ( state.seq[idx] == seq ) return idx;
  else return MAXBUFFER;
}

uint8_t getLastIndexWritten() {
  if ( state.writePtr == 0 ) {
    if ( state.readPtr == 0 ) {
      return MAXBUFFER;
    }
    return MAXBUFFER-1;
  }
  return state.writePtr-1;
}


void tst_setPower(int8_t pwr) {
//  if ( pwr < 2 ) pwr = 2;
/*  #if defined CFG_eu868
   if ( pwr > 16 ) pwr = 16;
   pwr &= 0xFE;
  #elif defined CFG_us915
    if ( pwr > 20 ) pwr = 20;
  #else
    #error "Not Yet implemented"
  #endif*/
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
//  E5_Module_Data.Pwr = pwr;
//  E5_Module_AT_Cmd("POWER");
}

void tst_setSf(uint8_t sf) {

/*  if ( sf < 7 ) sf = 7;
  #if defined CFG_eu868
    if ( sf > 12 ) sf = 12;
  #elif defined CFG_us915
    if ( sf > 10 ) sf = 10;
  #else
    #error "Not Yet implemented"
  #endif*/
  if ( sf > 12 ) sf = 12;
  if((state.cRegion == US915) || (state.cRegion == US915HYBRID)){ 
    if ( sf < 9 ) sf = 9;
    if ( sf > 11 ) sf = 11;  // Region US915 and US915HYBRID only send 11 bytes in the case of DR0,but we need send 13 bytes,  
  }
  else{
    if ( sf < 7 ) sf = 7;
  }
  state.cSf = sf;
//  E5_Module_Data.Sf = (e_Lora_Regional)sf;
//  E5_Module_AT_Cmd("DR");  
}

void tst_setRetry(uint8_t retry) {
  if ( retry > TXCONF_ATTEMPTS ) retry = TXCONF_ATTEMPTS;
  if ( retry < 0 ) retry = 0;
  state.cRetry = retry;
//  E5_Module_Data.Retry = retry;
//  E5_Module_AT_Cmd("RETRY"); 
}

void tst_setRegion(uint8_t Region) {
  if ( Region >= IN865 ) Region = IN865;
  if ( Region <= EU868 ) Region = EU868;
  state.cRegion= Region;
  tst_setPower(state.cPwr);
  refreshPower();
  tst_setSf(state.cSf);
  refreshSf();
  E5_Module_Data.State = NOT_JOINED;
  is_join = false;
  refreshState();
  Clear_Data();
//  E5_Module_Data.Retry = retry;
//  E5_Module_AT_Cmd("RETRY"); 
}
