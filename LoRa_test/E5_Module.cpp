#include "E5_Module.h"
#include "ui.h"
#include "testeur.h"
#include "config.h"
#include "gps.h"

#include<SoftwareSerial.h>

#define LORA_DEFAULT_TIMEOUT (1000)

SqQueue SqQueueRssi;
SqQueue SqQueueSnr;

//新建一个softSerial对象，rx,tx
//SoftwareSerial softSerial1(8,1);
//SoftwareSerial softSerial1(1,8);
//SoftwareSerial softSerial1(1,0); // D0脚有问题
//SoftwareSerial softSerial1(3,2);

//SoftwareSerial softSerial1(2,3);

//SoftwareSerial softSerial1(4,5);
//SoftwareSerial softSerial1(5,4);
//SoftwareSerial softSerial1(7,6);
//SoftwareSerial softSerial1(6,7);

/*void init_softSerial(void)
{
  //初始化软串口通信；
  softSerial1.begin(9600);
  //监听软串口通信
  softSerial1.listen();
}*/

int Check_E5_Module(void)
{
    int i=0;
    do
    {
        if(E5_Module_AT_Cmd("AT"))
        {
            return 1;  
        }
        delay(100);
        i++;
    }while(i<3);
    return 0;  
}

int Init_E5_Mode(void)  // 使用默认参数初始化E5模块
{
    if(Check_E5_Module())
    {
        E5_Module_Data.Moudlue_Is_Ok = true;
        E5_Module_AT_Cmd("ID");
        E5_Module_AT_Cmd("MODE");
        E5_Module_AT_Cmd("POWER");
        E5_Module_Data.Adr = false;
        E5_Module_AT_Cmd("ADR");
        E5_Module_AT_Cmd("DR");
        E5_Module_Data.Sf = EU868;
        E5_Module_AT_Cmd("DR");
        E5_Module_AT_Cmd("CH");
        E5_Module_AT_Cmd("KEY");
        E5_Module_AT_Cmd("CLASS");
        E5_Module_AT_Cmd("PORT");
        E5_Module_AT_Cmd("VER");
        ui.selected_display = DISPLAY_DEVICE_INFO;
    }
    else
    {
        E5_Module_Data.Moudlue_Is_Ok = false;
        ui.selected_display = DISPLAY_DEVICE_INFO;        
    }
    return E5_Module_Data.Moudlue_Is_Ok;   
}

int E5_Moudle_Jion(void)
{
    E5_Module_AT_Cmd("JOIN");
}

int E5_Module_SendCmsgHexData(void)
{
    Serial.println("Send Data....");
    if(Module_Is_Busy() == true)
    {
        Serial.println("Module_Is_Busy");
        return 0;
    }
    if((E5_Module_Data.State == NOT_JOINED) || (E5_Module_Data.State == JOIN_FAILED))
    {
        if(E5_Module_Data.Moudlue_Is_Ok == false)
        {
            Init_E5_Mode(); 
            if(E5_Module_Data.Moudlue_Is_Ok == false)
            {
                ui.selected_display = DISPLAY_DEVICE_INFO;
                return 0;  
            }
        }
        E5_Module_Data.State = JOINING;
        E5_Moudle_Jion();
    } 
    else
    {
        if(E5_Module_Data.SendNumber < state.cTotal)
        {
            E5_Module_Data.State = IN_TX;
            E5_Module_Data.SendNumber++;
            //strcpy(E5_Module_Data.SendData,"112233");
            memset(E5_Module_Data.SendData, 0, sizeof(E5_Module_Data.SendData));
            UpdateGpsData(E5_Module_Data.SendData);
            E5_Module_AT_Cmd("CMSGHEX");               
        }
    }
}

char recv_buf[512];
char g_str[100];

char* SetPower(void);
char* SetSf(void);
char* SetAdr(void);
char* SetKey(void);
char* SetPort(void);
char* SetMode(void);
char* SetCh(void);
char* SetRetry(void);
char* SetCmsgHexData(void);
char* SetClass(void);

