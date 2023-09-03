#include "RTClib.h
RTC_DS3231 rtc;

// default I2C address is : 0x57 (you can choose 0x57 ~ 0x50)

const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};



if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }



