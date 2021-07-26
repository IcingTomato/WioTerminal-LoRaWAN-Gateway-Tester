#ifndef __E5_Module_H__
#define __E5_Module_H__

#include <Arduino.h>
#include "SqQueue.h"

extern SqQueue SqQueueRssi;
extern SqQueue SqQueueSnr;
extern SqQueue SqQueueAtCmd;

extern bool Atcmd_Timeout;

#define E5_MODULE_CMD_LENGTH (sizeof(E5_Module_Cmd)/sizeof(E5_Module_Cmd[0]))//(10)

enum e_module_AT_Cmd{ 
  AT_OK,
  AT_POWER,
  AT_REGION,
  AT_SF,
  AT_ADR,
  AT_VER,
  AT_ID,
  AT_DEVEUI,
  AT_APPEUI,
  AT_RESET,
  AT_PORT,
  AT_MODE,
  AT_JOIN,
  AT_KEY,
  AT_CH,
  AT_RETRY,
  AT_LOWPOWER,
  AT_TEST,
  AT_CMSGHEX,
  AT_CLASS,
  AT_MAX,
};

enum e_module_Response_Result{
  MODULE_IDLE, 
  MODULE_RECEIVING,    
  MODULE_TIMEOUT,
  MODULE_ACK_SUCCESS,   
};

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

enum e_Lora_Sf {
  SF7    = 7,
  SF8,
  SF9,
  SF10,
  SF11,
  SF12,        
  UKN_SF = 255        
};

enum e_Lora_Regional {
  MIN_REGIONAL,
  EU868, 
  US915,
  US915HYBRID,
  AU915,
  AS923,
  KR920,
  IN865,        
  MAX_REGIONAL        
};

enum e_Lora_Class {
  CLASSA,
  CLASSB,
  CLASSC,      
  UKN_CLASS = 255
};

typedef struct s_E5_Module_Data {
  uint8_t           Pwr;     // Current Power
  e_Lora_Sf         Sf;      // Current SF
  e_Lora_Regional   Region;
  uint8_t           Port;    // Current PORT
  uint8_t           Ch;      // CH Number
  uint8_t           Retry;   // Current Number of retry
  bool              Adr;
  e_module_state    State;   // Current State (Joined / NotJoined)
  e_Lora_Mode       Mode;
  e_Lora_Class      Class;
  bool              Moudlue_is_exist;
  bool              Moudlue_is_join;
  int               SendNumber;
  int               RecvNumber;
  
  int               rssi;     // Rssi history
  int               snr;      // Snr history

  char DevAddr[12];
  char DevEui[24];
  char AppEui[24];
  char AppKey[48];
  char Version[20];
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
int E5_Module_AT_Cmd(char *p_cmd);
void Init_E5_Mode(void);
int E5_Module_SendCmsgHexData(void);
bool Module_Is_Busy(void);
void E5_Module_Polling();
#endif
