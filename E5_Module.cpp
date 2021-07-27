#include "E5_Module.h"
#include "ui.h"
#include "testeur.h"
#include "config.h"
#include "gps.h"

#define LORA_DEFAULT_TIMEOUT (1000)

SqQueue SqQueueRssi;
SqQueue SqQueueSnr;
SqQueue SqQueueAtCmd;

void Init_E5_Mode(void)  // 使用默认参数初始化E5模块
{
    readConfig();
    SqQueueFillData(&SqQueueAtCmd,AT_OK);
    SqQueueFillData(&SqQueueAtCmd,AT_VER);
    SqQueueFillData(&SqQueueAtCmd,AT_KEY);
    SqQueueFillData(&SqQueueAtCmd,AT_ID);
    SqQueueFillData(&SqQueueAtCmd,AT_MODE);
    SqQueueFillData(&SqQueueAtCmd,AT_RETRY);
    E5_Module_Data.Region = (e_Lora_Regional)state.cRegion;
    SqQueueFillData(&SqQueueAtCmd,AT_REGION);
    E5_Module_Data.Pwr = state.cPwr;
    SqQueueFillData(&SqQueueAtCmd,AT_POWER);
    SqQueueFillData(&SqQueueAtCmd,AT_ADR);
    E5_Module_Data.Sf  = (e_Lora_Sf)state.cSf;
    SqQueueFillData(&SqQueueAtCmd,AT_SF);
    SqQueueFillData(&SqQueueAtCmd,AT_CLASS);
    SqQueueFillData(&SqQueueAtCmd,AT_PORT);  
}

int E5_Module_SendCmsgHexData(void)
{
    if(E5_Module_Data.Moudlue_is_exist == false)
    {
        Init_E5_Mode();
    }
    else if(E5_Module_Data.Moudlue_is_join == false)
    {
        SqQueueFillData(&SqQueueAtCmd,AT_JOIN);
    } 
    else
    {
        if(E5_Module_Data.SendNumber < state.cTotal)
        {
            memset(E5_Module_Data.SendData, 0, sizeof(E5_Module_Data.SendData));
            UpdateGpsData(E5_Module_Data.SendData);
            SqQueueFillData(&SqQueueAtCmd,AT_CMSGHEX);               
        }
    }
}

char recv_buf[512];
char g_str[100];

char* SetPower(void);
char* SetRegion(void);
char* SetSf(void);
char* SetAdr(void);
char* SetDevEui(void);
char* SetAppEui(void);
char* SetKey(void);
char* SetPort(void);
char* SetMode(void);
char* SetNetState(void);
char* SetCh(void);
char* SetRetry(void);
char* SetCmsgHexData(void);
char* SetClass(void);

void GetDevice(void);
void GetPower(void);
void GetVersion(void);
void GetId(void);
void GetCmsgHexData(void);
void GetNetState(void);
void GetKey(void);

