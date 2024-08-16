/*---- include guard ----*/
#ifndef MYSCHEDULER_H
#define MYSCHEDULER_H

/*---- macro or define ----*/
#define ms_TO_uS_FACTOR 1000ULL
// RTOS setting
#define configUSE_TIME_SLICING 1
#define configUSE_PREEMPTION 1

/*---- official lib ----*/ 
#include <stdint.h>
#include <sys/types.h>

/*---- other involve lib  ----*/
#include "myDHT.h"
#include "utills.h"   // include sd_operation / rtc_timer / setting
#include "myINMP441.h"
#include "myDS18B20.h"
#include "taskFileReader.h"

/*---- variables or function define  ----*/
float recordTime;
char channel_tag;
float gain_ratio;
String DHT_TimeStamp;
String DS18B20_TimeStamp;
String Battery_TimeStamp;
bool previousRoundOfSleepFinished;
void checkGoSleep(void* pvParameters);
void checkTimeAndTask(void* pvParameters);
void transmitSoundDataToSD(void* pvParameters);
void recordSound(void* pvParameters);
void recordDHT(void* pvParameters);
void recordDS18B20(void* pvParameters);
void recordBattery(void* pvParameters);
void checkDayChange();
void goToSleep(int sleepTime_sec);
void showLowBattery(void* pvParameters);
void showTaskRunningLED(void* pvParameters);
// void aliveLedShow();  // from led.h

/* RTOS handler */
TaskHandle_t tShowTaskRunningLEDHandler;
TaskHandle_t tShowLowBatteryHandler;
TaskHandle_t tCheckGoSleepHandler;
TaskHandle_t tCheckTimeAndTaskHandler;
  // TaskHandle_t tSdTransmitHandler;  --> at myINMP file
  TaskHandle_t tRecordSoundHandler;
  TaskHandle_t tRecordDHTHandler;
  TaskHandle_t tRecordDS18B20Handler;
  TaskHandle_t tRecordBatteryHandler;


/*-------- function implement --------*/
void createRTOSTasks() {
  // hide evaluation check here
  checkEvaluation();
  Serial.println("== RTOS : createCoreTasks");
  writeMsgToPath(systemLogPath, "RTOS : createCoreTasks");

  xTaskCreatePinnedToCore(
    showTaskRunningLED,                           /* Task function. */
    "showTaskRunningLED",                         /* name of task. */
    2048,                                  /* Stack size of task */
    NULL,                                   /* parameter of the task */
    3,                                      /* priority of the task */
    &tShowTaskRunningLEDHandler,                       /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    showLowBattery,                           /* Task function. */
    "showLowBattery",                         /* name of task. */
    2048,                                  /* Stack size of task */
    NULL,                                   /* parameter of the task */
    3,                                      /* priority of the task */
    &tShowLowBatteryHandler,                       /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    checkGoSleep,                           /* Task function. */
    "checkGoSleep",                         /* name of task. */
    4096,                                  /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &tCheckGoSleepHandler,                       /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    checkTimeAndTask,                           /* Task function. */
    "checkTimeAndTask",                         /* name of task. */
    12288,                                  /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &tCheckTimeAndTaskHandler,                       /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );


    xTaskCreatePinnedToCore(
      transmitSoundDataToSD,                           /* Task function. */
      "transmitSoundDataToSD",                         /* name of task. */
      16384,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      4,                                      /* priority of the task */
      &tSdTransmitHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordSound,                           /* Task function. */
      "recordSound",                         /* name of task. */
      32768,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      5,                                      /* priority of the task */
      &tRecordSoundHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordDHT,                           /* Task function. */
      "recordDHT",                         /* name of task. */
      4096,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      3,                                      /* priority of the task */
      &tRecordDHTHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */  
    );

    xTaskCreatePinnedToCore(
      recordDS18B20,                           /* Task function. */
      "recordDS18B20",                         /* name of task. */
      4096,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      3,                                      /* priority of the task */
      &tRecordDS18B20Handler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordBattery,                           /* Task function. */
      "recordBattery",                         /* name of task. */
      4096,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      3,                                      /* priority of the task */
      &tRecordBatteryHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );
}

// TODO : RTOS Error flag
// When have any error, set flag to true, then use highest task to block all tasks.

