#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <WiFiAP.h>
#include "softAp.h"
#include "updateServer.h"

#define ms_TO_uS_FACTOR 1000ULL
unsigned long loopStartTime;

void setup(void) {
  Serial.begin(115200);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  if(isFirstBoot){
    Serial.println("Fisrt boot -- check firmware updates");
    startSoftAp();
    if (isNeedToUpdate) startUpdateServer();
    closeSoftAP();
    isFirstBoot = false;
  }else{
    Serial.println("Not fisrt boot -- skip firmware updates check");
  }
  
  loopStartTime = millis();
}


void loop(void) {
  digitalWrite(LED_PIN, LOW);  // turn the LED on (HIGH is the voltage level)
  delay(100);                      // wait for a second
  digitalWrite(LED_PIN, HIGH);   // turn the LED off by making the voltage LOW
  delay(100);      

  if ((millis() - loopStartTime) >= 5000){
    Serial.println("Trigger sleep");
    delay(2);
    esp_sleep_enable_timer_wakeup(1 * 1000 * ms_TO_uS_FACTOR);  // 1 sec
    esp_deep_sleep_start();
  }
}
