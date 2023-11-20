#ifndef UTILLS_H
#define UTILLS_H

#define PRINT_RESET_REASON
#define PRINT_WAKEUP_REASON

void turnOnLed(){
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH); 
  delay(100);
}

void showInitFinishedLED(){
  digitalWrite(16, LOW); 
  delay(100);
  digitalWrite(16, HIGH); 
  delay(100);
  digitalWrite(16, LOW); 
  delay(100);
  digitalWrite(16, HIGH); 
  delay(100);
  digitalWrite(16, LOW); 
  delay(100);
  digitalWrite(16, HIGH); 
  delay(1000);
  digitalWrite(16, LOW); 
}

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
}

#endif