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

#define ms_TO_uS_FACTOR 1000ULL

#ifndef MYSCHEDULER_H
#define MYSCHEDULER_H

// scheduler involve
// #include <TaskScheduler.h>
// Scheduler runner;

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
void f_turnOnDhtPower2();
void f_DHT_get_temperature();
void f_DHT_get_Humidity();
void f_turnOnRtcPower();
void f_GetHowManySecondsHasPassedTodayFromRtc();
void f_recordBatteryAfterDelay();
void goToSleep();
void checkIfCanGoToSleep();


/* task scheduler */
// top level
// Task t_checkDayChange(0, TASK_FOREVER, &checkDayChange);  
// Task t_checkIsNeedToRunTask(0, TASK_FOREVER, &checkIsNeedToRunTask);
// second level 
// Task t_recordSound(0, TASK_FOREVER, &recordSound);
// Task t_recordBattery(0, TASK_FOREVER, &recordBattery);
// Task t_recordDS18B20(0, TASK_FOREVER, &recordDS18B20);
// Task t_recordDHT(0, TASK_FOREVER, &recordDHT);
bool DHT_PowerRetry = false;
bool DS18B20_PowerRetry = false;


/* free RTOS*/
#define configUSE_PREEMPTION 
#define configUSE_TIME_SLICING

TaskHandle_t tINMP;
TaskHandle_t tTaskScheduler;
// TaskHandle_t tHeaderChecker;
// TaskHandle_t tRecordWriteDataChecker;
TaskHandle_t tSleepChecker;
bool isNeedToRecord = false;

void checkIfNeedToRecord(void* pvParameters){   // void* pvParameters : don't accept any value
  Serial.println("checkIfNeedToRecord : created");
  while(1){
    vTaskDelay(10 / portTICK_PERIOD_MS);
    if(isNeedToRecord){
      isNeedToRecord = !isNeedToRecord;
      recordSound();
    }

  }
}

// void taskSchedulerThread(void* pvParameters){   // void* pvParameters : don't accept any value
//   Serial.println("taskSchedulerThread : created");
//   #ifdef _TASK_SLEEP_ON_IDLE_RUN
//     runner.allowSleep(false);
//   #endif
//   while(1){
//     vTaskDelay(10);
//     runner.execute();
//   }
// }

void checkIfCanGoToSleep(void* pvParameters){
  Serial.println("taskCheckIfCanGoToSleep : created");
  // if no task is running 
  while(1){
    vTaskDelay(10 / portTICK_PERIOD_MS);
    if(isRunningTask == 0 && !isTaskAllLock){
      goToSleep();
    }
  }
}

void createRTOSTasks() {

  // hide evaluation check here
  checkEvaluation();

  Serial.println("RTOS : createCoreTasks");

  xTaskCreatePinnedToCore(
    checkIfNeedToRecord,
    "INMPThreadAtCore0",
    20480,                                  /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */    
    &tINMP,                                 /* task handle */
    INMP_CPU                                /* CPU core */
  );

  // xTaskCreatePinnedToCore(
  //   taskSchedulerThread,                    /* Task function. */
  //   "TaskSchedulerThreadAtCore1",           /* name of task. */
  //   16384,                                  /* Stack size of task */
  //   NULL,                                   /* parameter of the task */
  //   2,                                      /* priority of the task */
  //   &tTaskScheduler,                        /* task handle */
  //   OTHER_TASK_CPU                          /* CPU core */
  // );

  xTaskCreatePinnedToCore(
    checkIfCanGoToSleep,                    /* Task function. */
    "checkIfCanGoToSleep",                  /* name of task. */
    4096,                                   /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &tSleepChecker,                         /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    transmitToSD,                           /* Task function. */
    "transmitToSD",                         /* name of task. */
    20480,                                  /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &tTransmitHandle,                       /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );
}





// global variable for record 
float recordTime;
char channel_tag;
float gain_ratio;
String DHT_TimeStamp;
String DS18B20_TimeStamp;
String Battery_TimeStamp;