void showTaskRunningLED(void* pvParameters){
  Serial.println("|- showTaskRunningLED : created");
  writeMsgToPath(systemLogPath, "showTaskRunningLED : created");

  #ifdef STACK_DEBUG
  UBaseType_t uxHighWaterMark;
  #endif

  while(true){
    if(isRunningTask > 0){
      runningTaskLedShow();
    }
    vTaskDelay(30000 / portTICK_PERIOD_MS);

    #ifdef STACK_DEBUG
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println("#### showTaskRunningLED stack : " + String(uxHighWaterMark) + "(byte) ####");
    writeMsgToPath(systemLogPath, "#### showTaskRunningLED stack : " + String(uxHighWaterMark) + "(byte) ####");
    #endif
  }
}

void showLowBattery(void* pvParameters){
  Serial.println("|- showLowBattery : created");
  writeMsgToPath(systemLogPath, "showLowBattery : created");

  #ifdef STACK_DEBUG
  UBaseType_t uxHighWaterMark;
  #endif

  while(true){
    // check state
    if(!isLowBattery){
      if (getBatteryVoltage() < LOW_BATTERY_VOLTAGE){
        isLowBattery = true;
      }
    }
    // do the action
    // low battery : blink every 30 sec   
    if(isLowBattery) {
      lowBatteryLedShow();
      vTaskDelay(30000 / portTICK_PERIOD_MS);
    // normal : check every 5 min (== 300 sec == 300000 ms)
    }else{
      vTaskDelay(300000 / portTICK_PERIOD_MS);
    }

    #ifdef STACK_DEBUG
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println("#### showLowBattery stack : " + String(uxHighWaterMark) + "(byte) ####");
    writeMsgToPath(systemLogPath, "#### showLowBattery stack : " + String(uxHighWaterMark) + "(byte) ####");
    #endif
  }
}

void checkGoSleep(void* pvParameters){
  previousRoundOfSleepFinished = true;
  Serial.println("|- checkGoSleep : created");
  writeMsgToPath(systemLogPath, "checkGoSleep : created");
  
  #ifdef STACK_DEBUG
  UBaseType_t uxHighWaterMark;
  #endif
  
  // if no task is running 
  while(true){
    // block itself first, untill we resume it
    #ifdef RTOS_DETIAL
      Serial.println("checkGoSleep : suspended");
    #endif
    vTaskSuspend(NULL);

    // if called resume, will go to here
    #ifdef RTOS_DETIAL
      Serial.println("checkGoSleep : resumed");
    #endif
    Serial.println("* checkGoSleep");
    Serial.print("- isRunningTask : " + String(isRunningTask)); Serial.println(", previousRoundOfSleepFinished : " + String(previousRoundOfSleepFinished) + "\n");
    
    // no task running and the last triggered sleep was finished
    // or it might repeat trigger when temp wakeup
    if(isRunningTask == 0 && previousRoundOfSleepFinished){
      previousRoundOfSleepFinished = false;
      goToSleep(nextTaskPreserveTime_sec);
    }

    #ifdef STACK_DEBUG
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println("#### checkGoSleep stack : " + String(uxHighWaterMark) + "(byte) ####");
    writeMsgToPath(systemLogPath, "#### checkGoSleep stack : " + String(uxHighWaterMark) + "(byte) ####");
    #endif
  }
}

