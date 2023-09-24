#ifndef MYSCHEDULER_H
#define MYSCHEDULER_H

// scheduler involve
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskScheduler.h>
Scheduler runner;

// others involve 
#include "setting.h"
#include "sd_operation.h"
#include "battery.h"
#include "rtc_timer.h"
#include "sd_operation.h"
#include "myDS18B20.h"
#include "utills.h"
#include "myDHT.h"
#include "myINMP441.h"


// some global variable for task use 

void checkDayChange();
void checkIsNeedToRunTask();
void recordDS18B20();
void recordDHT();
void recordSound();
void recordBattery();
void f_turnOnDs18b20Power();
void f_getDS18B20Temp();
void f_turnOnDhtPower();
void f_DHT_get_temperature();
void f_DHT_get_Humidity();
void f_turnOnRtcPower();
void f_GetHowManySecondsHasPassedTodayFromRtc();


// top level
Task t_checkDayChange(5000, TASK_FOREVER, &checkDayChange);
Task t_checkIsNeedToRunTask(10000, TASK_FOREVER, &checkIsNeedToRunTask);
// second level 
Task t_recordSound(0, TASK_FOREVER, &recordSound);
Task t_recordBattery(0, TASK_FOREVER, &recordBattery);
Task t_recordDS18B20(0, TASK_FOREVER, &recordDS18B20);
Task t_recordDHT(0, TASK_FOREVER, &recordDHT);


// global variable for record 
float recordTime;
int channel;
float gain_ratio;

void checkIsNeedToRunTask(){
  Serial.println("checkIsNeedToRunTask");
  // get next task's start time 
  int startTimeOfNext = 0;
  if((taskArray + arrayReadIndex)->setType == 0){  // simple task
    startTimeOfNext = (taskArray + arrayReadIndex)->taskType.simple.start_min_of_a_day;
  }else{
    startTimeOfNext = (taskArray + arrayReadIndex)->taskType.complex.start_min_of_a_day;
  }

  // need to run task 
  if( getPassedSecOfToday() > startTimeOfNext * 60 ){  // change to sec and compare

    char task_code;
    // get task code 
    if((taskArray + arrayReadIndex)->setType == 0){  // simple task
      task_code = (taskArray + arrayReadIndex)->taskType.simple.task;
    }else{
      task_code = (taskArray + arrayReadIndex)->taskType.complex.task;
    }

    // TODO : write log  
    // start the task according to task code. A is record, will get more info.
    if (task_code == 'A'){
      // sound record 
      Serial.println(String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task A (sound record )." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
      recordTime = (taskArray + arrayReadIndex)->taskType.complex.time;
      channel = (taskArray + arrayReadIndex)->taskType.complex.channel;
      gain_ratio = (taskArray + arrayReadIndex)->taskType.complex.multiple;
      // runner.addTask(t_recordSound);
      // t_recordSound.enable();
    }else if (task_code == 'B'){
      // DHT
      Serial.println( String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task B (DHT)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
      // runner.addTask(t_recordDHT);
      // t_recordDHT.enable();
    }else if (task_code == 'C'){
      // DS18B20
      Serial.println(String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task C (DS18B20)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
      // runner.addTask(t_recordDS18B20);
      // t_recordDS18B20.enable();
    }else if (task_code == 'D'){
      // battery 
      Serial.println(String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task D (battery)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
      // runner.addTask(t_recordBattery);
      // t_recordBattery.enable();
    }

    // plus index or re-zero 
    if(arrayReadIndex == arrayUsedIndex){
      arrayReadIndex = 0;
    }else{
      arrayReadIndex += 1;
    }
  }
}


void recordSound(){
  
}


void recordBattery(){
  Serial.println("Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)");
  writeMsgToPath(systemLogPath, "Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)");
}


// for recongnize
void recordDS18B20(){
  t_recordDS18B20.setCallback(&f_turnOnDs18b20Power);
}
void f_turnOnDs18b20Power(){
  turnOnDs18b20Power();
  t_recordDS18B20.setCallback(&f_getDS18B20Temp);
  t_recordDS18B20.delay(100);
}
void f_getDS18B20Temp(){
  float temperature = getDS18B20Temp();
  // print 
  Serial.println("DS18B20 : " + String(temperature));
  // write SD 
  writeMsgToPath(systemLogPath, "DS18B20 : " + String(temperature) );
  t_recordDS18B20.setCallback(&recordDS18B20);
  runner.deleteTask(t_recordDS18B20);
}


void recordDHT(){
  t_recordDHT.setCallback(&f_turnOnDhtPower);
}
void f_turnOnDhtPower(){
  turnOnDhtPower();
  t_recordDHT.setCallback(&f_DHT_get_temperature);
  t_recordDHT.delay(200);
}
void f_DHT_get_temperature(){
  float temperature = DHT_get_temperature();
  // write SD 
  t_recordDHT.setCallback(&f_DHT_get_Humidity);
}
void f_DHT_get_Humidity(){
  float humidity = DHT_get_Humidity();
  // write SD 
  t_recordDHT.setCallback(&recordDHT);
  runner.deleteTask(t_recordDHT);
}



// if day changed, create new folder
void checkDayChange(){
  Serial.println("check day change");
  t_checkDayChange.setCallback(&f_turnOnRtcPower);
}
void f_turnOnRtcPower(){
  turnOnRtcPower();
  t_checkDayChange.setCallback(&f_GetHowManySecondsHasPassedTodayFromRtc);
  t_checkDayChange.delay(100);
}
void f_GetHowManySecondsHasPassedTodayFromRtc(){
  if( getPassedSecOfToday() > 86400){  // a day : 86400 sec
    // if > a day, calibrate with RTC timer
    sys_RTC_time_offset = GetHowManySecondsHasPassedTodayFromRtc();
    sys_millis_time_offset = millis();
    Serial.println("day changed");
    // Write log
    writeMsgToPath(systemLogPath, "Day changed");
  }
  t_checkDayChange.setCallback(&checkDayChange);
  runner.deleteTask(t_checkDayChange);
}












// taskArray

// arrayReadIndex



// if((taskArray+arrayReadIndex)->setType == 0){  // simple task
//       Serial.print("start_min_of_a_day : " + String( (taskArray+index)->taskType.simple.start_min_of_a_day ) + "(" + String( minConvertTohour24( (taskArray+index)->taskType.simple.start_min_of_a_day ) ) + ")");
//       Serial.println(" / task : " + String( (taskArray+index)->taskType.simple.task ));
//     }else{  // complex task
//       Serial.print("start_min_of_a_day : " + String( (taskArray+index)->taskType.complex.start_min_of_a_day ) + "(" + String( minConvertTohour24((taskArray+index)->taskType.complex.start_min_of_a_day) ) + ")");
//       Serial.print(" / task : " + String( (taskArray+index)->taskType.complex.task ));
//       Serial.print(" / time : " + String( (taskArray+index)->taskType.complex.time ));
//       Serial.print(" / channel : " + String( (taskArray+index)->taskType.complex.channel ));
//       Serial.println(" / multiple : " + String( (taskArray+index)->taskType.complex.multiple ));
//     }





#endif