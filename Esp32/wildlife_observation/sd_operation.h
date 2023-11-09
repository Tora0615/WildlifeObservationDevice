#include "rtc_timer.h"

#ifndef SD_OPERATION_H
#define SD_OPERATION_H

#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#define SPI_SPEED SD_SCK_MHZ(10)
#define CHIP_SELECT 5
const int8_t DISABLE_CHIP_SELECT = -1;


// debug switch
// #define SD_WRITE_MSG_DEBUG


void showErrorLed(){
  digitalWrite(16, HIGH); 
}

// global file pointer (SD_FAT_TYPE 3 (FAT16/FAT32 and exFAT) )
#ifdef SD_USE_BASIC
  SdFs sd;
  FsFile systemLog;
  FsFile sensorData;
#else
  SdExFat sd;
  ExFile systemLog;
  ExFile sensorData;
#endif 

void SDInit(){

  if(!sd.begin(CHIP_SELECT, SPI_SPEED)){
    Serial.println("SD card init error : NO SD card");
    while(1){
      delay(1000);
    }
  }

  // show
  #ifdef SD_WRITE_MSG_DEBUG
    Serial.println("SPI_SPEED : " + String(SPI_SPEED));
  #endif
}

void writeMsgToPath(String path, String msg, String customTimeStamp = "", bool replace = false, bool timeStamp = true){
  
  #ifdef SD_WRITE_MSG_DEBUG
    Serial.println("writeMsgToPath called : " + String(millis()));
  #endif

  #ifdef SD_USE_NORMAL
    FsFile tempfile;
  #else
    ExFile tempfile;
  #endif 

  if(timeStamp){
    if(customTimeStamp == ""){
      msg = "[" + String(today) + "_" + String(secMapTo24Hour(getPassedSecOfToday())) + "] " + msg; 
    }else{
      msg = "[" + customTimeStamp + "] " + msg; 
    }
  }
  
  // lock SD opreation 
  if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
    #ifdef SD_WRITE_MSG_DEBUG
      Serial.println("Got Semaphore : " + String(millis()));
    #endif
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
      writeMsgToPath(systemLogPath, "===> folder at path : " + path + " create successful", "", false, false);
    }else{
      Serial.println("folder at path : " + path + " create successful");
    }
  }
}


void checkAndCreateFile(String path){
  #ifdef SD_USE_NORMAL
    FsFile createFile;
  #else
    ExFile createFile;
  #endif 

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




void checkRtcAdjustFile(){
  char timeWords[15] = {0};
  long theTimeNeedToBeSet = -1;
  #ifdef SD_USE_BASIC
    FsFile setTimeFile;
  #else
    ExFile setTimeFile;
  #endif 

  // check file exist or not 
  if (sd.exists("setTime.txt")) {
    // if exist 
    if (!setTimeFile.open("setTime.txt", O_RDONLY)) {
      Serial.println("open RTC set file failed"); 
    }

    // read a line 
    int n = setTimeFile.fgets(timeWords, sizeof(timeWords));  // e.q. 20231109203200 -> 14 nums
    if (timeWords[n - 1] != '\n' && n == (sizeof(timeWords) - 1)) {
      Serial.println("time format illegal, it is too long");
    }

    // finished
    setTimeFile.close();
    sd.remove("setTime.txt");
  }

  // if need to set time, then do it
  if(theTimeNeedToBeSet != -1){
    setTime(timeWords);
  }
}

#endif