void checkTimeAndTask(void* pvParameters){
  Serial.println("|- checkTimeAndTask : created");
  writeMsgToPath(systemLogPath, "checkTimeAndTask : created");
  uint8_t aliveCounter = 0;

  #ifdef STACK_DEBUG
  UBaseType_t uxHighWaterMark;
  #endif
  
  // every 500ms for a task check 
  while(true){
    // jump out to other tasks
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // every 50 sec show led when running (500 ms * 100)
    if(aliveCounter >= 100){
      aliveLedShow();
      aliveCounter = 0;
    }

    // Debug msg
    #ifdef CHECK_IS_NEED_TO_RUN_TASK
      Serial.println("* checkTimeAndTask");
      writeMsgToPath(sensorDataPath, "* checkTimeAndTask");
    #endif

    // before task, check day change 
    // will update variables add create file / folder
    // in myScheduler.h
    checkDayChange();

    /*
      About task execute
      role 1 : only direct execute by the index "readTaskIndex"
        * trigger time is by "readTaskIndex"
        * we sleep exceed the real trigger time --> so we can direct use it
      role 2 : if is first boot, ignore role 1 until next task is coming

      min 
      | 0 | 1 | 2 | 3 | ... | 1439 |  (A day 1440 min, 0 min == 0 sec to 59sec)

      - [condition A] : 0 min 30 sec boot up, first boot
        1. findTheMatchedTaskIndex 
          * It will return task "TO BE RUN", see taskFileReader.h
          * | 0 | 1 | 2 | 3 |
              | |___| 
             now  |
                  | TO BE RUN 
        2. To sleep time = (Time of next task - now )
      - [condition B] : 1 min 00 sec wakeup (not first boot)
        1. direct run the task by "readTaskIndex" (the task "TO BE RUN")
        2. change the index
      
      About "readTaskIndex"
      * Rules : 
        * For execute : 
          * It is the index of which we need to run
          * We can direct use it
        * For save
          * When start-up
            * We need to find the latest(nearest) one in the future and give to it
          * When normal running 
            * We save the next index after the current one
      * example
        1. (only for start-up)know what time is it -> find the nearest one in the future -> save the variable
        2. sleep until a little be exceed of target time 
        3. direct run by the index
        4. save the next index 
    */

    

    // by index get the task(s), and resume the RTOS tasks
    // uint8NumToCharArr return a array(pointer) of task 
    uint8_t *tempTaskPointer = uint8NumToCharArr(taskScheduleList[readTaskIndex]);
    
    int taskSetTimeMin = readTaskIndex;
    // check if need to run task now 
    // if not cross day, change to sec and compare
    // if time is not actually exceed the schdule, go sleep again, else is keep running
    if( getPassedSecOfToday() > taskSetTimeMin * 60 && !isCrossDay){  
      //// print status and is need to run or not 
      #ifdef CHECK_IS_NEED_TO_RUN_TASK
        Serial.println("- Execute status of this loop : ");
        Serial.println("--- now : " + secMapTo24Hour(getPassedSecOfToday()));
        Serial.println("--- readTaskIndex : " + String(readTaskIndex) + "(" + minConvertTohour24(readTaskIndex) +")");
        Serial.println("--- tasks : " + uint8ToString(taskScheduleList[readTaskIndex]));
        Serial.println("--- execute : [True], now is exceed the set time.");

        writeMsgToPath(sensorDataPath, "- Execute status of this loop : ");
        writeMsgToPath(sensorDataPath, "--- now : " + secMapTo24Hour(getPassedSecOfToday()));
        writeMsgToPath(sensorDataPath, "--- readTaskIndex : " + String(readTaskIndex) + "(" + minConvertTohour24(readTaskIndex) +")");
        writeMsgToPath(sensorDataPath, "--- tasks : " + uint8ToString(taskScheduleList[readTaskIndex]));
        writeMsgToPath(sensorDataPath, "--- execute : [True], now is exceed the set time.");
      #endif

      //// if task bit exist 
      if (tempTaskPointer[TASK_A]){
        // add sleep lock, this will be release in every task
        isRunningTask += 1;
        // print and writr log 
        Serial.println("----- " + String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task A (sound record)." + " Set time : " + String(taskSetTimeMin) + "(" + String( minConvertTohour24(taskSetTimeMin) ) + ")");
        writeMsgToPath(systemLogPath, "----- Run Task A (sound record), set time : " + String(taskSetTimeMin) + "(" + String( minConvertTohour24(taskSetTimeMin) ) + ")");
        // set global variable 
        recordTime = recordSettingArray[readSettingIndex].duration_time; 
        channel_tag = recordSettingArray[readSettingIndex].channel;
        gain_ratio = recordSettingArray[readSettingIndex].multiple;
        // readSettingIndex add 
        //// it have auto re-zero when overflow
        readSettingIndex_moveToNext();
        // active the suspended task
        vTaskResume(tRecordSoundHandler);
      }
      if (tempTaskPointer[TASK_B]){
        // add sleep lock, this will be release in every task
        isRunningTask += 1;
        // print and writr log 
        Serial.println("----- " + String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task B (DHT)." + " Set time : " + String(taskSetTimeMin) + "(" + String( minConvertTohour24(taskSetTimeMin) ) + ")");
        DHT_TimeStamp = String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()));
        writeMsgToPath(systemLogPath, "----- Run Task B (DHT), set time : " + String(taskSetTimeMin) + "(" + String( minConvertTohour24(taskSetTimeMin) ) + ")", DHT_TimeStamp);  // write with same timestamp before/after get the data
        // active the suspended task 
        vTaskResume(tRecordDHTHandler);
      }
      if (tempTaskPointer[TASK_C]){
        // add sleep lock, this will be release in every task
        isRunningTask += 1;
        // print and writr log 
        Serial.println("----- " + String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task C (DS18B20)." + " Set time : " + String(taskSetTimeMin) + "(" + String( minConvertTohour24(taskSetTimeMin) ) + ")");
        DS18B20_TimeStamp = String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()));
        writeMsgToPath(systemLogPath, "----- Run Task C (DS18B20), set time : " + String(taskSetTimeMin) + "(" + String( minConvertTohour24(taskSetTimeMin) ) + ")", DS18B20_TimeStamp);
        // active the suspended task
        vTaskResume(tRecordDS18B20Handler);
      }
      if (tempTaskPointer[TASK_D]){
        // add sleep lock, this will be release in every task
        isRunningTask += 1;
        // print and writr log 
        Serial.println("----- " + String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task D (battery)." + " Set time : " + String(taskSetTimeMin) + "(" + String( minConvertTohour24(taskSetTimeMin) ) + ")");
        Battery_TimeStamp = String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()));
        writeMsgToPath(systemLogPath, "----- Run Task D (battery), set time : " + String(taskSetTimeMin) + "(" + String( minConvertTohour24(taskSetTimeMin) ) + ")", Battery_TimeStamp);
        // active the suspended task
        vTaskResume(tRecordBatteryHandler);
      }
      
      // clear the ram after use
      free(tempTaskPointer);

      // swift to next task until have task in that index and check it is cross day or not
      //// it have auto re-zero when overflow
      readTaskIndex_moveToNext();
      //// lock the task, it will be release after chackDayChange()
      if(readTaskIndex == 0){
        isCrossDay = true;
      }
    }else{
      //// print status and is need to run or not 
      #ifdef CHECK_IS_NEED_TO_RUN_TASK
        Serial.println("- Execute status of this loop : ");
        Serial.println("--- now : " + secMapTo24Hour(getPassedSecOfToday()));
        Serial.println("--- readTaskIndex : " + String(readTaskIndex) + "(" + minConvertTohour24(readTaskIndex) +")");
        Serial.println("--- tasks : " + uint8ToString(taskScheduleList[readTaskIndex]));
        Serial.println("--- execute : [False], now is before the set time.");

        writeMsgToPath(sensorDataPath, "- Execute status of this loop : ");
        writeMsgToPath(sensorDataPath, "--- now : " + secMapTo24Hour(getPassedSecOfToday()));
        writeMsgToPath(sensorDataPath, "--- readTaskIndex : " + String(readTaskIndex) + "(" + minConvertTohour24(readTaskIndex) +")");
        writeMsgToPath(sensorDataPath, "--- tasks : " + uint8ToString(taskScheduleList[readTaskIndex]));
        writeMsgToPath(sensorDataPath, "--- execute : [False], now is before the set time.");
      #endif
    }

    //// save sleep time to global variable
    if(isCrossDay){
      // change day
      Serial.println("----- This sleep will cross day");
      nextTaskPreserveTime_sec = SECONDS_OF_A_DAY - getPassedSecOfToday() + taskSetTimeMin * 60;
    }else{
      // normal 
      nextTaskPreserveTime_sec = taskSetTimeMin * 60 - getPassedSecOfToday();
    }
    Serial.println("----- sleepingTimeUntilNextTask_sec : " + String(nextTaskPreserveTime_sec) + "\n");

    // active the suspended sleep task
    vTaskResume(tCheckGoSleepHandler);
    aliveCounter += 1;

    #ifdef STACK_DEBUG
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println("#### checkTimeAndTask stack : " + String(uxHighWaterMark) + "(byte) ####");
    writeMsgToPath(systemLogPath, "#### checkTimeAndTask stack : " + String(uxHighWaterMark) + "(byte) ####");
    #endif
  }
}

