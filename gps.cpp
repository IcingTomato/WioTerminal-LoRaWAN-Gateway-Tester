#include "config.h"
#include "gps.h"
#include "testeur.h"
#include"SoftwareSerial1.h"
#include <TinyGPS++.h>

String N_date, N_time,N_satellites= "0";
String P_date, P_time,P_lat,P_lng,P_satellites,P_meters;
String N_lat = "0:0:0.00";
String N_lng = "0:0:0.00";
String N_meters = "0.00";
static double Lat,Lng,Meters,Satellites;
SoftwareSerial softSerial1(3,2);

TinyGPSPlus gps;
TinyGPSCustom ExtLat(gps, "GPGGA", 3);  //N for Latitude
TinyGPSCustom ExtLng(gps, "GPGGA", 5);  //E for Longitude


void init_softSerial()
{
  //初始化软串口通信；
  softSerial1.begin(9600);
  //监听软串口通信
  softSerial1.listen();
}

void GetGpsInfoPolling(){
  while (softSerial1.available() > 0) {
    char c = softSerial1.read();
//    Serial.print(c);
    gps.encode(c);
  }  
}

void UpdateGpsInfo(){
  if (gps.location.isUpdated() && gps.location.isValid()) 
  {
    double lat0 = gps.location.lat();
    double lat1 = (lat0 -int(lat0))*60;
    double lat2 = (lat1 - int(lat1))*60;
    //String 
    N_lat = String(int(lat0))+':' + String(int(lat1))+':'+String(lat2) + ' ' + String(ExtLat.value());
    
    double lng0 = gps.location.lng();
    double lng1 = (lng0 -int(lng0))*60;
    double lng2 = (lng1 - int(lng1))*60;
    //String 
    N_lng = String(int(lng0))+':' + String(int(lng1))+':'+String(lng2) + ' ' + String(ExtLng.value());
    Lat = lat0*1000000;
    Lng = lng0*1000000;
  }
  if(gps.satellites.isUpdated() && gps.satellites.isValid())
  {
    N_satellites = String(gps.satellites.value());
    Satellites = gps.satellites.value();
  }
  if(gps.altitude.isUpdated() && gps.altitude.isValid())
  {
    N_meters = String(gps.altitude.meters());
    Meters = gps.altitude.meters();
  }
  if (gps.date.isUpdated() && gps.date.isValid()) 
  {
    int y = gps.date.year();
    int m = gps.date.month();
    int d = gps.date.day(); 
    N_date = String(y)+'/'+('0'+String(m)).substring(('0'+String(m)).length()-2)+'/'+('0'+String(d)).substring(('0'+String(d)).length()-2);    
  }
  if (gps.time.isUpdated() && gps.time.isValid()) 
  {
    int h = gps.time.hour();
    int m = gps.time.minute();
    int s = gps.time.second(); 
    N_time = ('0'+String(h)).substring(('0'+String(h)).length()-2)+':'+('0'+String(m)).substring(('0'+String(m)).length()-2)+':'+('0'+String(s)).substring(('0'+String(s)).length()-2);    
  }
}
int UpdateGpsData(char* destination){
  sprintf(destination, "\"%08X%08X%04X%02X\"\r\n", (int)(Lat), (int)(Lng),(int)(Meters),(int)Satellites); 
}
