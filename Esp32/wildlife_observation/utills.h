#ifndef UTILLS_H
#define UTILLS_H

#include "setting.h"
#include "sd_operation.h"

// if day changed, create new folder
void checkDayChange(){
  if( getPassedSecOfToday() > 86400){  // a day change 86400
    sys_RTC_time_offset = GetHowManySecondsHasPassedTodayFromRtc();
    sys_millis_time_offset = millis();
    Serial.println("day changed");
      // Write log
    writeMsgToPath(systemLogPath, "Day changed");
  }
}



void checkScheduleFile(){
  if (!sd.exists(SCHEDULE_FILE.c_str())){
    // print error
    Serial.println("Init failed! Don't have file : " + SCHEDULE_FILE );
    writeMsgToPath(systemLogPath, "Init failed! Don't have file : " + SCHEDULE_FILE + ". Please see exampleSchedule.txt");
    
    // write example file
    writeMsgToPath("example_schedule.txt", 
      "00:00, A, 6, 30, B, 100\n"
      "00:00, B, 0.5\n"
      "00:00, C, 0.5\n"
      "00:00, D, 1\n"
      "\n"
      "#---------\n"
      "任務代碼 : \n"
      "A : Sound (INMP441)\n"
      "B : temperature & moisture (DHT22)\n"
      "C : temperature (DS18B20)\n"
      "D : Battery voltage\n"
      "\n"
      "參數說明 : \n"
      "任務 A\n"
      "初始時間, 任務代碼, 執行間隔(hr), 執行時間(min), L/R/B (聲道左/右/兩者), 音量(基準為100%)\n"
      "任務 BCD\n"
      "初始時間, 任務代碼, 執行間隔(hr)\n"
    , true);

    // delay 
    while(1) delay(10000);
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