// RTOS task of transmit record data
void transmitSoundDataToSD(void* pvParameters){
  Serial.println("|- transmitSoundDataToSD : created");
  writeMsgToPath(systemLogPath, "transmitSoundDataToSD : created");
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  
  // SD setting 
  myFileFormat soundFile;

  #ifdef STACK_DEBUG
  UBaseType_t uxHighWaterMark;
  #endif
  
  // task part 
  while (true) {
    // jump out to other tasks
    // vTaskDelay(500 / portTICK_PERIOD_MS);
    // wait RTOS signal about buffer full for 100 ms
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    // if have signal 
    if(ulNotificationValue > 0){
      // lock SD and write
      if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
        if (!soundFile.open(_filenameWithPath, O_WRONLY | O_CREAT | O_APPEND )) {     // open need char array, not string. So use c_str to convert
          Serial.println(" --> open file failed, transmitToSD");
          continue;
        }
        soundFile.write((uint8_t*)transmitAudioBuffer, globalSDBufferByteSize);
        soundFile.close(); 
      }xSemaphoreGive( xSemaphore_SD );
    }

    #ifdef STACK_DEBUG
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println("#### transmitSoundDataToSD stack : " + String(uxHighWaterMark) + "(byte) ####");
    writeMsgToPath(systemLogPath, "#### transmitSoundDataToSD stack : " + String(uxHighWaterMark) + "(byte) ####");
    #endif
  }
}


