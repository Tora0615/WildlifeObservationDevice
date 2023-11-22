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

// include guard
#ifndef MYSCHEDULER_H
#define MYSCHEDULER_H

// variables 
float recordTime;
char channel_tag;
float gain_ratio;
String DHT_TimeStamp;
String DS18B20_TimeStamp;
String Battery_TimeStamp;
bool previousRoundOfSleepFinished;
#define ms_TO_uS_FACTOR 1000ULL
void checkGoSleep(void* pvParameters);
void checkTimeAndTask(void* pvParameters);
void transmitSoundDataToSD(void* pvParameters);
void recordSound(void* pvParameters);
void recordDHT(void* pvParameters);
void recordDS18B20(void* pvParameters);
void recordBattery(void* pvParameters);
void checkDayChange();
void goToSleep(int sleepTime_sec);

// RTOS setting
#define configUSE_TIME_SLICING 1
#define configUSE_PREEMPTION 1

/* RTOS handler */
TaskHandle_t tCheckGoSleepHandler;
TaskHandle_t tCheckTimeAndTaskHandler;
  // TaskHandle_t tSdTransmitHandler;  --> at myINMP file
  TaskHandle_t tRecordSoundHandler;
  TaskHandle_t tRecordDHTHandler;
  TaskHandle_t tRecordDS18B20Handler;
  TaskHandle_t tRecordBatteryHandler;

void createRTOSTasks() {
  // hide evaluation check here
  checkEvaluation();
  Serial.println("RTOS : createCoreTasks");

  xTaskCreatePinnedToCore(
    checkGoSleep,                           /* Task function. */
    "checkGoSleep",                         /* name of task. */
    8192,                                  /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &tCheckGoSleepHandler,                       /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );

  xTaskCreatePinnedToCore(
    checkTimeAndTask,                           /* Task function. */
    "checkTimeAndTask",                         /* name of task. */
    8192,                                  /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &tCheckTimeAndTaskHandler,                       /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );


    xTaskCreatePinnedToCore(
      transmitSoundDataToSD,                           /* Task function. */
      "transmitSoundDataToSD",                         /* name of task. */
      8192,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      4,                                      /* priority of the task */
      &tSdTransmitHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordSound,                           /* Task function. */
      "recordSound",                         /* name of task. */
      20480,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      5,                                      /* priority of the task */
      &tRecordSoundHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordDHT,                           /* Task function. */
      "recordDHT",                         /* name of task. */
      8192,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      3,                                      /* priority of the task */
      &tRecordDHTHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordDS18B20,                           /* Task function. */
      "recordDS18B20",                         /* name of task. */
      8192,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      3,                                      /* priority of the task */
      &tRecordDS18B20Handler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordBattery,                           /* Task function. */
      "recordBattery",                         /* name of task. */
      8192,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      3,                                      /* priority of the task */
      &tRecordBatteryHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );
  // OTHER_TASK_CPU
  // INMP_CPU
}




void checkGoSleep(void* pvParameters){
  previousRoundOfSleepFinished = true;
  Serial.println("checkGoSleep : created");
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
    Serial.print("isRunningTask : " + String(isRunningTask)); Serial.println(", previousRoundOfSleepFinished : " + String(previousRoundOfSleepFinished));
    if(isRunningTask == 0 && previousRoundOfSleepFinished){
      previousRoundOfSleepFinished = false;
      goToSleep(nextTaskPreserveTime_sec);
    }
  }
}



