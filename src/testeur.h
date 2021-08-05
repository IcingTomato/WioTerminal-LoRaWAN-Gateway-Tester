#ifndef __TESTER_H
#define __TESTER_H

#define MAXBUFFER    32

typedef struct {
  uint16_t  magic;
  uint8_t   version;
  int8_t    cPwr;     // Current Power
  uint8_t   cSf;      // Current SF
  uint8_t   cRegion;  // Current Region
  uint8_t   cRetry;   // Current Number of retry
  uint16_t  cTotal;
  uint8_t   selected_display;
  uint8_t   selected_mode;
  char      cAppkey[48];
} Config;

extern Config state;

void initState();
void tst_setPower(int8_t pwr);
void tst_setSf(uint8_t sf);
void tst_setRegion(uint8_t Region);

#endif