/* Task A*/
// RTOS task of recordSound
void recordSound(void* pvParameters){
  Serial.println("|- recordSound : created");
  writeMsgToPath(systemLogPath, "recordSound : created");
  
  #ifdef STACK_DEBUG
  UBaseType_t uxHighWaterMark;
  #endif
  
  while(true){
    // block itself first, untill we resume it
    #ifdef RTOS_DETIAL
      Serial.println("recordSound : suspended");
    #endif
    vTaskSuspend(NULL);

    // calculate filename
    #ifdef RTOS_DETIAL
      Serial.println("recordSound : resumed");
    #endif
    String recordPath = "/" + String(today) + "/" + String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()) + "_" + String(channel_tag) + ".wav");
    char filename[40];
    strcpy(filename, recordPath.c_str());

    #ifdef RECORD_SOUND_DEBUG
      Serial.println(recordPath);
    #endif

    // recognize channel tag
    if(channel_tag == 'B'){
      startTheRecord(recordTime, filename, gain_ratio, CHANNEL_STEREO);
    }else if(channel_tag == 'L'){
      startTheRecord(recordTime, filename, gain_ratio, CHANNEL_LEFT);
    }else if(channel_tag == 'R'){
      startTheRecord(recordTime, filename, gain_ratio, CHANNEL_RIGHT);
    }

    #ifdef STACK_DEBUG
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println("#### recordSound stack : " + String(uxHighWaterMark) + "(byte) ####");
    writeMsgToPath(systemLogPath, "#### recordSound stack : " + String(uxHighWaterMark) + "(byte) ####");
    #endif
  }
}


/* Task B*/
// RTOS task of recordDHT
void recordDHT(void* pvParameters){
  Serial.println("|- recordDHT : created");
  writeMsgToPath(systemLogPath, "recordDHT : created");
  
  #ifdef STACK_DEBUG
  UBaseType_t uxHighWaterMark;
  #endif
  
  while(true){
    // block itself first, untill we resume it
    #ifdef RTOS_DETIAL
      Serial.println("recordDHT : suspended");
    #endif
    vTaskSuspend(NULL);
  
    // if it was resumed by someone
    // try to power on the sensor
    #ifdef RTOS_DETIAL
      Serial.println("recordDHT : resumed");
    #endif
    while(true){
      vTaskDelay(100 / portTICK_PERIOD_MS);
      // if got the power of control 
      if ( xSemaphoreTake( xSemaphore_Temp_PMOS, pdMS_TO_TICKS(100) ) == pdTRUE) {
        turnOnDhtPower();
        // use delay to wait it fully powered 
        vTaskDelay(PMOS_CHARGING_TIME / portTICK_PERIOD_MS);
        break;
      }
    }
    
    // get data of temperature
    vTaskDelay(100 / portTICK_PERIOD_MS);
    float temperature = DHT_get_temperature();
    #ifdef DHT_GET_TEMPERATURE_DEBUG
      Serial.println("[DHT temperature : " + String(temperature) + " C]");
    #endif

    // get data of humidity
    vTaskDelay(100 / portTICK_PERIOD_MS);
    float humidity = DHT_get_Humidity();
    #ifdef DHT_GET_HUMIDITY_DEBUG
      Serial.println("[DHT Humidity : " + String(humidity) + " %]");
    #endif

    // finished, release power switch 
    xSemaphoreGive( xSemaphore_Temp_PMOS );

    // write SD 
    writeMsgToPath(sensorDataPath, "DHT temperature : " + String(temperature) + " C", DHT_TimeStamp);
    writeMsgToPath(sensorDataPath, "DHT Humidity : " + String(humidity) + " %", DHT_TimeStamp);

    // release sleep lock and delete itself
    isRunningTask -= 1;

    #ifdef STACK_DEBUG
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println("#### recordDHT stack : " + String(uxHighWaterMark) + "(byte) ####");
    writeMsgToPath(systemLogPath, "#### recordDHT stack : " + String(uxHighWaterMark) + "(byte) ####");
    #endif
  }
}


