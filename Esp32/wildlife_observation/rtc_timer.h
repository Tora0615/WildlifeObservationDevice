#include "setting.h"

#ifndef RTC_TIMER_H
#define RTC_TIMER_H

#define RTC_PMOS 17

#include "RTClib.h"
RTC_DS3231 rtc;  // default I2C address is : 0x57 (you can choose 0x57 ~ 0x50)
DateTime now;
bool isInit = true;

const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
uint32_t sys_RTC_time_offset;         // RTC clock. It will only change when first boot or date changed. First day equal the boot time, others day it will allways close to 0
uint32_t sys_millis_time_offset;    // millis clock.

void RTCInit(){
  // Power part 
  pinMode(RTC_PMOS, OUTPUT);
  #ifdef RTC_DEBUG
    Serial.println("Turn off DS3231_RTC POWER");
  #endif 
  digitalWrite(RTC_PMOS,HIGH);   // Turn off. GPIO default is low ->  will let mic ON

  // init lib part
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1) delay(1000);
  }
}


void turnOnRtcPower(){
  // power on
  #ifdef RTC_DEBUG
    Serial.println("Turn on DS3231_RTC POWER");
  #endif 
  digitalWrite(RTC_PMOS, LOW);   // Turn on.

  if(isInit){
    delay(100);  // by task_scheduler
  }
}

// use for first boot or day change 
uint32_t GetHowManySecondsHasPassedTodayFromRtc(){    // since today 0:00
  // power on
  if(isInit){
    turnOnRtcPower();  // by task_scheduler
    isInit = !isInit;
  }
  
  // a day is 86400 sec
  now = rtc.now(); 

  // power off 
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

uint32_t getPassedSecOfToday(){
  return sys_RTC_time_offset + (millis() - sys_millis_time_offset)/1000;  // RTC time when boot + (time counter now - time counter before)
}



#endif