void GetPower(void);
void GetVersion(void);
void GetId(void);
void GetCmsgHexData(void);
void GetNetState(void);
E5_Module_Cmd_t  E5_Module_Cmd[] =
{
  // 命令        应答             超时时间 设置参数      获取参数
    {"AT",      "+AT: OK",       LORA_DEFAULT_TIMEOUT,NULL,          NULL}, 
    {"POWER",   "+POWER",        LORA_DEFAULT_TIMEOUT,SetPower,      GetPower},
    {"DR",      "+DR",           LORA_DEFAULT_TIMEOUT,SetSf,         NULL},     // 还有地区协议
    {"ADR",     "+ADR",          LORA_DEFAULT_TIMEOUT,SetAdr,        NULL}, 
    {"VER",     "+VER",          LORA_DEFAULT_TIMEOUT,NULL,          GetVersion},
    {"ID",      "+ID: AppEui",   LORA_DEFAULT_TIMEOUT,NULL,          GetId},
    {"RESET",   "+RESET",        LORA_DEFAULT_TIMEOUT,NULL,          NULL},
    {"PORT",    "+PORT",         LORA_DEFAULT_TIMEOUT,SetPort,       NULL},
    {"MODE",    "+MODE",         LORA_DEFAULT_TIMEOUT,SetMode,       NULL},  
    {"JOIN",    "Done"/*"Network joined"*/,12000,NULL,         GetNetState},        
    {"KEY",     "+KEY: APPKEY",  LORA_DEFAULT_TIMEOUT,SetKey,        NULL},    
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
    8,
    2,
    3,
    true,
    NOT_JOINED,
    LWOTAA,
    CLASSA,
    true,
    0,
    0, 
    0,
    0,  
    {0},
    {0},
    {0},
    //{"2B:7E:15:16:28:AE:D2:A6:AB:F7:15:88:09:CF:4F:3C"},
    {"2B7E151628AED2A6ABF7158809CF4F3C"}, 
};
int E5_Comm_Flag = false;
int cmd_seq = 0;
int E5_Module_AT_Cmd(char *p_cmd)
{
    if(E5_Comm_Flag == true)
    {
        return 0;  
    }
    E5_Comm_Flag = true;
    char cmd[128];   
    int Response_Flag = 0;
    memset(cmd, 0, sizeof(cmd));
    strcpy(cmd,"AT");
    for(cmd_seq=0;cmd_seq<E5_MODULE_CMD_LENGTH;cmd_seq++)
    {
        if(strcmp(p_cmd,E5_Module_Cmd[cmd_seq].p_cmd) == 0)
        {
            Serial.println("p_cmd is equal to E5_Module_Cmd");
            break;  
        }  
    }
    if(cmd_seq == E5_MODULE_CMD_LENGTH)
    {
        Serial.println("END");
        return 0;
    }
    if(strcmp(p_cmd,cmd) == 0)
    {
        strcat(cmd,"\r\n");         
    }
    else
    {   
        strcat(cmd,"+");   
        strcat(cmd,p_cmd);
    }
    Serial.print("cmd_seq="); 
    Serial.println(cmd_seq); 
    if(E5_Module_Cmd[cmd_seq].Set_E5_Module_Para != NULL)
    {
        //Serial.print("Ser Para");
        char *p;//= getstring();//E5_Module_Cmd[i].Set_E5_Module_Para();
        p = E5_Module_Cmd[cmd_seq].Set_E5_Module_Para();
        //Serial.println(p);
        strcat(cmd,"=");
        strcat(cmd,p);    
    }
    strcat(cmd,"\r\n"); 
//    Serial.print(cmd); 
    Response_Flag = at_send_check_response(E5_Module_Cmd[cmd_seq].p_ack,E5_Module_Cmd[cmd_seq].timeout_ms,cmd);  
    if((Response_Flag == 1) && (E5_Module_Cmd[cmd_seq].Get_E5_Module_Para != NULL))
    {
        E5_Module_Cmd[cmd_seq].Get_E5_Module_Para();  
    }
    Serial.print("Response_Flag=");
    Serial.println(Response_Flag);
    if(Response_Flag==0)
    {
//E5_Comm_Flag = false;
if(E5_Module_Cmd[cmd_seq].timeout_ms<=1000)
{
    E5_Comm_Flag = false;  
}
        return 0;  
    }
    else
    {
        E5_Comm_Flag = false;
        return 1;  
    }      
}