E5_Module_Cmd_t  E5_Module_Cmd[] =
{
  // 命令        应答             超时时间 设置参数      获取参数
    {"AT",      "+AT: OK",       LORA_DEFAULT_TIMEOUT,NULL,          GetDevice}, 
    {"POWER",   "+POWER",        LORA_DEFAULT_TIMEOUT,SetPower,      GetPower},
    {"DR",      "+DR",           LORA_DEFAULT_TIMEOUT,SetRegion,     NULL},     // 还有地区协议
    {"DR",      "+DR",           LORA_DEFAULT_TIMEOUT,SetSf,         NULL},     // 还有地区协议
    {"ADR",     "+ADR",          LORA_DEFAULT_TIMEOUT,SetAdr,        NULL}, 
    {"VER",     "+VER",          LORA_DEFAULT_TIMEOUT,NULL,          GetVersion},
    {"ID",      "+ID: AppEui",   LORA_DEFAULT_TIMEOUT,NULL,          GetId},
    {"ID",      "+ID: DevEui",   LORA_DEFAULT_TIMEOUT,SetDevEui,     NULL},
    {"ID",      "+ID: AppEui",   LORA_DEFAULT_TIMEOUT,SetAppEui,     NULL},
    {"RESET",   "+RESET",        LORA_DEFAULT_TIMEOUT,NULL,          NULL},
    {"PORT",    "+PORT",         LORA_DEFAULT_TIMEOUT,SetPort,       NULL},
    {"MODE",    "+MODE",         LORA_DEFAULT_TIMEOUT,SetMode,       NULL},  
    {"JOIN",    "Done"/*"Network joined"*/,20000,SetNetState,        GetNetState},        
    {"KEY",     "+KEY: APPKEY",  LORA_DEFAULT_TIMEOUT,SetKey,        GetKey},    
    {"CH",      "+CH",           LORA_DEFAULT_TIMEOUT,SetCh,         NULL},        //      
    {"RETRY",   "+RETRY",        LORA_DEFAULT_TIMEOUT,SetRetry,      NULL},
    {"LOWPOWER","+LOWPOWER",     LORA_DEFAULT_TIMEOUT,NULL,          NULL}, 
    {"TEST",    "+TEST",         LORA_DEFAULT_TIMEOUT,NULL,          NULL},
    {"CMSGHEX", /*"RSSI",*/"Done",          30000,SetCmsgHexData,GetCmsgHexData},
    {"CLASS",   "+CLASS",        LORA_DEFAULT_TIMEOUT,SetClass,      NULL},         
//    {"REPT",    "+REPT",         1000,NULL,          NULL},         
//    {"RXWIN2",  "+RXWIN2",       1000,NULL,          NULL},  
//    {"RXWIN1",  "+RXWIN1",       1000,NULL,          NULL}, 
//    {"DELAY",   "+DELAY",        1000,NULL,          NULL},                    
//    {"MSG",     "Done",          5000,SetMsgData,    GetMsgData},
//    {"CMSG",    "Done",          5000,SetCmsgData,   GetCmsgData},
//    {"MSGHEX",  "Done",          5000,SetMsgHexData, GetMsgHexData},
//    {"PMSG",    "Done",          5000,SetPmsgData,   GetPmsgData},
//    {"PMSGHEX", "Done",          5000,SetPmsgHexData,GetPmsgHexData}, 
//    {"VDD",     "+VDD",          1000,NULL,          NULL},  
//    {"TEMP",    "+TEMP",         1000,NULL,          NULL}, 
//    {"RTC",     "+RTC",          1000,NULL,          NULL},                    
};
E5_Module_Data_t E5_Module_Data = 
{
    16,
    SF12,
    EU868,
    8,
    2,
    3,
    false,
    NOT_JOINED,
    LWOTAA,
    CLASSA,
    false,
    false,
    0,
    0, 
    0,
    0,  
    {0},
    {0},
    {0},
    {"2B7E151628AED2A6ABF7158809CF4F3C"}, 
};

char* SetCmsgHexData(void)
{
    E5_Module_Data.SendNumber++;
    E5_Module_Data.State = IN_TX;
    memset(g_str, 0, sizeof(g_str));
    strcpy(g_str,E5_Module_Data.SendData);
    return  g_str;        
}

void GetNetState(void)
{
    if (strstr(recv_buf,"Network joined") != NULL)
    { 
        E5_Module_Data.Moudlue_is_join = true;
        E5_Module_Data.State = JOINED;
        E5_Module_SendCmsgHexData();
    }
    else
    {
        E5_Module_Data.Moudlue_is_join = false;
        E5_Module_Data.State = JOIN_FAILED;            
    }   
}

void GetKey(void){
    char *p_start = NULL;
    p_start = strstr(recv_buf, "+KEY: APPKEY ");
    sscanf(p_start, "+KEY: APPKEY , %32s,", &E5_Module_Data.AppKey);     
}
void GetCmsgHexData(void)
{
    char *p_start = NULL;
    int data = 0;
    if (strstr(recv_buf,"+CMSGHEX: Please join network first") != NULL)
    {
        E5_Module_Data.Moudlue_is_join = false;
        E5_Module_Data.State = NOT_JOINED;          
    }
    if (strstr(recv_buf,"RSSI") != NULL)
    {
        E5_Module_Data.State = DWNLINK;
        E5_Module_Data.RecvNumber++; 
         
        p_start = strstr(recv_buf, "RX");
        if (p_start && (1 == sscanf(p_start, "RX: \"%d\"\r\n", &data))) {
        }
      
        p_start = strstr(recv_buf, "RSSI");
      
        if (p_start && (1 == sscanf(p_start, "RSSI %d,", &E5_Module_Data.rssi))) {
        }
      
        p_start = strstr(recv_buf, "SNR");
        if (p_start && (1 == sscanf(p_start, "SNR %d", &E5_Module_Data.snr))) {
        }
        if(E5_Module_Data.rssi >= 0)
        {
          E5_Module_Data.rssi = -1;   
        }
        SqQueueFillData(&SqQueueRssi,E5_Module_Data.rssi);
        SqQueueFillData(&SqQueueSnr,E5_Module_Data.snr); 
    }
    else
    {
        SqQueueFillData(&SqQueueRssi,0);
        SqQueueFillData(&SqQueueSnr,0);       
        E5_Module_Data.State = DWNLINK_FAILED;  
    }
    ui.refreshLastFrame = true;
    ui.hasRefreshed = true;         
}
char* SetKey(void)
{
    char cmd[100];
    memset(g_str, 0, sizeof(g_str));
    sprintf(cmd, "APPKEY,\"%32s\"\r\n",E5_Module_Data.AppKey); // 23
    strcpy(g_str,cmd);
    return  g_str;
}
char* SetPower(void)
{
    memset(g_str, 0, sizeof(g_str));
    itoa(E5_Module_Data.Pwr, g_str, 10);
    return g_str;
}

