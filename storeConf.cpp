#include <FlashAsEEPROM_SAMD.h>
#include "testeur.h"
#include "ui.h"

#define MAGIC 0x1D00
#define VERSION 0x01

const int START_ADDRESS     = 0;

Config state;

bool readConfig() {
  Config c ;
  EEPROM.get(START_ADDRESS,c);
  if ( c.magic == MAGIC && c.version == VERSION ) {
    state.cPwr = c.cPwr;
    state.cSf = c.cSf;
    state.cRegion = c.cRegion;
    state.cRetry = c.cRetry;
    state.cTotal = c.cTotal;
    ui.selected_display = c.selected_display;
    strcpy(E5_Module_Data.AppKey,c.cAppkey);
  } else {
    return false;
  }
  return true; 
}

void storeConfig() {
  Config c;
  c.magic = MAGIC;
  c.version = VERSION;
  c.cPwr = state.cPwr;
  c.cSf = state.cSf;
  c.cRegion = state.cRegion;
  c.cRetry = state.cRetry;
  c.cTotal = state.cTotal;
  c.selected_display = ui.selected_display;
  strcpy(c.cAppkey,E5_Module_Data.AppKey);
  EEPROM.put(START_ADDRESS, c);
  if (!EEPROM.getCommitASAP())
  {
    EEPROM.commit();
  } 
}
