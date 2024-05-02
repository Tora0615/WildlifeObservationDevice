#ifndef SETTING_H
#define SETTING_H

// This status will only keep when light/deep sleep (reset will clean the status)
RTC_DATA_ATTR bool isFirstBoot = true;
bool isNeedToUpdate = false;

void quickShine(){
  digitalWrite(4, LOW);  // turn the LED on (HIGH is the voltage level)
  delay(100);                      // wait for a second
  digitalWrite(4, HIGH);   // turn the LED off by making the voltage LOW
  delay(100); 
  digitalWrite(4, LOW);  // turn the LED on (HIGH is the voltage level)
  delay(100);                      // wait for a second
  digitalWrite(4, HIGH);   // turn the LED off by making the voltage LOW
  delay(100); 
}

#endif