void GetDevice(void){
    if (strstr(recv_buf,"+AT: OK") != NULL){
        E5_Module_Data.Moudlue_is_exist = true;
    }
    else{
        E5_Module_Data.Moudlue_is_exist = false;
    }
    ui.hasRefreshed = true;
}
void GetPower(void)
{
    char str_tmp[10];
    char *p_start = NULL;
    p_start = strstr(recv_buf, "+POWER");
    sscanf(p_start, "+POWER: %2s,", str_tmp);
    E5_Module_Data.Pwr = atoi(str_tmp);        
}
void GetVersion(void)
{
    char *p_start = NULL;
    p_start = strstr(recv_buf, "+VER");
    sscanf(p_start, "+VER: %10s,", E5_Module_Data.Version);    
}
void GetId(void) 
{
    char str[30];
    int j = 0;
    char *p_start = NULL;
    p_start = strstr(recv_buf, "DevAddr");
    sscanf(p_start, "DevAddr, %11s,", &str);//&E5_Module_Data.DevAddr);
    for(int i = 0;i<8;i++,j++)
    {
      if((i != 0) && (i%2 == 0))
      {         
          j += 1;  
      }      
      E5_Module_Data.DevAddr[i]= str[j];     
    }

    p_start = strstr(recv_buf, "DevEui");
    sscanf(p_start, "DevEui, %23s,", &str);//&E5_Module_Data.DevEui);
    j = 0;
    for(int i = 0;i<16;i++,j++)
    {
      if((i != 0) && (i%2 == 0))
      {         
          j += 1;  
      }      
      E5_Module_Data.DevEui[i]= str[j];     
    }    
    p_start = strstr(recv_buf, "AppEui");
    sscanf(p_start, "AppEui, %23s,", &str);//&E5_Module_Data.AppEui);
    j = 0;
    for(int i = 0;i<16;i++,j++)
    {
      if((i != 0) && (i%2 == 0))
      {         
          j += 1;  
      }      
      E5_Module_Data.AppEui[i]= str[j];     
    }
    ui.hasRefreshed = true;
    ui.refreshDeviceInfo = true;       
}
char* SetSf(void)
{
    memset(g_str, 0, sizeof(g_str));
    switch(E5_Module_Data.Sf)
    {
        case SF7:
        case SF8:
        case SF9:
        case SF10:
        case SF11:
        case SF12:
            itoa(12-E5_Module_Data.Sf, g_str, 10);
            break;
        default:
            return NULL;  
    }
    return g_str;
}
char* SetRegion(void)
{
    memset(g_str, 0, sizeof(g_str));
    switch(E5_Module_Data.Region)
    {
        case EU868:
            strcpy(g_str,"EU868");
            break;
        case US915:
            strcpy(g_str,"US915");
            break;
        case US915HYBRID:
            strcpy(g_str,"US915HYBRID");
            break;
        case AU915:
            strcpy(g_str,"AU915");
            break;
        case AS923:
            strcpy(g_str,"AS923");
            break;
        case KR920:
            strcpy(g_str,"KR920");
            break;
        case IN865:
            strcpy(g_str,"IN865");
            break;
        default:
            return NULL;  
    }
    return g_str;
}

char* SetPort(void)
{
    memset(g_str, 0, sizeof(g_str));
    itoa(E5_Module_Data.Port, g_str, 10);
    return g_str;  
}

char* SetNetState(void){
    E5_Module_Data.State = JOINING;
    memset(g_str, 0, sizeof(g_str));
    strcpy(g_str,"FORCE");
    return g_str;
}
char* SetMode(void)
{
    memset(g_str, 0, sizeof(g_str));
    switch(E5_Module_Data.Mode)
    {
        case TEST:
            strcpy(g_str,"TEST");
            break;
        case LWOTAA:
            strcpy(g_str,"LWOTAA");
            break;
        case LWABP:
            strcpy(g_str,"LWABP");
            break; 
        default:
            return NULL;
            break;      
    }
    return g_str;   
}

char* SetClass(void)
{
    memset(g_str, 0, sizeof(g_str));
    switch(E5_Module_Data.Class)
    {
        case CLASSA:
            strcpy(g_str,"A");
            break;
        case CLASSB:
            strcpy(g_str,"B");
            break;
        case CLASSC:
            strcpy(g_str,"C");
            break; 
        default:
            return NULL;
            break;      
    }
    return g_str;    
}
char* SetCh(void)
{
    memset(g_str, 0, sizeof(g_str));
    itoa(E5_Module_Data.Ch, g_str, 10);
    return g_str;     
}

