#include "rtc_timer.h"

#ifndef SD_OPERATION_H
#define SD_OPERATION_H

#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#define SPI_SPEED SD_SCK_MHZ(4)
#define CHIP_SELECT 5
const int8_t DISABLE_CHIP_SELECT = -1;


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
  
  
  if(replace){
    if (!tempfile.open(path.c_str(), O_WRONLY | O_CREAT)) {     // open need char array, not string. So use c_str to convert
      Serial.println(" --> open " + path + " failed");
      while(1){
        delay(1000);
      }
    }else{
      tempfile.println(msg.c_str());
    }
  }else{
    if (!tempfile.open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND)) {     // open need char array, not string. So use c_str to convert
      Serial.println(" --> open " + path + " failed");
      while(1){
        delay(1000);
      }
    }else{
      tempfile.println(msg.c_str());
    }
  }
}


void checkAndCreateFolder(String path){
  if (!sd.exists(path)) {
    sd.mkdir(path);
    if (sd.exists(systemLogPath)) {
      writeMsgToPath(systemLogPath, "folder at path : " + path + " create successful");
    }else{
      Serial.println("folder at path : " + path + " create successful");
    }
  }
}


void checkAndCreateFile(String path){
  FsFile createFile;
  if (!sd.exists(path)) {
    if (!createFile.open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND)) {     // open need char array, not string. So use c_str to convert
      if (sd.exists(systemLogPath)) {
        writeMsgToPath(systemLogPath, "open " + path + " failed");
      }else{
        Serial.println("open " + path + " failed");
      }
    }else{
      createFile.close();
      if (sd.exists(systemLogPath)) {
        writeMsgToPath(systemLogPath, "file : " + path + " create successful");
      }else{
        Serial.println("file : " + path + " create successful");
      }
    }
  }
}




#endif