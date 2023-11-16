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
#define ms_TO_uS_FACTOR 1000ULL

// RTOS setting
#define configUSE_TIME_SLICING 1
#define configUSE_PREEMPTION 1

/* RTOS handler */
TaskHandle_t tCheckTimeAndTaskHandler;
  TaskHandle_t tSdTransmitHandler;
  TaskHandle_t tRecordSoundHandler;
  TaskHandle_t tRecordDHTHandler;
  TaskHandle_t tRecordDS18B20Handler;
  TaskHandle_t tRecordBatteryHandler;


void createRTOSTasks() {
  // hide evaluation check here
  checkEvaluation();
  Serial.println("RTOS : createCoreTasks");

  xTaskCreatePinnedToCore(
    checkTimeAndTask,                           /* Task function. */
    "checkTimeAndTask",                         /* name of task. */
    4096,                                  /* Stack size of task */
    NULL,                                   /* parameter of the task */
    2,                                      /* priority of the task */
    &tCheckTimeAndTaskHandler,                       /* task handle */
    OTHER_TASK_CPU                          /* CPU core */
  );


    xTaskCreatePinnedToCore(
      transmitSoundDataToSD,                           /* Task function. */
      "transmitSoundDataToSD",                         /* name of task. */
      4096,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      2,                                      /* priority of the task */
      &tSdTransmitHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordSound,                           /* Task function. */
      "recordSound",                         /* name of task. */
      4096,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      2,                                      /* priority of the task */
      &tTransmitHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordDHT,                           /* Task function. */
      "recordDHT",                         /* name of task. */
      4096,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      2,                                      /* priority of the task */
      &tTransmitHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordDS18B20,                           /* Task function. */
      "recordDS18B20",                         /* name of task. */
      4096,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      2,                                      /* priority of the task */
      &tTransmitHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

    xTaskCreatePinnedToCore(
      recordBattery,                           /* Task function. */
      "recordBattery",                         /* name of task. */
      4096,                                  /* Stack size of task */
      NULL,                                   /* parameter of the task */
      2,                                      /* priority of the task */
      &tTransmitHandler,                       /* task handle */
      OTHER_TASK_CPU                          /* CPU core */
    );

}





void checkTimeAndTask(){
  while(true){
    // jump out to other tasks
    vTaskDelay(500);

    //before task, check day change 
    checkDayChange();





  }
}

// RTOS task of transmit record data
void transmitSoundDataToSD(void* pvParameters){
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  while (true) {
    // jump out to other tasks
    vTaskDelay(500);
    // wait RTOS signal about buffer full for 100 ms
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    // if have signal 
    if(ulNotificationValue > 0){
      // lock SD and write
      if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
        // SD setting 
        #ifdef SD_USE_NORMAL
          FsFile soundFile;
        #else
          ExFile soundFile;
        #endif
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
  while(true){
    // block itself first, untill we resume it
    vTaskSuspend(NULL);
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
  }
}


/* Task B*/
// RTOS task of recordDHT
void recordDHT(void* pvParameters){
  while(true){
    // block itself first, untill we resume it
    vTaskSuspend(NULL);
  
    // if it was resumed by someone
    // try to power on the sensor
    while(true){
      vTaskDelay(500);
      // if got the power of control 
      if ( xSemaphoreTake( xSemaphore_Temp_PMOS, pdMS_TO_TICKS(100) ) == pdTRUE) {
        turnOnDhtPower();
        // use delay to wait it fully powered 
        vTaskDelay(500 / portTICK_PERIOD_MS);
        break;
      }
    }
    
    // get data of temperature
    vTaskDelay(500);
    float temperature = DHT_get_temperature();
    #ifdef DHT_GET_TEMPERATURE_DEBUG
      Serial.println("DHT temperature : " + String(temperature) + " C");
    #endif

    // get data of humidity
    vTaskDelay(500);
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
  while(true){
    // block itself first, untill we resume it
    vTaskSuspend(NULL);

    // if it was resumed by someone
    // try to power on the sensor
    while(true){
      vTaskDelay(500);
      if( xSemaphoreTake( xSemaphore_Temp_PMOS, pdMS_TO_TICKS(100) ) == pdTRUE){
        DS18B20_PowerRetry = false;
        turnOnDs18b20Power();
        // use delay to wait it fully powered 
        vTaskDelay(500 / portTICK_PERIOD_MS);
        break;
      }
    }

    // get data of temperature
    vTaskDelay(500);
    float temperature = getDS18B20Temp();
    #ifdef GET_DS18B20_TEMP_DEBUG
      Serial.println("DS18B20 : " + String(temperature) + " C");
    #endif

    // finished, release power switch 
    xSemaphoreGive( xSemaphore_Temp_PMOS );

    // write SD 
    writeMsgToPath(sensorDataPath, "DS18B20 : " + String(temperature) + " C", DS18B20_TimeStamp);
    
    // release sleep lock and delete task
    isRunningTask -= 1;
    vTaskDelete(NULL);
  }
}


/* Task D*/
// RTOS task of recordBattery
void recordBattery(void* pvParameters){
  while(true){
    // block itself first, untill we resume it
    vTaskSuspend(NULL);

    // if it was resumed by someone
    // read part 
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

  if( getPassedSecOfToday() > 86400){  // a day : 86400 sec
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
  }
}





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