/* Task C*/
// RTOS task of recordDS18B20
void recordDS18B20(void* pvParameters){
  Serial.println("|- recordDS18B20 : created");
  writeMsgToPath(systemLogPath, "recordDS18B20 : created");
  
  #ifdef STACK_DEBUG
  UBaseType_t uxHighWaterMark;
  #endif
  
  while(true){
    // block itself first, untill we resume it
    #ifdef RTOS_DETIAL
      Serial.println("recordDS18B20 : suspended");
    #endif
    vTaskSuspend(NULL);

    // if it was resumed by someone
    // try to power on the sensor
    #ifdef RTOS_DETIAL
      Serial.println("recordDS18B20 : resumed");
    #endif
    while(true){
      vTaskDelay(100 / portTICK_PERIOD_MS);
      if( xSemaphoreTake( xSemaphore_Temp_PMOS, pdMS_TO_TICKS(100) ) == pdTRUE){
        turnOnDs18b20Power();
        // use delay to wait it fully powered 
        vTaskDelay(PMOS_CHARGING_TIME / portTICK_PERIOD_MS);
        break;
      }
    }

    // get data of temperature
    vTaskDelay(100 / portTICK_PERIOD_MS);
    float temperature = getDS18B20Temp();
    #ifdef GET_DS18B20_TEMP_DEBUG
      Serial.println("[DS18B20 : " + String(temperature) + " C]");
    #endif

    // finished, release power switch 
    xSemaphoreGive( xSemaphore_Temp_PMOS );

    // write SD 
    writeMsgToPath(sensorDataPath, "DS18B20 : " + String(temperature) + " C", DS18B20_TimeStamp);
    
    // release sleep lock 
    isRunningTask -= 1;

    #ifdef STACK_DEBUG
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println("#### recordDS18B20 stack : " + String(uxHighWaterMark) + "(byte) ####");
    writeMsgToPath(systemLogPath, "#### recordDS18B20 stack : " + String(uxHighWaterMark) + "(byte) ####");
    #endif
  }
}


/* Task D*/
// RTOS task of recordBattery
void recordBattery(void* pvParameters){
  Serial.println("|- recordBattery : created");
  writeMsgToPath(systemLogPath, "recordBattery : created");
  
  #ifdef STACK_DEBUG
  UBaseType_t uxHighWaterMark;
  #endif
  
  while(true){
    // block itself first, untill we resume it
    #ifdef RTOS_DETIAL
      Serial.println("recordBattery : suspended");
    #endif
    vTaskSuspend(NULL);

    
    // if it was resumed by someone
    // read part 
    #ifdef RTOS_DETIAL
      Serial.println("recordBattery : resumed");
    #endif
    #ifdef RECORD_BATTERY_DEBUG
      Serial.println("[Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ " %)]");
    #endif
    writeMsgToPath(sensorDataPath, "Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)", Battery_TimeStamp);

    isRunningTask -= 1;

    #ifdef STACK_DEBUG
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println("#### recordBattery stack : " + String(uxHighWaterMark) + "(byte) ####");
    writeMsgToPath(systemLogPath, "#### recordBattery stack : " + String(uxHighWaterMark) + "(byte) ####");
    #endif
  }
}


