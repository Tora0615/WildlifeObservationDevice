#ifndef UTILLS_H
#define UTILLS_H

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


#endif