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
void showErrorLedThenReboot(){
  Serial.println("!!!! showErrorLedThenReboot : triggered !!!!");
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

void quickShine(unsigned long remainTimeMs){
  int LED;
  if ( remainTimeMs > (2.0f * AP_TIMEOUT_MS / 3.0f) ){    // remain > 2/3, blue
    LED = LED_BLUE;
    Serial.println("Shine blue");
  }else if( remainTimeMs > (AP_TIMEOUT_MS / 3.0f) && remainTimeMs < (2.0f * AP_TIMEOUT_MS / 3.0f)){   // 2/3 > remain > 1/3, green
    LED = LED_GREEN;
    Serial.println("Shine green");
  }else{    // remain < 1/3, red
    LED = LED_RED;
    Serial.println("Shine red");
  }
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(100);                      // wait for a second
  digitalWrite(LED, LOW);   // turn the LED off by making the voltage LOW
  delay(100); 
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(100);                      // wait for a second
  digitalWrite(LED, LOW);   // turn the LED off by making the voltage LOW
  delay(100); 
}

void showNoScheduleFileLED(){
    while(1) {
    digitalWrite(LED_RED, LOW); 
    delay(500);
    digitalWrite(LED_RED, HIGH); 
    delay(500);
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