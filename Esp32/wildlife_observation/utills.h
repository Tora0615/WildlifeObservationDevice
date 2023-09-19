#ifndef UTILLS_H
#define UTILLS_H

#include "setting.h"
#include "sd_operation.h"

// if day changed, create new folder
void checkDayChange(){
  if( getPassedSecOfToday() > 86400){  // a day : 86400 sec
    // if > a day, calibrate with RTC timer
    sys_RTC_time_offset = GetHowManySecondsHasPassedTodayFromRtc();
    sys_millis_time_offset = millis();
    Serial.println("day changed");
    // Write log
    writeMsgToPath(systemLogPath, "Day changed");
  }
}







void showInitFinishedLED(){
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH); 
  delay(100);
  digitalWrite(16, LOW); 
  delay(100);
  digitalWrite(16, HIGH); 
  delay(100);
  digitalWrite(16, LOW); 
  delay(100);
  digitalWrite(16, HIGH); 
  delay(100);
  digitalWrite(16, LOW); 
  delay(100);
  digitalWrite(16, HIGH); 
  delay(3000);
  digitalWrite(16, LOW); 
}


#endif