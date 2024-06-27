/*---- include guard ----*/
#ifndef UTILLS_H
#define UTILLS_H

/*---- macro or define ----*/
#define PRINT_RESET_REASON
#define PRINT_WAKEUP_REASON

/*---- official lib ----*/ 
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <WiFiAP.h>

/*---- other involve lib  ----*/
#include "sd_operation.h"  // include rtc_timer / setting 
#include "softAp.h"
#include "updateServer.h"

/*---- classes, variables or function define  ----*/

/*-------- function implement --------*/


bool isFirstCheckEvaluation = true;
DateTime eva = DateTime(2023, 11, 1, 0, 0, 0);
void checkEvaluation(){
  if (isEvaluation && !skipEvalution){
    if(isFirstCheckEvaluation){
      // Write log
      Serial.println("======== !! You are in the evaluation mode !! ========");
      writeMsgToPath(systemLogPath, "======== !! You are in the evaluation mode !! ========");
      isFirstCheckEvaluation = false;
    }
    // if evaluation is end 
    if(now.unixtime() >= eva.unixtime()){
      Serial.println("======== !! Evaluation time has ended, don't forget to install activate key !! ========");
      writeMsgToPath(systemLogPath, "======== !! Evaluation time has ended, don't forget to install activate key !! ========");
      digitalWrite(16, HIGH); 
      while(1){
        delay(1000000);
      }
    }
  }else{
    if(isFirstCheckEvaluation){
      // Write log
      Serial.println("Normal mode");
      writeMsgToPath(systemLogPath, "Normal mode");
      isFirstCheckEvaluation = false;
    }
  }
}


void getResetReason(){
  esp_reset_reason_t reset_reason = esp_reset_reason();
  String reason_text;
  switch(reset_reason){
    case ESP_RST_POWERON : reason_text = "ESP_RST_POWERON"; break;
    case ESP_RST_EXT : reason_text = "ESP_RST_EXT"; break;
    case ESP_RST_SW : reason_text = "ESP_RST_SW"; break;
    case ESP_RST_PANIC : reason_text = "ESP_RST_PANIC"; break;
    case ESP_RST_INT_WDT : reason_text = "ESP_RST_INT_WDT"; break;
    case ESP_RST_TASK_WDT : reason_text = "ESP_RST_TASK_WDT"; break;
    case ESP_RST_WDT : reason_text = "ESP_RST_WDT"; break;
    case ESP_RST_DEEPSLEEP : reason_text = "ESP_RST_DEEPSLEEP"; break;
    case ESP_RST_BROWNOUT : reason_text = "ESP_RST_BROWNOUT"; break;
    case ESP_RST_SDIO : reason_text = "ESP_RST_SDIO"; break;
    default : reason_text = "ESP_RST_UNKNOWN"; break;
  }
  #ifdef PRINT_RESET_REASON
    Serial.println("== " + reason_text + " ==");
  #endif
  writeMsgToPath(systemLogPath, "== " + reason_text + " ==");
}

void getWakeupReason(){
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  String reason_text;
  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : reason_text = "Wakeup caused by external signal using RTC_IO"; break;
    case ESP_SLEEP_WAKEUP_EXT1 : reason_text = "Wakeup caused by external signal using RTC_CNTL"; break;
    case ESP_SLEEP_WAKEUP_TIMER : reason_text = "Wakeup caused by timer"; break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : reason_text = "Wakeup caused by touchpad"; break;
    case ESP_SLEEP_WAKEUP_ULP : reason_text = "Wakeup caused by ULP program"; break;
    default : reason_text = "Wakeup was not caused by deep sleep"; break;
  }
  #ifdef PRINT_WAKEUP_REASON
    Serial.println("== " + reason_text + " ==");
  #endif
  writeMsgToPath(systemLogPath, "== " + reason_text + " ==");
  delay(1); // for log print order
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

void checkFirmwareUpdate(){
  Serial.println("== Start to check for firmware update ==");
  if(isFirstCheckOTA){
    Serial.println("Fisrt boot -- check firmware updates");
    startSoftAp();
    if (isNeedToUpdate) {
      // turn rtc switch on if test env is PCB board
      #ifndef HAND_MADE_ENV   //current is pcb_v1 env
        digitalWrite(RTC_PMOS, POWER_ON);   
      #endif
      // the start the server
      startUpdateServer();
      // the code will keep execute to next
      // block the code get into "closeSoftAP()""
      while(1) {yield();}
    }
    closeSoftAP();
    isFirstCheckOTA = false;
  }else{
    Serial.println("Not fisrt boot -- skip firmware updates check");
  }
}

// 
void readTaskIndex_moveToNext() {
  // always add one until there have a task
  while(1){
    readTaskIndex +=1;
    //// cross day
    if (readTaskIndex == MIN_A_DAY){
      Serial.println("checkTimeAndTask : Task array out of range, it is cross-day task! Re-zero index now.");
      writeMsgToPath(systemLogPath, "checkTimeAndTask : Task array out of range, it is cross-day task! Re-zero index now.");
      readTaskIndex = 0;
    }
    if (taskScheduleList[readTaskIndex] > 0){
      break;
    }
  }
}

void readSettingIndex_moveToNext() {
  readSettingIndex +=1;
  //// overflow re-zero
  if (readSettingIndex == recordSettingArrayUsedSize){
    Serial.println("checkTimeAndTask : record sound setting array out of range, re-zero index now.");
    writeMsgToPath(systemLogPath, "checkTimeAndTask : record sound setting  array out of range, re-zero index now.");
    readSettingIndex = 0;
  }
}

#endif