char* SetRetry(void)
{
    memset(g_str, 0, sizeof(g_str));
    itoa(E5_Module_Data.Retry, g_str, 10);
    return g_str;    
}
char* SetAdr(void)
{
    memset(g_str, 0, sizeof(g_str));
    if(E5_Module_Data.Adr == true)
    {
       strcpy(g_str,"ON");
    }
    else
    {
       strcpy(g_str,"OFF"); 
    }  
    return g_str; 
}

char* SetDevEui(void){
    char cmd[100];
    memset(g_str, 0, sizeof(g_str));
    sprintf(cmd, "DevEui,\"%16s\"\r\n",E5_Module_Data.DevEui); 
    strcpy(g_str,cmd);
    return  g_str;    
}
char* SetAppEui(void){
    char cmd[100];
    memset(g_str, 0, sizeof(g_str));
    sprintf(cmd, "AppEui,\"%16s\"\r\n",E5_Module_Data.AppEui); 
    strcpy(g_str,cmd);
    return  g_str;       
}
static bool Lora_is_busy = false;
int module_AT_Cmd = 0; 

bool Module_Is_Busy(void)
{
    if((Lora_is_busy == false) && (QueueEmpty(SqQueueAtCmd))){
        return false;
    }
    return true;
}

int E5_Module_AT_Cmd(e_module_AT_Cmd AT_Cmd)
{
  if(Lora_is_busy == true)
  {
      return 0;  
  }
  Lora_is_busy = true;
  module_AT_Cmd = AT_Cmd;
  char cmd[128];   
  int Response_Flag = 0;
  memset(cmd, 0, sizeof(cmd));
  strcpy(cmd,"AT");
  if(strcmp(E5_Module_Cmd[module_AT_Cmd].p_cmd,cmd) != 0) // not AT
  {
      strcat(cmd,"+");   
      strcat(cmd,E5_Module_Cmd[module_AT_Cmd].p_cmd);      
  }

  if(E5_Module_Cmd[module_AT_Cmd].Set_E5_Module_Para != NULL)
  {
      char *p;//= getstring();//E5_Module_Cmd[i].Set_E5_Module_Para();
      p = E5_Module_Cmd[module_AT_Cmd].Set_E5_Module_Para();
      if(p!=NULL)
      {
          strcat(cmd,"=");
          strcat(cmd,p);   
      }
  }
  strcat(cmd,"\r\n");
  Serial1.printf(cmd);
	return 1;
}

static int check_message_response()
{
  static bool init_flag = false;
  static int  startMillis = 0;
  static int  index = 0;
  int ch;
  if(Lora_is_busy == true)
  {
      if(init_flag == false)
      {
         startMillis = millis();
         init_flag = true; 
         index = 0; 
         memset(recv_buf, 0, sizeof(recv_buf));  
      }
      Lora_is_busy = false;
      init_flag = false; 
      while (Serial1.available() > 0)
      {
        ch = Serial1.read();
        recv_buf[index++] = ch;
        delay(2);
      }
    
      if (strstr(recv_buf, E5_Module_Cmd[module_AT_Cmd].p_ack) != NULL)
      {
		  if((E5_Module_Cmd[module_AT_Cmd].Get_E5_Module_Para != NULL))
		  {
			  E5_Module_Cmd[module_AT_Cmd].Get_E5_Module_Para();	
		  }
          return MODULE_ACK_SUCCESS;
      }
      
      if(millis() - startMillis >= E5_Module_Cmd[module_AT_Cmd].timeout_ms)
      {
		   if((E5_Module_Cmd[module_AT_Cmd].Get_E5_Module_Para != NULL))
		   {
		 	  E5_Module_Cmd[module_AT_Cmd].Get_E5_Module_Para();	
		   }      
          return MODULE_TIMEOUT;
      }  
      Lora_is_busy = true;
      init_flag = true;  
      return MODULE_RECEIVING;
  }
  return MODULE_IDLE;
}

void E5_Module_Polling(){
	switch(check_message_response())
	{
    case MODULE_IDLE:
        QElemType d; 
        if(DeQueue(&SqQueueAtCmd,&d))
        {
            E5_Module_AT_Cmd((e_module_AT_Cmd)d);
        }            
        break; 
    case MODULE_RECEIVING:
        break;
    case MODULE_TIMEOUT:
        ClearQueue(&SqQueueAtCmd);
        Init_E5_Mode(); 
        break;
    case MODULE_ACK_SUCCESS:
        if(Clear_Data_Flag == true){
            if(E5_Module_Data.Moudlue_is_join == false)
            {
               E5_Module_Data.State = NOT_JOINED;
            }
            Clear_Data_Flag = false; 
            Clear_Data();  
        } 
        break;
    default:
        ClearQueue(&SqQueueAtCmd);
        Init_E5_Mode();
        break;  
	}
}
