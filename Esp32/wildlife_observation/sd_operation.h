#include "rtc_timer.h"

#ifndef SD_OPERATION_H
#define SD_OPERATION_H

#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#define SPI_SPEED SD_SCK_MHZ(15)
#define CHIP_SELECT 5
const int8_t DISABLE_CHIP_SELECT = -1;


// debug switch
// #define SD_WRITE_MSG_DEBUG


void showErrorLed(){
  digitalWrite(16, HIGH); 
  // reboot
  ESP.restart();
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
      // reboot
      ESP.restart();
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
  
  
  while(true){
    // vTaskDelay(50);
    // if got the power of control, lock SD opreation 
    if(xSemaphoreTake( xSemaphore_SD, pdMS_TO_TICKS(50) ) == pdTRUE){
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
  #ifdef SD_USE_BASIC
    FsFile setTimeFile;
  #else
    ExFile setTimeFile;
  #endif 

  // check file exist or not 
  if (sd.exists("setTime.txt")) {
    Serial.println("|-- RTC adjust file exist");

    // if exist 
    if (!setTimeFile.open("setTime.txt", O_RDONLY)) {
      Serial.println("open RTC adjust file failed"); 
    }

    // read all 14 char
    for (int i=0; i<14; i++){
      timeWords[i] = setTimeFile.read();
    }
    timeWords[14] = '\0';
    Serial.println("|-- The time read from file : " + String(timeWords));

    // Do file operation 
    setTimeFile.close();
    #ifdef KEEP_SET_TIME_FILE
      Serial.println("!! Don't forgot to adjust the setting of change time by file !!");
    #else
      sd.remove("setTime.txt");
    #endif
    Serial.println("|-- setTime.txt deleted");

    // set time
    setTime(timeWords);
  }else{
    Serial.println("|-- No need to adjust RTC time, skip");
  }
}

#endif