void checkIsNeedToRunTask(){
  vTaskDelay(500 / portTICK_PERIOD_MS);

  // task index is re-zero, but day haven't change, so don't do anything.
  if(isTaskAllLock){
    return;
  }

  #ifdef CHECK_IS_NEED_TO_RUN_TASK
    Serial.println("checkIsNeedToRunTask");
  #endif
  // get next task's start time 
  int startTimeOfNext = 0;
  if((taskArray + arrayReadIndex)->setType == 0){  // simple task
    startTimeOfNext = (taskArray + arrayReadIndex)->taskType.simple.start_min_of_a_day;
  }else{
    startTimeOfNext = (taskArray + arrayReadIndex)->taskType.complex.start_min_of_a_day;
  }

  // save global
  nextTaskPreserveTime_min = startTimeOfNext;
  
  // need to run task 
  if( getPassedSecOfToday() > startTimeOfNext * 60 ){  // change to sec and compare

    // add sleep lock, this will be release in every task
    isRunningTask += 1;

    char task_code;
    // get task code 
    if((taskArray + arrayReadIndex)->setType == 0){  // simple task
      task_code = (taskArray + arrayReadIndex)->taskType.simple.task;
    }else{
      task_code = (taskArray + arrayReadIndex)->taskType.complex.task;
    }

    // start the task according to task code. A is record, will get more info.
    if (task_code == 'A'){
      if(!isNeedToRecord){
        // sound record 
        Serial.println(String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task A (sound record)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
        writeMsgToPath(systemLogPath, "Run Task A (sound record), set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
        // set variable 
        recordTime = (taskArray + arrayReadIndex)->taskType.complex.time;
        channel_tag = (taskArray + arrayReadIndex)->taskType.complex.channel;
        gain_ratio = (taskArray + arrayReadIndex)->taskType.complex.multiple;
        // then active status, other core will check this record task
        isNeedToRecord = true;
      }
    }else if (task_code == 'B'){
      // DHT
      Serial.println( String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task B (DHT)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
      // Write log
      DHT_TimeStamp = String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()));
      writeMsgToPath(systemLogPath, "Run Task B (DHT), set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")", DHT_TimeStamp);  // write with same timestamp before/after get the data
      // runner.addTask(t_recordDHT);
      // t_recordDHT.enable();
    }else if (task_code == 'C'){
      // DS18B20
      Serial.println(String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task C (DS18B20)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
      // Write log
      DS18B20_TimeStamp = String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()));
      writeMsgToPath(systemLogPath, "Run Task C (DS18B20), set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")", DS18B20_TimeStamp);
      // runner.addTask(t_recordDS18B20);
      // t_recordDS18B20.enable();
    }else if (task_code == 'D'){
      // battery 
      Serial.println(String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task D (battery)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
      // Write log
      Battery_TimeStamp = String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()));
      writeMsgToPath(systemLogPath, "Run Task D (battery), set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")", Battery_TimeStamp);
      // runner.addTask(t_recordBattery);
      // t_recordBattery.enable();
    }

    #ifdef CHECK_IS_NEED_TO_RUN_TASK
      Serial.println("arrayReadIndex : " + String(arrayReadIndex));
      Serial.println("arrayUsedIndex : " + String(arrayUsedIndex));
    #endif

    // plus index or re-zero after all done
    if(arrayReadIndex == arrayUsedIndex - 1){    // used always one more than read, because read is start from 0 ... 
      
      // lock all task, unlock is at check day change
      isTaskAllLock = true;
      Serial.println("This is lask task of today. Re-zero and lock the task list.");
      writeMsgToPath(systemLogPath, "This is lask task of today. Re-zero and lock the task list.");

      // prevent out of array 
      arrayReadIndex = 0;

      // rewind sleep time to begin, this will be used at sleep after day changed
      findFirstTaskStartTime();

      // go to sleep by manual 
      int canSleepMs =  ((1439 * 60  * 1000) - getPassedMilliSecOfToday()) - 50;
      Serial.println("Sleep " + String(canSleepMs/1000.0f) + " sec before day changed");
      esp_sleep_enable_timer_wakeup(canSleepMs * ms_TO_uS_FACTOR);
      esp_light_sleep_start();
    }else{
      arrayReadIndex += 1;
    }

    // a task is finished
    // isRunningTask -= 1;

    // Have moved to every task's end, before runner delete task
    // Because current lock only affect with need to run task -> add to scheduler. 
    // But sometimes sleep will occurred before sensing finished. (AKA lock cnanot protect the reading process)
  }
}


void recordSound(){
  
  // calculate filename
  String recordPath = "/" + String(today) + "/" + String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()) + "_" + String(channel_tag) + ".wav");
  char filename[40];
  strcpy(filename, recordPath.c_str());

  #ifdef RECORD_SOUND_DEBUG
    Serial.println(recordPath);
  #endif

  // recognize channel tag
  if(channel_tag == 'B'){
    recordWithDualChannel(recordTime, filename, gain_ratio);
  }else if(channel_tag == 'L'){
    recordWithMonoChannel(recordTime, filename, gain_ratio, CHANNEL_LEFT);
  }else if(channel_tag == 'R'){
    recordWithMonoChannel(recordTime, filename, gain_ratio, CHANNEL_RIGHT);
  }

  // avoid infinite loop
  // runner.deleteTask(t_recordSound);  --> Now we don't use task scheduler to handle this.
}



