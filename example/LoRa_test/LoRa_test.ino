#include <Arduino.h>
#include <SPI.h>
#include "E5_Module.h"
#include "gps.h"
#include "ui.h"
#include "testeur.h"
#include "config.h"
#include "SqQueue.h"

void setup(void) {
  Serial.begin(115200);

  init_softSerial();
    
  Serial1.begin(9600);
  while (!Serial1);
  InitQueue(&SqQueueRssi);
  InitQueue(&SqQueueSnr);
  InitQueue(&SqQueueAtCmd);
  initState();
  initScreen();
  Init_E5_Mode();
}
void loop(void)
{
  static long cTime = 5 * 60 * 1000;
  static long hasActionTime = 0;
  static long batUpdateTime = 0;

  long sTime = millis();
  bool fireMessage = false;
  GetGpsInfoPolling();
  UpdateGpsInfo();

  if((hasAction==false) || (hasActionTime >= 300)){
     Button_Detection();
     hasActionTime = 0;
  }
  refresUI();
  CheckSerialData();
  E5_Module_Polling();
  switch ( ui.selected_mode ) {
    case MODE_MANUAL:
      if ( ui.hasClick && (Module_Is_Busy() == false))
      { 
        fireMessage = true;
        ui.hasClick = false;
      }
      break;
    case MODE_AUTO_1MIN:
      if ( (cTime >= ( 1 * 60 * 1000 )) && (Module_Is_Busy() == false)) 
      {
          cTime = 0;
          fireMessage = true;
      }
      break;
    case MODE_MAX_RATE:
      if(Module_Is_Busy() == false)
      {
          fireMessage = true;
      }
      break;
  }  
  if(fireMessage == true)
  {
      E5_Module_SendCmsgHexData();
  }
  long duration = millis() - sTime;
  cTime += duration;
  if(hasAction)
  {
     hasActionTime += duration;
  }
}

char Serial_recv_buf[100];
void CheckSerialData(){
   int recv_index = 0;
   while (Serial.available() > 0) {
    char ch = Serial.read();
    Serial_recv_buf[recv_index++] = ch;
  }
  if(recv_index > 0)
  {
    char str[40] = {0};
    bool Set_Error_Flag = true;
    bool Set_DevEui_Flag = false;
    bool Set_AppEui_Flag = false;
    bool Set_AppKey_Flag = false;
    char *p_start = NULL;
    p_start = strstr(Serial_recv_buf, "DevEui");
    if(p_start != NULL)
    {
        Set_Error_Flag = false;
        sscanf(p_start, "DevEui,\"%16s,", &str);
        if(Check_Parameter(str,16)){
             Serial.println("DevEui parameter error");
        }
        else{
            strcpy(E5_Module_Data.DevEui,str);
            Set_DevEui_Flag = true;
            Serial.println("DevEui is set Success");          
        }
    }
    memset(str, 0, sizeof(str));
    p_start = strstr(Serial_recv_buf, "AppEui");
    if(p_start != NULL)
    {
        Set_Error_Flag = false;
        sscanf(p_start, "AppEui,\"%16s,", &str);
        if(Check_Parameter(str,16)){
           Serial.println("AppEui parameter error");
        }
        else{
           strcpy(E5_Module_Data.AppEui,str);
           Set_AppEui_Flag = true;
           Serial.println("AppEui is set Success");          
        }
    }
    memset(str, 0, sizeof(str));
    p_start = strstr(Serial_recv_buf, "AppKey");
    if(p_start != NULL)
    {
        Set_Error_Flag = false;
        sscanf(p_start, "AppKey,\"%32s,", &str);
        if(Check_Parameter(str,32)){
           Serial.println("AppKey parameter error");
        }
        else{
           strcpy(E5_Module_Data.AppKey,str);
           storeConfig();
           Set_AppKey_Flag = true;
           Serial.println("AppKey is set Success");         
        }
    }
    if(Set_Error_Flag == true)
    {
       Serial.println("The correct format is as follows");
       Serial.println("DevEui,\"0123456789ABCDEF\"");
       Serial.println("AppEui,\"0123456789ABCDEF\"");
       Serial.println("AppKey,\"2B7E151628AED2A6ABF7158809CF4F3C\"");            
    }
    else{
        if(ui.selected_mode != MODE_MANUAL) // Forced in MODE_MANUAL 
        {
            ui.selected_mode = MODE_MANUAL;
            ui.refreshMode = true;
        }
        E5_Module_Data.State = NOT_JOINED;
        E5_Module_Data.Moudlue_is_join = false;
        Clear_Data();
        Clear_Data_Flag = true;
        ClearQueue(&SqQueueAtCmd);
        if(Set_DevEui_Flag){
           SqQueueFillData(&SqQueueAtCmd,AT_DEVEUI);          
        }
        if(Set_AppEui_Flag){
           SqQueueFillData(&SqQueueAtCmd,AT_APPEUI);          
        }
        if(Set_AppKey_Flag){
           SqQueueFillData(&SqQueueAtCmd,AT_KEY);          
        }
        SqQueueFillData(&SqQueueAtCmd,AT_ID);
    }
  }
  recv_index = 0;
  memset(Serial_recv_buf, 0, sizeof(Serial_recv_buf));
}

int Check_Parameter(char* data,int Length){
  for(int i= 0;i<Length;i++)
  {
    if((data[i]>='0' && data[i]<='9') || (data[i]>='a' && data[i]<='f') || (data[i]>='A' && data[i]<='F'))
    {
             
    } 
    else{
       return 1;          
    }
  }
  return 0;
}