void checkTimeAndTask(void* pvParameters){
  Serial.println("checkTimeAndTask : created");
  while(true){
    // jump out to other tasks
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // Debug msg
    #ifdef CHECK_IS_NEED_TO_RUN_TASK
      Serial.println("checkTimeAndTask");
    #endif

    // before task, check day change 
    // will update variables add create file / folder
    checkDayChange();

    // get next task's start time
    int startTimeOfNext = 0;
    if((taskArray + arrayReadIndex)->setType == 0){  // simple task
      startTimeOfNext = (taskArray + arrayReadIndex)->taskType.simple.start_min_of_a_day;
    }else{
      startTimeOfNext = (taskArray + arrayReadIndex)->taskType.complex.start_min_of_a_day;
    }

    // check if need to run task now ?
    if( getPassedSecOfToday() > startTimeOfNext * 60 && !isCrossDay){  // if not cross day, change to sec and compare

      #ifdef CHECK_IS_NEED_TO_RUN_TASK
        Serial.println("Execute contition of this loop : ");
        Serial.println("arrayReadIndex : " + String(arrayReadIndex));
        Serial.println("arrayUsedIndex : " + String(arrayUsedIndex));
      #endif

      // add sleep lock, this will be release in every task
      isRunningTask += 1;

      // get task code 
      char task_code;
      if((taskArray + arrayReadIndex)->setType == 0){  // simple task
        task_code = (taskArray + arrayReadIndex)->taskType.simple.task;
      }else{
        task_code = (taskArray + arrayReadIndex)->taskType.complex.task;
      }

      // start the task according to task code. 
      if (task_code == 'A'){          // Sound record
        // print and writr log 
        Serial.println(String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task A (sound record)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
        writeMsgToPath(systemLogPath, "Run Task A (sound record), set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
        // set global variable 
        recordTime = (taskArray + arrayReadIndex)->taskType.complex.time;
        channel_tag = (taskArray + arrayReadIndex)->taskType.complex.channel;
        gain_ratio = (taskArray + arrayReadIndex)->taskType.complex.multiple;
        // active the suspended task
        vTaskResume(tRecordSoundHandler);
      }else if (task_code == 'B'){    // DHT
        // print and writr log 
        Serial.println( String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task B (DHT)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
        DHT_TimeStamp = String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()));
        writeMsgToPath(systemLogPath, "Run Task B (DHT), set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")", DHT_TimeStamp);  // write with same timestamp before/after get the data
        // active the suspended task 
        vTaskResume(tRecordDHTHandler);
      }else if (task_code == 'C'){    // DS18B20
        // print and writr log 
        Serial.println(String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task C (DS18B20)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
        DS18B20_TimeStamp = String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()));
        writeMsgToPath(systemLogPath, "Run Task C (DS18B20), set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")", DS18B20_TimeStamp);
        // active the suspended task
        vTaskResume(tRecordDS18B20Handler);
      }else if (task_code == 'D'){    // battery 
        // print and writr log 
        Serial.println(String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + " : run Task D (battery)." + " Set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")");
        Battery_TimeStamp = String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday()));
        writeMsgToPath(systemLogPath, "Run Task D (battery), set time : " + String(startTimeOfNext) + "(" + String( minConvertTohour24(startTimeOfNext) ) + ")", Battery_TimeStamp);
        // active the suspended task
        vTaskResume(tRecordBatteryHandler);
      }

      // swift to next task and check it is cross day or not
      arrayReadIndex +=1;
      if (arrayReadIndex == arrayUsedIndex){
        Serial.println("checkTimeAndTask : Task array out of range, it is cross-day task! Re-zero index now.");
        writeMsgToPath(systemLogPath, "checkTimeAndTask : Task array out of range, it is cross-day task! Re-zero index now.");
        arrayReadIndex = 0;
        // lock the task
        isCrossDay = true;
      }
    }


    // save sleep time to global variable
    if(isCrossDay){
      // change day
      Serial.println("checkTimeAndTask -- This sleep will cross day");
      nextTaskPreserveTime_sec = SECONDS_OF_A_DAY - getPassedSecOfToday() + startTimeOfNext * 60;
    }else{
      // normal 
      nextTaskPreserveTime_sec = startTimeOfNext * 60 - getPassedSecOfToday();
    }
    Serial.println("checkTimeAndTask -- nextTaskPreserveTime_sec : " + String(nextTaskPreserveTime_sec));
    char task_code;
    if((taskArray + arrayReadIndex)->setType == 0){  // simple task
      task_code = (taskArray + arrayReadIndex)->taskType.simple.task;
    }else{
      task_code = (taskArray + arrayReadIndex)->taskType.complex.task;
    }
    Serial.println("checkTimeAndTask -- nextTask : " + String(task_code));

    // active the suspended sleep task
    vTaskResume(tCheckGoSleepHandler);

  }
}

// RTOS task of transmit record data
void transmitSoundDataToSD(void* pvParameters){
  Serial.println("transmitSoundDataToSD : created");
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  // SD setting 
  #ifdef SD_USE_NORMAL
    FsFile soundFile;
  #else
    ExFile soundFile;
  #endif
  // task part 
  while (true) {
    // jump out to other tasks
    vTaskDelay(500 / portTICK_PERIOD_MS);
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
  }
}