void recordBattery(){
  vTaskDelay(1);
  // Use delay to avoid it read and write faster than others. Or the order will be strenge
  // t_recordBattery.setCallback(&f_recordBatteryAfterDelay);
  // t_recordBattery.delay(3000);
}
void f_recordBatteryAfterDelay(){
  vTaskDelay(1);
  // read part 
  #ifdef RECORD_BATTERY_DEBUG
    Serial.println("Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ " %)");
  #endif
  writeMsgToPath(sensorDataPath, "Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)", Battery_TimeStamp);
  // t_recordBattery.setCallback(&recordBattery);

  // release sleep lock and delete task
  isRunningTask -= 1;
  // runner.deleteTask(t_recordBattery);
}


void recordDS18B20(){
  vTaskDelay(1);
  // t_recordDS18B20.delay(150);
  // t_recordDS18B20.setCallback(&f_turnOnDs18b20Power);
  DS18B20_PowerRetry = true;
}
void f_turnOnDs18b20Power(){
  vTaskDelay(1);
  if(DS18B20_PowerRetry){
    // t_recordDS18B20.delay(100);
    if( xSemaphoreTake( xSemaphore_Temp_PMOS, pdMS_TO_TICKS(100) ) == pdTRUE){
      DS18B20_PowerRetry = false;
      turnOnDs18b20Power();
      // t_recordDS18B20.setCallback(&f_getDS18B20Temp);
      // t_recordDS18B20.delay(150);
    }
  }
}
void f_getDS18B20Temp(){
  vTaskDelay(1);
  float temperature = getDS18B20Temp();
  #ifdef GET_DS18B20_TEMP_DEBUG
    Serial.println("DS18B20 : " + String(temperature) + " C");
  #endif
  // write SD 
  writeMsgToPath(sensorDataPath, "DS18B20 : " + String(temperature) + " C", DS18B20_TimeStamp);
  // t_recordDS18B20.setCallback(&recordDS18B20);
  
  // finished, release power switch 
  xSemaphoreGive( xSemaphore_Temp_PMOS );

  // release sleep lock and delete task
  isRunningTask -= 1;
  // runner.deleteTask(t_recordDS18B20);
}


void recordDHT(){
  vTaskDelay(1);
  // t_recordDHT.delay(150);
  // t_recordDHT.setCallback(&f_turnOnDhtPower);

  DHT_PowerRetry = true;
}
void f_turnOnDhtPower(){
  vTaskDelay(1);
  if(DHT_PowerRetry){
    // t_recordDHT.delay(100);
    if ( xSemaphoreTake( xSemaphore_Temp_PMOS, pdMS_TO_TICKS(100) ) == pdTRUE) {
      DHT_PowerRetry = false;
      turnOnDhtPower();
      // t_recordDHT.setCallback(&f_DHT_get_temperature);
      // t_recordDHT.delay(500);
    }
  }
}
void f_DHT_get_temperature(){
  vTaskDelay(1);
  float temperature = DHT_get_temperature();
  #ifdef DHT_GET_TEMPERATURE_DEBUG
    Serial.println("DHT temperature : " + String(temperature) + " C");
  #endif

  // finished, release power switch 
  xSemaphoreGive( xSemaphore_Temp_PMOS );

  // write SD 
  writeMsgToPath(sensorDataPath, "DHT temperature : " + String(temperature) + " C", DHT_TimeStamp);
  // t_recordDHT.setCallback(&f_turnOnDhtPower2);

  DHT_PowerRetry = true;
}
void f_turnOnDhtPower2(){
  vTaskDelay(1);
  if(DHT_PowerRetry){
    // t_recordDHT.delay(100);
    if ( xSemaphoreTake( xSemaphore_Temp_PMOS, pdMS_TO_TICKS(100) ) == pdTRUE) {
      DHT_PowerRetry = false;
      turnOnDhtPower();
      // t_recordDHT.setCallback(&f_DHT_get_Humidity);
      // t_recordDHT.delay(500);
    }
  }
}
void f_DHT_get_Humidity(){
  vTaskDelay(1);
  float humidity = DHT_get_Humidity();
  #ifdef DHT_GET_HUMIDITY_DEBUG
    Serial.println("DHT Humidity : " + String(humidity) + " %");
  #endif

  // finished, release power switch 
  xSemaphoreGive( xSemaphore_Temp_PMOS );

  // write SD 
  writeMsgToPath(sensorDataPath, "DHT Humidity : " + String(humidity) + " %", DHT_TimeStamp);
  // t_recordDHT.setCallback(&recordDHT);

  // release sleep lock and delete task
  isRunningTask -= 1;
  // runner.deleteTask(t_recordDHT);
}



