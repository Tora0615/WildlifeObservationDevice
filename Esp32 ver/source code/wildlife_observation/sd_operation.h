/*---- include guard ----*/
#ifndef SD_OPERATION_H
#define SD_OPERATION_H
/*
This file should only have basic SD oprate functions
*/

/*---- macro or define ----*/

// #define SD_WRITE_MSG_DEBUG

/*---- official lib ----*/ 
#include <Update.h>
#include <SPI.h>
#include <SdFat.h>
#include <sdios.h>

/*---- other involve lib  ----*/
#include "rtc_timer.h"  // include setting 

/*---- classes, variables or function define  ----*/
const int8_t DISABLE_CHIP_SELECT = -1;


/*-------- function implement --------*/
#ifdef SD_USE_FAT16_32
  // FAT16/FAT32 (SD_FAT_TYPE 1)
  SdFat32 sd;
  typedef File32 myFileFormat;
#endif 

#ifdef SD_EXFAT
  // exFAT (SD_FAT_TYPE 2)
  SdExFat sd;
  typedef ExFile myFileFormat;
#endif 

#ifdef SD_HYBRID_FAT1632_EXFAT
  // FAT16/FAT32 and exFAT (SD_FAT_TYPE 3)
  SdFs sd;
  typedef FsFile myFileFormat;
#endif 

void SDInit(){
  // Power part (16 Pin)
  pinMode(SD_PMOS, OUTPUT);
  #ifdef SD_WRITE_MSG_DEBUG
    Serial.println("Direct turn on SD POWER");
  #endif 
  digitalWrite(SD_PMOS, POWER_ON);   // Turn off. GPIO default is low ->  will let mic ON
  delay(1000); // give enough time for PMOS power up
  // TODO : turn off when sleep

  if(!sd.begin(CHIP_SELECT, SPI_SPEED)){
    Serial.println("SD card init error : NO SD card");
    while(1){
      delay(10000);

      if (noSdRetry < NO_SD_RETRY_LIMIT){
        // reboot, 
      // by using deep sleep and wake
      // this can remember something in RTC memory
      Serial.println("Trigger sleep (Fake no SD reboot), retry : " + String(noSdRetry));
      noSdRetry +=1;
      delay(2); // for print fully
      esp_sleep_enable_timer_wakeup(1 * 1000 * ms_TO_uS_FACTOR);  // 1 sec
      esp_deep_sleep_start();
      }else{
        // sleep forever
        Serial.println("sleep forever");
        esp_deep_sleep_start();
      }
    }
  }

  // show
  #ifdef SD_WRITE_MSG_DEBUG
    Serial.println("SPI_SPEED : " + String(SPI_SPEED));
  #endif
}


// Not a full RTOS task, it belong to the corresponding task (e.g : recordDHT, recordBattery ... etc)
// We only use vTaskDelay to prevent blocking
void writeMsgToPath(String path, String msg, String customTimeStamp = "", bool replace = false, bool timeStamp = true){
  
  #ifdef SD_WRITE_MSG_DEBUG
    Serial.println("writeMsgToPath called : " + String(millis()));
  #endif

  myFileFormat tempfile;

  // enable timestamp
  if(timeStamp){
    // default timestamp
    if(customTimeStamp == ""){
      msg = "[" + String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + "] " + msg; 
    }
    // custom timestamp
    else{
      msg = "[" + customTimeStamp + "] " + msg; 
    }
  }
  
  
  while(true){
    // if got the power of control, lock SD opreation 
    if(xSemaphoreTake( xSemaphore_SD, pdMS_TO_TICKS(50) ) == pdTRUE){
      #ifdef SD_WRITE_MSG_DEBUG
        Serial.println("Got Semaphore : " + String(millis()));
      #endif

      // flag setting
      oflag_t flag;
      if(replace){
        flag = O_WRONLY | O_CREAT;
      }else{
        flag = O_WRONLY | O_CREAT | O_APPEND;
      }

      // open file
      // Serial.println("path : " + path);
      // Serial.println("msg : " + msg);
      //// failed to open
      ////// open need char array, not string. So use c_str to convert
      if (!tempfile.open(path.c_str(), flag)) {      
        Serial.println(" --> open " + path + " failed");
        showErrorLedThenReboot();
      }
      //// success to open
      else{
        tempfile.println(msg.c_str());
      }

      // close file
      tempfile.close();
      xSemaphoreGive( xSemaphore_SD );
      break;
    }
  }
}


void checkAndCreateFolder(String path){
  bool isExist;
  // lock sd 
  if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
    isExist = sd.exists(path);
  }xSemaphoreGive( xSemaphore_SD );

  if (!isExist) {
    // lock sd 
    if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
      sd.mkdir(path);
    }xSemaphoreGive( xSemaphore_SD );

    bool isSysLogExist;
    if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
      isSysLogExist = sd.exists(systemLogPath);
    }xSemaphoreGive( xSemaphore_SD );

    if (isSysLogExist) {
      writeMsgToPath(systemLogPath, "===> folder at path : " + path + " create successful", "", false, false);
    }else{
      Serial.println("folder at path : " + path + " create successful");
    }
  }
}


void checkAndCreateFile(String path){
  myFileFormat createFile;
  bool isExist;
  // lock sd 
  if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
    isExist = sd.exists(path);
  }xSemaphoreGive( xSemaphore_SD );

  if (!isExist) {
    bool isCreateFileSuccess;
    if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
      isCreateFileSuccess = createFile.open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND);
    }xSemaphoreGive( xSemaphore_SD );
    if (!isCreateFileSuccess) {     // open need char array, not string. So use c_str to convert
      bool isSysLogExist;
      if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
        isSysLogExist = sd.exists(systemLogPath);
      }xSemaphoreGive( xSemaphore_SD );
      if (isSysLogExist) {
        writeMsgToPath(systemLogPath, "open " + path + " failed");
      }else{
        Serial.println("open " + path + " failed");
      }
    }else{
      if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
        createFile.close();
      }xSemaphoreGive( xSemaphore_SD );
      bool isSysLogExist;
      if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
        isSysLogExist = sd.exists(systemLogPath);
      }xSemaphoreGive( xSemaphore_SD );
      if (isSysLogExist) {
        writeMsgToPath(systemLogPath, "file : " + path + " create successful");
      }else{
        Serial.println("file : " + path + " create successful");
      }
    }
  }
}






// void progressCallBack(size_t currSize, size_t totalSize) {
//       Serial.printf("CALLBACK:  Update process at %d of %d bytes...\n", currSize, totalSize);
// }
// void checkUpdate(){
//   Serial.print(F("\nSearch for firmware.."));
//   File firmware =  sd.open("/firmware.bin");
//   if (firmware) {
//       Serial.println(F("found!"));
//       Serial.println(F("Try to update!"));
 
//       Update.onProgress(progressCallBack);
 
//       Update.begin(firmware.size(), U_FLASH);
//       Update.writeStream(firmware);
//       if (Update.end()){
//           Serial.println(F("Update finished!"));
//       }else{
//           Serial.println(F("Update error!"));
//           Serial.println(Update.getError());
//       }
 
//       firmware.close();
 
//       if (sd.rename("/firmware.bin", "/firmware.bak")){
//           Serial.println(F("Firmware rename succesfully!"));
//       }else{
//           Serial.println(F("Firmware rename error!"));
//       }
//       delay(2000);
 
//       ESP.restart();
//   }else{
//       Serial.println(F("not found!"));
//   }
// }



#endif