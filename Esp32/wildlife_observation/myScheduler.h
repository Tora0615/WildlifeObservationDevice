#ifndef MYSCHEDULER_H
#define MYSCHEDULER_H

// scheduler involve
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskScheduler.h>
Scheduler runner;
// day change involve 
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




void checkIsNeedToRunTask(){
  // get next task's start time 
  if((taskArray + arrayReadIndex)->setType == 0){  // simple task
    int startTimeOfNext = (taskArray + arrayReadIndex)->taskType.simple.start_min_of_a_day;
  }else{
    int startTimeOfNext = (taskArray + arrayReadIndex)->taskType.complex.start_min_of_a_day;
  }

  // need to run task 
  if( getPassedSecOfToday() > startTimeOfNext*60 ){  // change to sec and compare

  }

  // plus index or re-zero 
  if(arrayReadIndex == arrayUsedIndex){
    arrayReadIndex = 0;
  }else{
    arrayReadIndex += 1;
  }
}


void(){

}




taskArray

arrayReadIndex



if((taskArray+arrayReadIndex)->setType == 0){  // simple task
      Serial.print("start_min_of_a_day : " + String( (taskArray+index)->taskType.simple.start_min_of_a_day ) + "(" + String( minConvertTohour24( (taskArray+index)->taskType.simple.start_min_of_a_day ) ) + ")");
      Serial.println(" / task : " + String( (taskArray+index)->taskType.simple.task ));
    }else{  // complex task
      Serial.print("start_min_of_a_day : " + String( (taskArray+index)->taskType.complex.start_min_of_a_day ) + "(" + String( minConvertTohour24((taskArray+index)->taskType.complex.start_min_of_a_day) ) + ")");
      Serial.print(" / task : " + String( (taskArray+index)->taskType.complex.task ));
      Serial.print(" / time : " + String( (taskArray+index)->taskType.complex.time ));
      Serial.print(" / channel : " + String( (taskArray+index)->taskType.complex.channel ));
      Serial.println(" / multiple : " + String( (taskArray+index)->taskType.complex.multiple ));
    }





#endif