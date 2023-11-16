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
TaskHandle_t tINMP;
TaskHandle_t tTaskScheduler;




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


// RTOS task of transmit record data
void transmitSoundDataToSD(void* pvParameters){
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  while (true) {
    // feed dog of each loop 
    vTaskDelay(10);
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

void transmitSimpleDataToSD(void* pvParameters){

}



// RTOS task of DHT sensor
void recordDHT(){
  // try to power on the sensor
  while(true){
    vTaskDelay(10);
    // if got the power of control 
    if ( xSemaphoreTake( xSemaphore_Temp_PMOS, pdMS_TO_TICKS(100) ) == pdTRUE) {
      turnOnDhtPower();
      break;
    }
  }
  
  // get data of temperature
  vTaskDelay(10);
  float temperature = DHT_get_temperature();
  #ifdef DHT_GET_TEMPERATURE_DEBUG
    Serial.println("DHT temperature : " + String(temperature) + " C");
  #endif

  // get data of humidity
  vTaskDelay(10);
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
  vTaskDelete(NULL);
}




void recordBattery(){
  vTaskDelay(10);

  // read part 
  #ifdef RECORD_BATTERY_DEBUG
    Serial.println("Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ " %)");
  #endif
  writeMsgToPath(sensorDataPath, "Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)", Battery_TimeStamp);

  isRunningTask -= 1;
}


void recordDS18B20(){
  // try to power on the sensor
  while(true){
    vTaskDelay(10);
    if( xSemaphoreTake( xSemaphore_Temp_PMOS, pdMS_TO_TICKS(100) ) == pdTRUE){
      DS18B20_PowerRetry = false;
      turnOnDs18b20Power();
      break;
    }
  }

  // get data of temperature
  vTaskDelay(10);
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