char* SetCmsgHexData(void)
{
    memset(g_str, 0, sizeof(g_str));
    strcpy(g_str,E5_Module_Data.SendData);
    return  g_str;        
}

void recv_prase(char *p_msg) 
{
  if (p_msg == NULL) {
    return;
  }
  char *p_start = NULL;
  int data = 0;
  int rssi = 0;
  int snr = 0;

  p_start = strstr(p_msg, "RX");
  if (p_start && (1 == sscanf(p_start, "RX: \"%d\"\r\n", &data))) {
    Serial.println(data);
  }

  p_start = strstr(p_msg, "RSSI");

  if (p_start && (1 == sscanf(p_start, "RSSI %d,", &rssi))) {
    char strr[25];
    itoa(rssi, strr, 10);
  }

  p_start = strstr(p_msg, "SNR");
  if (p_start && (1 == sscanf(p_start, "SNR %d", &snr))) {
    char str[25];
    itoa(snr, str, 10);
  }
}

void GetNetState(void)
{
    if (strstr(recv_buf,"Network joined") != NULL)
    { 
        E5_Module_Data.State = JOINED;
        E5_Module_SendCmsgHexData();
    }
    else
    {
        E5_Module_Data.State = JOIN_FAILED;            
    }   
}

void GetCmsgHexData(void)
{
    char *p_start = NULL;
    int data = 0;
    if (strstr(recv_buf,"RSSI") != NULL)
    {
        E5_Module_Data.State = DWNLINK;
        E5_Module_Data.RecvNumber++; 
         
        p_start = strstr(recv_buf, "RX");
        if (p_start && (1 == sscanf(p_start, "RX: \"%d\"\r\n", &data))) {
            Serial.print("RX:");
            Serial.println(data);
        }
      
        p_start = strstr(recv_buf, "RSSI");
      
        if (p_start && (1 == sscanf(p_start, "RSSI %d,", &E5_Module_Data.rssi))) {
            Serial.print("RSSI:");
            Serial.println(E5_Module_Data.rssi);
        }
      
        p_start = strstr(recv_buf, "SNR");
        if (p_start && (1 == sscanf(p_start, "SNR %d", &E5_Module_Data.snr))) {
            Serial.print("SNR:");
            Serial.println(E5_Module_Data.snr);
        } 
        SqQueueFillData(&SqQueueRssi,E5_Module_Data.rssi);
        SqQueueFillData(&SqQueueSnr,E5_Module_Data.snr);
 //       static int snr_tmp = -20;
 //       E5_Module_Data.snr = snr_tmp; 
 //       SqQueueFillData(&SqQueueSnr,snr_tmp);
 //       snr_tmp += 2;  
    }
    else
    {
        SqQueueFillData(&SqQueueRssi,0);
        SqQueueFillData(&SqQueueSnr,0);       
        E5_Module_Data.State = DWNLINK_FAILED;  
    }
    state.hasRefreshed = true;         
}
char* SetKey(void)
{
    char cmd[100];
    memset(g_str, 0, sizeof(g_str));
    sprintf(cmd, "APPKEY,\"%23s\"\r\n",E5_Module_Data.AppKey); 
    strcpy(g_str,cmd);
    return  g_str;
}
char* SetPower(void)
{
    memset(g_str, 0, sizeof(g_str));
    if(E5_Module_Data.Pwr>16)
    {
        E5_Module_Data.Pwr = 16;  
    }
    else if(E5_Module_Data.Pwr<2)
    {
         E5_Module_Data.Pwr = 2;  
    }
    itoa(E5_Module_Data.Pwr, g_str, 10);
    return g_str;
}

