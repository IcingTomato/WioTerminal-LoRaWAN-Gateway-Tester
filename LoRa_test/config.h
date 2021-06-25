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
#ifndef __CONFIG_H
#define __CONFIG_H


//#define DEBUG

#define WITH_SPLASH         1
#define WITH_SPLASH_HELIUM  1   
#define WITH_SPLASH_TTN     1

//#define WITH_GPS
//#define WITH_LIPO

//#ifdef DEBUG
//  #define LOGLN(x)  Serial.println x
//  #define LOG(x) Serial.print x
//#else
//  #define LOGLN(x) 
//  #define LOG(x)
//#endif

bool readConfig();
void storeConfig();

#endif // __CONFIG_H
