#ifndef MYDHT_H
#define MYDHT_H

#include "setting.h"

#include <DHT.h>
#define DHTPIN 33
#define DHTTYPE DHT22

#define DHT22_PMOS 32

DHT dht(DHTPIN, DHTTYPE);

void DHT_init(){
  // Power part (32 Pin)
  pinMode(DHT22_PMOS, OUTPUT);
  #ifdef DHT22_DEBUG
    Serial.println("Turn off DHT POWER");
  #endif 
  digitalWrite(DHT22_PMOS,HIGH);   // Turn off. GPIO default is low ->  will let mic ON

  // init library 
  dht.begin();
}


float DHT_get_temperature(){
  // power on
  #ifdef DHT22_DEBUG
    Serial.println("Turn on DHT POWER");
  #endif 
  digitalWrite(DHT22_PMOS, LOW);   // Turn on.
  delay(100);

  // get value 
  const float sample_time = 10.0;
  float sum = 0;
  for (int i = 0; i<sample_time; i++){
    sum += dht.readTemperature();
    delay(50);
  }

  // power off 
  #ifdef DHT22_DEBUG
    Serial.println("Turn off DHT POWER");
  #endif 
  digitalWrite(DHT22_PMOS, HIGH);   // Turn off. GPIO default is low ->  will let mic ON

  // return value
  return sum/sample_time;
}


float DHT_get_Humidity(){
  // power on
  #ifdef DHT22_DEBUG
    Serial.println("Turn on DHT POWER");
  #endif 
  digitalWrite(DHT22_PMOS, LOW);   // Turn on.
  delay(100);

  // get value 
  const float sample_time = 10.0;
  float sum = 0;
  for (int i = 0; i<sample_time; i++){
    sum += dht.readHumidity();
    delay(50);
  }

  // power off 
  #ifdef DHT22_DEBUG
    Serial.println("Turn off DHT POWER");
  #endif 
  digitalWrite(DHT22_PMOS, HIGH);   // Turn off. GPIO default is low ->  will let mic ON

  // return value
  return sum/sample_time;
}


#endif
