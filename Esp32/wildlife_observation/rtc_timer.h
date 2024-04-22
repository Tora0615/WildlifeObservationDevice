/*---- include guard ----*/
#ifndef RTC_TIMER_H
#define RTC_TIMER_H

/*---- macro or define ----*/
#define RTC_PMOS 17

/*---- official lib ----*/ 
#include <RTClib.h>

/*---- other involve lib  ----*/
#include "setting.h"

/*---- variables or function define  ----*/
DateTime now;
RTC_DS3231 rtc;  // default I2C address is : 0x57 (you can choose 0x57 ~ 0x50)
uint32_t sys_RTC_time_offset;         // RTC clock. It will only change when first boot or date changed. First day equal the boot time, others day it will allways close to 0
uint32_t sys_millis_time_offset;    // millis clock.
const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


/*-------- function implement --------*/
void RTCInit(){
  // init lib part
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1) delay(1000);
  }
  
  // Power part off
  pinMode(RTC_PMOS, OUTPUT);
  #ifdef RTC_DEBUG
    Serial.println("Turn off DS3231_RTC POWER");
  #endif 
  digitalWrite(RTC_PMOS,HIGH);   // Turn off. GPIO default is low ->  will let it ON
}


void turnOnRtcPower(){
  // power on
  #ifdef RTC_DEBUG
    Serial.println("Turn on DS3231_RTC POWER");
  #endif 
  digitalWrite(RTC_PMOS, LOW);   // Turn on.

  // wait it powered up
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

// use for first boot or day change 
uint32_t GetHowManySecondsHasPassedTodayFromRtc(){    // since today 0:00
  // power on
  turnOnRtcPower();  

  // a day is 86400 sec
  now = rtc.now(); 

  // stop i2c pullup
  gpio_pullup_dis(GPIO_NUM_21);
  gpio_pullup_dis(GPIO_NUM_22);

  // power off PMOS
  #ifdef RTC_DEBUG
    Serial.println("Turn off DS3231_RTC POWER");
  #endif 
  digitalWrite(RTC_PMOS, HIGH);   // Turn off. GPIO default is low ->  will let mic ON

  // return part 
  return now.hour() * 60 * 60 + now.minute() * 60 + now.second();
}


String getDate(){
  String date;
  date += now.year();
  date += now.month()<10 ? "0" + String(now.month()) : String(now.month());
  date += now.day()<10 ? "0" + String(now.day()) : String(now.day());
  return date; 
}


String secMapTo24Hour(uint32_t sec){
  int min = sec / 60;
  sec = sec % 60;
  int hour = min / 60;
  min = min % 60;

  String time;
  time += hour < 10 ? "0" + String(hour) : String(hour);
  time += min < 10 ? "0" + String(min) : String(min);
  time += sec < 10 ? "0" + String(sec) : String(sec);

  return time;
}

/*
    <- wakeup delay
|--------|------------------|--------------------|
0      RTC_off          now(millis)             86400
    = millis_off
*/
uint32_t getPassedSecOfToday(){
  return sys_RTC_time_offset + (millis() - sys_millis_time_offset)/1000;  // RTC time when boot + (time counter now - time counter before)
}

uint32_t getPassedMilliSecOfToday(){
  return sys_RTC_time_offset * 1000 + (millis() - sys_millis_time_offset);  // RTC time when boot + (time counter now - time counter before)
}


void printNow(){
  turnOnRtcPower();  
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  digitalWrite(RTC_PMOS,HIGH);   // Turn off.
}

void setTestTime(){
  turnOnRtcPower();  
  rtc.adjust(DateTime(2023, 10, 10, 0, 43, 0));
  digitalWrite(RTC_PMOS, HIGH);   // Turn off.

  Serial.println("RTC test mode ON");
  printNow();
}

void setTime(char timeWords[]){
  // parse the string
  int YEAR    = (timeWords[0] - '0')*1000 + (timeWords[1] - '0')*100 + (timeWords[2] - '0')*10 + (timeWords[3] - '0');
  int MONTH   = (timeWords[4] - '0')*10 + (timeWords[5] - '0');
  int DAY     = (timeWords[6] - '0')*10 + (timeWords[7] - '0');
  int HOUR_24 = (timeWords[8] - '0')*10 + (timeWords[9] - '0');
  int MINUTE  = (timeWords[10] - '0')*10 + (timeWords[11] - '0');
  int SECOND  = (timeWords[12] - '0')*10 + (timeWords[13] - '0');

  // execute set time 
  turnOnRtcPower();  
  rtc.adjust(DateTime(YEAR, MONTH, DAY, HOUR_24, MINUTE, SECOND));
  digitalWrite(RTC_PMOS,HIGH);   // Turn off.

  Serial.print("Set time from file successful, now : ");
  printNow();
}



#endif