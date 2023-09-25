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

  // Write log
  writeMsgToPath(systemLogPath, "DHT init successful!");
}


void turnOnDhtPower(){
  // power on
  #ifdef DHT22_DEBUG
    Serial.println("Turn on DHT POWER");
    Serial.println(millis());
  #endif 
  digitalWrite(DHT22_PMOS, LOW);   // Turn on.
  // delay(200);
}


float DHT_get_temperature(){
  // turnOnDhtPower();

  // get value 
  const float sample_time = 1.0; // 10.0;
  float sum = 0;
  for (int i = 0; i<sample_time; i++){
    float temp = dht.readTemperature();
    #ifdef DHT22_DEBUG
      Serial.println("DHT Temperature : " + String(temp));
      Serial.println(millis());
    #endif 
    sum += temp;
    // delay(50);
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
  // turnOnDhtPower();

  // get value 
  const float sample_time = 1.0; // 10.0;
  float sum = 0;
  for (int i = 0; i<sample_time; i++){
    float temp = dht.readHumidity();
    #ifdef DHT22_DEBUG
      Serial.println("DHT Humidity : " + String(temp));
    #endif 
    sum += temp; 
    // delay(50);
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