void GetPower(void)
{
    char str_tmp[10];
    char *p_start = NULL;
    p_start = strstr(recv_buf, "+POWER");
    sscanf(p_start, "+POWER: %2s,", str_tmp);
    E5_Module_Data.Pwr = atoi(str_tmp);
    Serial.println(E5_Module_Data.Pwr);        
}
void GetVersion(void)
{
    char *p_start = NULL;
    p_start = strstr(recv_buf, "+VER");
    sscanf(p_start, "+VER: %10s,", E5_Module_Data.Version);
    Serial.println(E5_Module_Data.Version);      
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
#ifdef DEBUG      
    Serial.println(E5_Module_Data.DevAddr);
    Serial.println(E5_Module_Data.DevEui);
    Serial.println(E5_Module_Data.AppEui);
    Serial.println(E5_Module_Data.AppKey); 
#endif 
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
        case EU868:
            strcpy(g_str,"EU868");
            break;
        case US915:
            strcpy(g_str,"US915");
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

int at_send_check_response(char *p_ack, int timeout_ms, char *p_cmd, ...) 
{
  int ch;
  int num = 0;
  int index = 0;
  int startMillis = 0;
  va_list args;
// 清空模块应答超时导致的后续数据缓存在硬件buff中
while (Serial1.available() > 0) {
ch = Serial1.read();
Serial.print((char)ch);
}
  memset(recv_buf, 0, sizeof(recv_buf));
  va_start(args, p_cmd);
  Serial1.printf(p_cmd, args);
//  softSerial1.printf(p_cmd, args);
  Serial.printf(p_cmd, args);
//Serial.println(p_cmd);
  va_end(args);

  startMillis = millis();

  if ((p_ack == NULL) || (timeout_ms > 1000)) {
//  if(p_ack == NULL){
    return 0;
  }
  do {
  while (Serial1.available() > 0) {
      ch = Serial1.read();
      recv_buf[index++] = ch;
      Serial.print((char)ch);
      delay(2);
    }

/*     while (softSerial1.available() > 0) {
      ch = softSerial1.read();
      recv_buf[index++] = ch;
      Serial.print((char)ch);
      delay(2);
    }*/
    if (strstr(recv_buf, p_ack) != NULL) {
      return 1;
    }

  } while (millis() - startMillis < timeout_ms);
Serial.println("AT CMD Timeout....");
  return 0;
}

void GetSerialDataPolling(void)
{
   static bool init_flag = false;
   static int startMillis = 0;
   static int index = 0;
   int ch;
   if(E5_Comm_Flag == true)
   {    
       if(init_flag == false)
       {
           startMillis = millis();
           init_flag = true; 
           index = 0; 
           Serial.print("startMillis = ");
           Serial.println(startMillis);      
       }
     while (Serial1.available() > 0) {
        ch = Serial1.read();
        recv_buf[index++] = ch;
        Serial.print((char)ch);
        delay(2);
      }
  
      /* while (softSerial1.available() > 0) {
        ch = softSerial1.read();
        recv_buf[index++] = ch;
        Serial.print((char)ch);
        delay(2);
      }*/
      if (strstr(recv_buf, E5_Module_Cmd[cmd_seq].p_ack) != NULL) 
      {
          E5_Comm_Flag = false;
          init_flag = false;
          Serial.println("AT CMD Polling Success....");   
          //return 1;
          if(E5_Module_Cmd[cmd_seq].Get_E5_Module_Para != NULL)
          {
              E5_Module_Cmd[cmd_seq].Get_E5_Module_Para();  
          }
      }
      if(millis() - startMillis >= E5_Module_Cmd[cmd_seq].timeout_ms)
      {
          E5_Module_Data.Moudlue_Is_Ok = false;
          ui.selected_display = DISPLAY_DEVICE_INFO;
          //state.hasRefreshed = true;
          if(E5_Module_Cmd[cmd_seq].Get_E5_Module_Para != NULL)
          {
              E5_Module_Cmd[cmd_seq].Get_E5_Module_Para();  
          }
          Serial.print("millis = ");
          Serial.println(millis()); 
          Serial.print("timeout_ms = ");
          Serial.println(E5_Module_Cmd[cmd_seq].timeout_ms);     
          E5_Comm_Flag = false;
          init_flag = false;
          Serial.println("AT CMD Polling Timeout....");          
      }
   }      
}

bool Module_Is_Busy(void)
{
   return E5_Comm_Flag;  
}
