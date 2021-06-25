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
#include <FlashAsEEPROM_SAMD.h>
#include "testeur.h"
#include "ui.h"

#define MAGIC 0x1D00
#define VERSION 0x01

const int START_ADDRESS     = 0;

typedef struct {
  uint16_t  magic;
  uint8_t   version;
  int8_t    cPwr;     // Current Power
  uint8_t   cSf;      // Current SF
  uint8_t   cRetry;   // Current Number of retry
  uint16_t  cTotal;
  uint8_t   selected_display;
  uint8_t   selected_mode;
} Config;

// Load configuration from flash
// return tru if loaded false if default must be set
bool readConfig() {
  Serial.println("Read Data.....");
  Config c ;
  EEPROM.get(START_ADDRESS,c);
  if ( c.magic == MAGIC && c.version == VERSION ) {
    state.cPwr = c.cPwr;
    state.cSf = c.cSf;
    state.cRetry = c.cRetry;
    state.cTotal = c.cTotal;
    ui.selected_display = c.selected_display;
    ui.selected_mode = c.selected_mode;
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
  c.cRetry = state.cRetry;
  c.cTotal = state.cTotal;
  c.selected_display = ui.selected_display;
  c.selected_mode = ui.selected_mode;
  EEPROM.put(START_ADDRESS, c);
  if (!EEPROM.getCommitASAP())
  {
    Serial.println("CommitASAP not set. Need commit()");
    EEPROM.commit();
  } 
  Serial.println("Save Data.....");
}
