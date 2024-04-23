#include "esp32-hal.h"
/*---- include guard ----*/
#ifndef LED_H
#define LED_H

/*---- macro or define ----*/

/*---- official lib ----*/ 

/*---- other involve lib  ----*/
#include "setting.h"

// reg LED to system
void initLEDs(){
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}


// Only LED function put here
void showErrorLed(){
  Serial.println("!!!! showErrorLed : triggered !!!!");
  digitalWrite(LED_RED, HIGH); 
  delay(10000);
  ESP.restart();
}

// 
// init state visible
void showInitStatusLED(int stage){
  if (stage == JUST_START){
    // all on 
    digitalWrite(LED_BLUE, HIGH); 
    digitalWrite(LED_GREEN, HIGH); 
    digitalWrite(LED_RED, HIGH); 
    delay(100);
  } else if (stage == SD_STARTED) {
    // blue off others on
    digitalWrite(LED_BLUE, LOW); 
    digitalWrite(LED_GREEN, HIGH); 
    digitalWrite(LED_RED, HIGH); 
  } else if (stage == RTC_STARTED) {
    // green off others on
    digitalWrite(LED_BLUE, HIGH); 
    digitalWrite(LED_GREEN, LOW); 
    digitalWrite(LED_RED, HIGH); 
  } else if (stage == SCHEDULE_FILE_EXIST) {
    // red off others on
    digitalWrite(LED_BLUE, HIGH); 
    digitalWrite(LED_GREEN, HIGH); 
    digitalWrite(LED_RED, LOW); 
  } else if (stage == TASK_ADDED) {
    // red & blue off, green on
    digitalWrite(LED_BLUE, LOW); 
    digitalWrite(LED_GREEN, HIGH); 
    digitalWrite(LED_RED, LOW); 
  } else if (stage == ALL_INIT_FINISHED) {
    // all off
    digitalWrite(LED_BLUE, LOW); 
    digitalWrite(LED_GREEN, LOW); 
    digitalWrite(LED_RED, LOW); 
  }
}


// running state visible
void aliveLedShow(){
  // show 5 ms we called
  digitalWrite(LED_BLUE, HIGH);  
  vTaskDelay(5 / portTICK_PERIOD_MS);                   
  digitalWrite(LED_BLUE, LOW);   
}

void runningTaskLedShow(){
  // show 5 ms we called
  digitalWrite(LED_GREEN, HIGH);  
  vTaskDelay(5 / portTICK_PERIOD_MS);                   
  digitalWrite(LED_GREEN, LOW);   
}

void lowBatteryLedShow(){
  // show 5 ms we called
  digitalWrite(LED_RED, HIGH);  
  vTaskDelay(5 / portTICK_PERIOD_MS);                   
  digitalWrite(LED_RED, LOW);   
}

#endif