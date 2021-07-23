#ifndef __E5_Module_H__
#define __E5_Module_H__

#include <Arduino.h>
#include"SoftwareSerial1.h"
#include "SqQueue.h"

extern SqQueue SqQueueRssi;
extern SqQueue SqQueueSnr;
extern bool Atcmd_Timeout;
extern bool is_join;
#define DEBUG
#define E5_MODULE_CMD_LENGTH (sizeof(E5_Module_Cmd)/sizeof(E5_Module_Cmd[0]))//(10)

enum e_module_state {
  NOT_JOINED    = 1,    // Not connected yet
  JOIN_FAILED   = 2,    // Connection failed
  JOINING       = 3,    // Currenlty running the joining procedure
  JOINED        = 4,    // Joined, waiting for action
  IN_TX         = 5,    // Transmitting 
  IN_RPT        = 6,    // 1st transmission failed, running a repeat
  DWNLINK       = 7,    // downlink
  DWNLINK_FAILED= 8,    // downlink failed

  UKN_STAT = 255
};

enum e_Lora_Mode {
  TEST  ,    // Test Mode
  LWOTAA,    // OTAA Mode
  LWABP ,    // ABP  Mode
  UKN_MODE = 255
};

enum e_Lora_Regional {
  SF7    = 7,
  SF8,
  SF9,
  SF10,
  SF11,
  SF12,
  EU868  = 13, 
  US915,
  US915HYBRID,
  AU915,
  AS923,
  KR920,
  IN865,        
  UKN_REGIONAL = 255        
};

enum e_Lora_Class {
  CLASSA,
  CLASSB,
  CLASSC,      
  UKN_CLASS = 255
};

typedef struct s_E5_Module_Data {
  uint8_t           Pwr;     // Current Power
  e_Lora_Regional   Sf;      // Current SF
  uint8_t           Port;    // Current PORT
  uint8_t           Ch;      // CH Number
  uint8_t           Retry;   // Current Number of retry
  bool              Adr;
  e_module_state    State;   // Current State (Joined / NotJoined)
  e_Lora_Mode       Mode;
  e_Lora_Class      Class;
  bool              Moudlue_Is_Ok;
  int               SendNumber;
  int               RecvNumber;
  
  int               rssi;     // Rssi history
  int               snr;      // Snr history

  char DevAddr[12];
  char DevEui[24];
  char AppEui[24];
  char AppKey[48];
  char Version[10];
  char SendData[100];
  char RecvData[100];  
} E5_Module_Data_t;

typedef struct s_E5_Module_Cmd {
    char p_cmd[10];
    char p_ack[15];
    int  timeout_ms;
    char* (*Set_E5_Module_Para)(void);
    void (*Get_E5_Module_Para)(void);
} E5_Module_Cmd_t;


extern E5_Module_Data_t E5_Module_Data;
extern E5_Module_Cmd_t  E5_Module_Cmd[];
extern char recv_buf[512];

int at_send_check_response(char *p_ack, int timeout_ms, char *p_cmd, ...);
void recv_prase(char *p_msg);
int E5_Module_AT_Cmd(char *p_cmd);
//void init_softSerial(void);

int Init_E5_Mode(void);
int E5_Module_SendCmsgHexData(void);
void GetSerialDataPolling(void);
bool Module_Is_Busy(void);
#endif