// A function to check day change. If happened, create new folder and new log file
// Not a full RTOS task, we only use vTaskDelay to prevent blocking
void checkDayChange(){

  #ifdef CHECK_DAY_CHANGE_DEBUG
    Serial.println("check day change");
  #endif

  // confirmed that the day is changed
  // local time crossed day 
  if( getPassedSecOfToday() > SECONDS_OF_A_DAY){  
    turnOnRtcPower();
    // use delay to wait it fully powered 
    vTaskDelay(PMOS_CHARGING_TIME / portTICK_PERIOD_MS);

    /* calibrate local time with RTC timer */
    // a day is 1440min == 86400sec
    uint32_t tempRtcTime = GetHowManySecondsHasPassedTodayFromRtc();
    
    // if local time && RTC time both cross day (normal situation) or RTC cross day but local not
    // 1hr tolerance range of rtc time
    if(tempRtcTime < 3600){
      sys_RTC_time_offset = tempRtcTime;
      sys_millis_time_offset = millis();   // notice : millis only can count 49 days
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

      // if task list lock, release task lock
      if(isTaskAllLock){
        isTaskAllLock = false;
        Serial.println("Unlock the task list.");
        writeMsgToPath(systemLogPath, "Unlock the task list.");
      }

      // not first time check == already have boot check
      // boot check at findTheMatchedArrayReadIndex()
      if(!isFirstCheckEvaluation){
        checkEvaluation();
      }

      // release the task lock
      isCrossDay = false;
    }
    // if local time cross day but RTC time not 
    else{
      // do nothing, system will go to sleep again automatically 
    }
  }
}



// Not a full RTOS task, we only use vTaskDelay to prevent blocking
#if defined( ARDUINO_ARCH_ESP32 )  
  void goToSleep(int sleepTime_sec) {
    // have X sec to sleep
    int canSleepMs =  sleepTime_sec * 1000;  // Serial.println("canSleepMs : " + String(canSleepMs));
    if(canSleepMs > 100){
      #ifdef GOTOSLEEP_DEBUG
        #ifdef FEED_DOG_DEBUG
          Serial.println(String(secMapTo24Hour(getPassedSecOfToday())));
        #endif
      #endif
      Serial.println("--- Go to sleep for : " + String(canSleepMs/1000.0f) + " sec");
      writeMsgToPath(systemLogPath, "Go to sleep for : " + String(canSleepMs/1000.0f) + " sec");

      int oneMinTimes = canSleepMs / (1000 * 60);
      int remainMs = canSleepMs % (1000 * 60);

      // loop sleep part of long time
      for(int i=0; i<oneMinTimes; i++){

        // lock if write sd not finished
        if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
          esp_sleep_enable_timer_wakeup(60 * 1000 * ms_TO_uS_FACTOR);
          #ifdef USE_DEEP_SLEEP
            esp_deep_sleep_start();
          #else
            esp_light_sleep_start();
          #endif
        }xSemaphoreGive( xSemaphore_SD );

        // debug MSG part
        #ifdef GOTOSLEEP_DEBUG
          #ifdef FEED_DOG_DEBUG
            Serial.println(String(secMapTo24Hour(getPassedSecOfToday())));
          #endif
          Serial.println("Have slept for : " + String( (i+1) ) + " min. Wake up to feed dog.");
        #endif

        // feed dog
        vTaskDelay(40 / portTICK_PERIOD_MS);
        aliveLedShow();
      }

      // short sleep part 
      #ifdef GOTOSLEEP_DEBUG
        #ifdef FEED_DOG_DEBUG
          Serial.println(String(secMapTo24Hour(getPassedSecOfToday())));
        #endif
        Serial.println("--- Sleep for remain " + String(remainMs/1000.0f) + " seconds\n");
      #endif
      vTaskDelay(40 / portTICK_PERIOD_MS);

      // lock if write sd not finished
      if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
        esp_sleep_enable_timer_wakeup(remainMs * ms_TO_uS_FACTOR);
        #ifdef USE_DEEP_SLEEP
          esp_deep_sleep_start();
        #else
          esp_light_sleep_start();
        #endif
      }xSemaphoreGive( xSemaphore_SD );

      // wake feed dog
      vTaskDelay(40 / portTICK_PERIOD_MS);

      // every wakeup print a log
      getWakeupReason();
    }
    // After sleep finished, wait long enough to check task
    vTaskDelay(500 / portTICK_PERIOD_MS);
    // then release sleep lock
    previousRoundOfSleepFinished = true;
  }
#else
  void SleepMethod( unsigned long aDuration ) {
  }
#endif




#endif


/* RTOS note*/

// prefix
// vXXXXX --> return void
// xXXXXX --> return other type (like struct)

// most often used
// xTaskCreatePinnedToCore()
// vTaskDelete()
// vTaskDelay()

// URL 
// https://wiki.csie.ncku.edu.tw/embedded/freertos



