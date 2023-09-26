#include "rtc_timer.h"

#ifndef SD_OPERATION_H
#define SD_OPERATION_H

#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#define SPI_SPEED SD_SCK_MHZ(4)
#define CHIP_SELECT 5
const int8_t DISABLE_CHIP_SELECT = -1;


void showErrorLed(){
  digitalWrite(16, HIGH); 
}


// global file pointer / SD_FAT_TYPE 3 (FAT16/FAT32 and exFAT)
SdFs sd;
FsFile systemLog;
FsFile sensorData;

void SDInit(){
  if (!sd.begin(CHIP_SELECT, SPI_SPEED)) {
    Serial.println("SD card init error");
    while(1){
      delay(1000);
    }
  }
}

void writeMsgToPath(String path, String msg, bool replace = false, bool timeStamp = true){
  FsFile tempfile;
  
  if(timeStamp){
    msg = "[" + String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + "] " + msg; 
  }
  
  // lock SD opreation 
  if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){

    if(replace){
      if (!tempfile.open(path.c_str(), O_WRONLY | O_CREAT)) {     // open need char array, not string. So use c_str to convert
        Serial.println(" --> open " + path + " failed");
        showErrorLed();
        while(1){
          delay(1000);
        }
      }else{
        tempfile.println(msg.c_str());
      }
    }else{
      if (!tempfile.open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND)) {     // open need char array, not string. So use c_str to convert
        Serial.println(" --> open " + path + " failed");
        showErrorLed();
        while(1){
          delay(1000);
        }
      }else{
        tempfile.println(msg.c_str());
      }
    }
    tempfile.close();

  }xSemaphoreGive( xSemaphore_SD );
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
      writeMsgToPath(systemLogPath, "folder at path : " + path + " create successful");
    }else{
      Serial.println("folder at path : " + path + " create successful");
    }
  }
}


void checkAndCreateFile(String path){
  FsFile createFile;

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




#endif