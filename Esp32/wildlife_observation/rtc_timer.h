#include "RTClib.h"
RTC_DS3231 rtc;  // default I2C address is : 0x57 (you can choose 0x57 ~ 0x50)
DateTime now;

const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void RTCInit(){
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1) delay(1000);
  }
}


// use for first boot 
uint32_t GetHowManySecondsHasPassedTodayFromRtc(){    // since today 0:00
  // a day is 86400 sec
  now = rtc.now(); 
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