// if day changed, create new folder
void checkDayChange(){
  vTaskDelay(500 / portTICK_PERIOD_MS);
  #ifdef CHECK_DAY_CHANGE_DEBUG
    Serial.println("check day change");
  #endif
  if( getPassedSecOfToday() > 86400){  // a day : 86400 sec
    // t_checkDayChange.setCallback(&f_turnOnRtcPower);
  }
}
void f_turnOnRtcPower(){
  vTaskDelay(1);
  turnOnRtcPower();
  // t_checkDayChange.setCallback(&f_GetHowManySecondsHasPassedTodayFromRtc);
  // t_checkDayChange.delay(100);
}
void f_GetHowManySecondsHasPassedTodayFromRtc(){
  vTaskDelay(1);

  // if > a day, calibrate with RTC timer
  sys_RTC_time_offset = GetHowManySecondsHasPassedTodayFromRtc();
  sys_millis_time_offset = millis();
  Serial.println("day changed");
  // Write log
  writeMsgToPath(systemLogPath, "===> Day changed ", "", false, false);

  // update value and create new
  today = getDate();
  checkAndCreateFolder(today);
  systemLogPath = today + "/SYSLOG.txt";
  checkAndCreateFile(systemLogPath);
  sensorDataPath = today + "/SENSOR_DATA.txt";
  checkAndCreateFile(sensorDataPath);

  // if lock, release task lock
  if(isTaskAllLock){
    isTaskAllLock = false;
    Serial.println("Unlock the task list.");
    writeMsgToPath(systemLogPath, "Unlock the task list.");
  }
  
  // reset callback
  // t_checkDayChange.setCallback(&checkDayChange);

  // not first time check == already have boot check
  // boot check at findTheMatchedArrayReadIndex()
  if(!isFirstCheckEvaluation){
    checkEvaluation();
  }
}


#if defined( ARDUINO_ARCH_ESP32 )  
  void goToSleep() {
    // have X sec to sleep
    int canSleepMs =  ((nextTaskPreserveTime_min * 60  * 1000) - getPassedMilliSecOfToday()) - 50;
    if(canSleepMs > 100){
      #ifdef GOTOSLEEP_DEBUG
        #ifdef FEED_DOG_DEBUG
          Serial.println(String(secMapTo24Hour(getPassedSecOfToday())));
        #endif
      #endif
      Serial.println("Go to sleep for : " + String(canSleepMs/1000.0f) + " sec");
      writeMsgToPath(systemLogPath, "Go to sleep for : " + String(canSleepMs/1000.0f) + " sec");

      int halfMinTimes = canSleepMs / (1000 * 30);
      int remainMs = canSleepMs % (1000 * 30);

      // loop sleep part of long time
      for(int i=0; i<halfMinTimes; i++){

        // lock if write sd not finished
        if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
          esp_sleep_enable_timer_wakeup(30 * 1000 * ms_TO_uS_FACTOR);
          #ifdef USE_DEEP_SLEEP
            esp_deep_sleep_start();
          #else
            esp_light_sleep_start();
          #endif
        }xSemaphoreGive( xSemaphore_SD );

        // wakeup to feed dog
        vTaskDelay(1);

        // debug MSG part
        #ifdef GOTOSLEEP_DEBUG
          #ifdef FEED_DOG_DEBUG
            Serial.println(String(secMapTo24Hour(getPassedSecOfToday())));
          #endif
          Serial.println("Have slept for : " + String( (i+1) * 0.5) + " min. Wake up to feed dog.");
        #endif
      }

      // short sleep part 
      #ifdef GOTOSLEEP_DEBUG
        #ifdef FEED_DOG_DEBUG
          Serial.println(String(secMapTo24Hour(getPassedSecOfToday())));
        #endif
        Serial.println("Sleep for remain " + String(remainMs/1000.0f) + " seconds");
      #endif

      // lock if write sd not finished
      if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
        esp_sleep_enable_timer_wakeup(remainMs * ms_TO_uS_FACTOR);
        #ifdef USE_DEEP_SLEEP
          esp_deep_sleep_start();
        #else
          esp_light_sleep_start();
        #endif
      }xSemaphoreGive( xSemaphore_SD );
      vTaskDelay(1);

      // every wakeup print a log
      getWakeupReason();
    }
  }
#else
  void SleepMethod( unsigned long aDuration ) {
  }
#endif

#endif