/* Task A*/
// RTOS task of recordSound
void recordSound(void* pvParameters){
  Serial.println("recordSound : created");
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
      recordWithDualChannel(recordTime, filename, gain_ratio);
    }else if(channel_tag == 'L'){
      recordWithMonoChannel(recordTime, filename, gain_ratio, CHANNEL_LEFT);
    }else if(channel_tag == 'R'){
      recordWithMonoChannel(recordTime, filename, gain_ratio, CHANNEL_RIGHT);
    }
  }
}


/* Task B*/
// RTOS task of recordDHT
void recordDHT(void* pvParameters){
  Serial.println("recordDHT : created");
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
        vTaskDelay(500 / portTICK_PERIOD_MS);
        break;
      }
    }
    
    // get data of temperature
    vTaskDelay(100 / portTICK_PERIOD_MS);
    float temperature = DHT_get_temperature();
    #ifdef DHT_GET_TEMPERATURE_DEBUG
      Serial.println("DHT temperature : " + String(temperature) + " C");
    #endif

    // get data of humidity
    vTaskDelay(100 / portTICK_PERIOD_MS);
    float humidity = DHT_get_Humidity();
    #ifdef DHT_GET_HUMIDITY_DEBUG
      Serial.println("DHT Humidity : " + String(humidity) + " %");
    #endif

    // finished, release power switch 
    xSemaphoreGive( xSemaphore_Temp_PMOS );

    // write SD 
    writeMsgToPath(sensorDataPath, "DHT temperature : " + String(temperature) + " C", DHT_TimeStamp);
    writeMsgToPath(sensorDataPath, "DHT Humidity : " + String(humidity) + " %", DHT_TimeStamp);

    // release sleep lock and delete itself
    isRunningTask -= 1;
  }
}


/* Task C*/
// RTOS task of recordDS18B20
void recordDS18B20(void* pvParameters){
  Serial.println("recordDS18B20 : created");
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
        vTaskDelay(500 / portTICK_PERIOD_MS);
        break;
      }
    }

    // get data of temperature
    vTaskDelay(100 / portTICK_PERIOD_MS);
    float temperature = getDS18B20Temp();
    #ifdef GET_DS18B20_TEMP_DEBUG
      Serial.println("DS18B20 : " + String(temperature) + " C");
    #endif

    // finished, release power switch 
    xSemaphoreGive( xSemaphore_Temp_PMOS );

    // write SD 
    writeMsgToPath(sensorDataPath, "DS18B20 : " + String(temperature) + " C", DS18B20_TimeStamp);
    
    // release sleep lock 
    isRunningTask -= 1;
  }
}


/* Task D*/
// RTOS task of recordBattery
void recordBattery(void* pvParameters){
  Serial.println("recordBattery : created");
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
      Serial.println("Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ " %)");
    #endif
    writeMsgToPath(sensorDataPath, "Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)", Battery_TimeStamp);

    isRunningTask -= 1;
  }
}


// A function to check day change. If happened, create new folder and new log file
// Not a full RTOS task, we only use vTaskDelay to prevent blocking
void checkDayChange(){

  #ifdef CHECK_DAY_CHANGE_DEBUG
    Serial.println("check day change");
  #endif

  // confirmed that the day is changed
  if( getPassedSecOfToday() > SECONDS_OF_A_DAY){  
    turnOnRtcPower();
    // use delay to wait it fully powered 
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // if > a day, calibrate with RTC timer
    sys_RTC_time_offset = GetHowManySecondsHasPassedTodayFromRtc();
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
}



// Not a full RTOS task, we only use vTaskDelay to prevent blocking
#if defined( ARDUINO_ARCH_ESP32 )  
  void goToSleep(int sleepTime_sec) {
    // have X sec to sleep
    int canSleepMs =  sleepTime_sec  * 1000;  // Serial.println("canSleepMs : " + String(canSleepMs));
    if(canSleepMs > 100){
      #ifdef GOTOSLEEP_DEBUG
        #ifdef FEED_DOG_DEBUG
          Serial.println(String(secMapTo24Hour(getPassedSecOfToday())));
        #endif
      #endif
      Serial.println("Go to sleep for : " + String(canSleepMs/1000.0f) + " sec");
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

        // wakeup to feed dog
        vTaskDelay(40 / portTICK_PERIOD_MS);

        // debug MSG part
        #ifdef GOTOSLEEP_DEBUG
          #ifdef FEED_DOG_DEBUG
            Serial.println(String(secMapTo24Hour(getPassedSecOfToday())));
          #endif
          Serial.println("Have slept for : " + String( (i+1) ) + " min. Wake up to feed dog.");
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
