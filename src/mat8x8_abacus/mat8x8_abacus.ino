//cf https://lang-ship.com/reference/unofficial/M5StickC/UseCase/RTCSetNTP/
#include <M5StickCPlus.h>
#include <WiFi.h>
#include "time.h"

char wifi_ssid[33]    = "";
char wifi_passwd[65]  = ""; 
const char* ntpServer = "ntp.nict.jp";

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

int matrix_address = 0x09;

int matrix_bright(int bright){ //bright=1(max)..62(off) \-
  int ret;
  int val;
    
  Wire.beginTransmission(matrix_address);
  Wire.write(0xAF);
  Wire.write(bright);
  ret = Wire.endTransmission();

  return ret;
}
int matrix_putimg(char *p){
  int ret;
  
  Wire.beginTransmission(matrix_address);
  Wire.write(0x90);
  Wire.write(p[0]);
  Wire.write(p[1]);
  Wire.write(0);
  Wire.write(0);
  Wire.write(p[2]);
  Wire.write(p[3]);
  Wire.write(p[4]);
  Wire.write(p[5]);
  Wire.write(p[6]);
  Wire.write(p[7]);
  ret = Wire.endTransmission();
  return ret;
}

void setup() {
  static char img[] = {0xF5, 0xF5, 0x00, 0xF5, 0xF5, 0x00, 0xF5, 0xF5}; 

// initialize M5StickCPlus
  M5.begin();
  Wire.begin(32, 33);

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextSize(2);

  matrix_bright(1);
  matrix_putimg(img);

// Connect Wi-Fi
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.println("Connecting to");
  
  if ( wifi_ssid[0]=='\0' ){
    M5.Lcd.println("Previous SSID");
    WiFi.begin();
  }
  else{
    M5.Lcd.println(wifi_ssid);
    WiFi.begin(wifi_ssid,wifi_passwd);
  }
  
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    M5.Lcd.print(".");
  }

// Set ntp time to local
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.println("Request time from");
  M5.Lcd.println(ntpServer);
  configTime(9 * 3600, 0, ntpServer);

// Get local time
  struct tm timeInfo;
  if (getLocalTime(&timeInfo)) {
    // Set RTC time
    RTC_TimeTypeDef TimeStruct;
    TimeStruct.Hours   = timeInfo.tm_hour;
    TimeStruct.Minutes = timeInfo.tm_min;
    TimeStruct.Seconds = timeInfo.tm_sec;
    M5.Rtc.SetTime(&TimeStruct);

    RTC_DateTypeDef DateStruct;
    DateStruct.WeekDay = timeInfo.tm_wday;
    DateStruct.Month = timeInfo.tm_mon + 1;
    DateStruct.Date = timeInfo.tm_mday;
    DateStruct.Year = timeInfo.tm_year + 1900;
    M5.Rtc.SetData(&DateStruct);
  }

// Disconnect WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

// Display title
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(30, 0, 2);
  M5.Lcd.println("Abacus Clock");
}
void loop(){
  static char week_str[][4]= { {"Sun"}, {"Mon"}, {"Tue"}, {"Wed"}, {"Thu"}, {"Fri"}, {"Sat"} };
  static char nums[] = {0xF1, 0xE9, 0xD9, 0xB9, 0x79, 0xF2, 0xEA, 0xDA, 0xBA, 0x7A};
  static int  old_sec=0;
  char str[8];
  char p1[8], p2[8];
  int  i, n;
  
  do{
    delay(1);
    M5.Rtc.GetTime(&RTC_TimeStruct);
  }while(RTC_TimeStruct.Seconds==old_sec);
  old_sec = RTC_TimeStruct.Seconds;
  M5.Rtc.GetData(&RTC_DateStruct);
  
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.printf("Data: %04d-%02d-%02d\n", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date);
//  M5.Lcd.printf(" (%s)\n", week_str[RTC_DateStruct.WeekDay]);
  M5.Lcd.printf("Time: %02d : %02d : %02d\n", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);

  sprintf(str,"%02d/%02d/%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
  for( i=0; i<8; i++ ){
    p1[i] = nums[str[i]-'0'];  
  }
  for( i=0; i<8; i++ ){
    p2[i] = p1[i]|0x04;
  }
  
//  Serial.printf("%02x,%02x,%02x,%02x\n", p1[0],p1[1],p2[0],p2[1]);

  n = 8;

  matrix_bright(1);
  matrix_putimg(p1);
  delay(500);
  for(i=0;i<450/n;i++){
    matrix_putimg(p2);
    delay(1);
    matrix_putimg(p1);
    delay(n-1);
  }
}
