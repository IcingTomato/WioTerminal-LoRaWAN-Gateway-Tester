#include "testeur.h"
#include "E5_Module.h"

#define LGFX_USE_V1
#define LGFX_AUTODETECT // LGFX_WIO_TERMINAL // LGFX_AUTODETECT

#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

#ifndef __UI_H
#define __UI_H

enum e_mode_state {
  MODE_MANUAL,
  MODE_CLEAR_DATA,
  MODE_AUTO_1MIN,
  MODE_MAX_RATE,
  MODE_MAX,
};

enum e_display_state {
  DISPLAY_RSSI_HIST,
  DISPLAY_SNR_HIST,
  DISPLAY_DEVICE_INFO,
  DISPLAY_GPS_INFO,
  DISPLAY_MAX,
};

#ifdef WITH_SPLASH_HELIUM
  #ifdef WITH_SPLASH_TTN
    #define HELIUM_XCENTER (160-100)/2
    #define TTN_XCENTER 110
  #else
    #define HELIUM_XCENTER (320-100)/2
  #endif
#else
  #define TTN_XCENTER (320-100)/2
#endif

typedef struct s_ui {
  uint8_t selected_menu;    // What menu zone is selected for settings
  uint8_t selected_mode;    // What mode do we want ? manual / auto
  e_module_state displayed_state;  // showning LoRaWan join state

  uint8_t selected_display; // What graph we want to display
  uint8_t previous_display;
  bool    hasClick;         // Hit on button
  bool    hasRefreshed;
  bool    refreshPower;
  bool    refreshSf;
  bool    refreshTotal;
  bool    refreshMode;
  bool    refreshLastFrame;
  bool    refreshDeviceInfo;
} ui_t;

extern ui_t ui;
extern LGFX lcd;
extern bool Clear_Data_Flag; 
extern bool hasAction;

void initScreen();
void refresUI();
void refreshPower(); 
void refreshSf();
void refreshRetry();
void refreshTotal();
void refreshState();
void refreshRssiHist();
void refreshSnrHist();
void refreshRetryHist();
void refreshMode();
void refreshLastFrame();
void refreshDeviceInfo();
void refreshGpsInfo();
void UpdateGpsInfo();
void Clear_Data();
void refreshGps();
void Button_Detection(void);
void draw_splash_helium(int xOffset, int yOffset, int density);
void draw_splash_ttn(int xOffset, int yOffset, int density);

#endif
