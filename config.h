#ifndef __CONFIG_H
#define __CONFIG_H


//#define DEBUG

#define WITH_SPLASH         1
#define WITH_SPLASH_HELIUM  1   
#define WITH_SPLASH_TTN     1


#ifdef DEBUG
  #define LOGLN(x)  Serial.println x
  #define LOG(x) Serial.print x
#else
  #define LOGLN(x) 
  #define LOG(x)
#endif

bool readConfig();
void storeConfig();

#endif // __CONFIG_H
