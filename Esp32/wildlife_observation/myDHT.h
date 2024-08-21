/*---- include guard ----*/
#ifndef MYDHT_H
#define MYDHT_H

/*---- macro or define ----*/

/*---- official lib ----*/ 
#include <DHT.h>

/*---- other involve lib  ----*/
#include "utills.h"

/*---- classes, variables or function define  ----*/
DHT dht(DHTPIN, DHTTYPE);


/*-------- function implement --------*/
void DHT_init(){
  // Power part (32 Pin)
  pinMode(DHT22_PMOS, OUTPUT);
  #ifdef DHT22_DEBUG
    Serial.println("Turn off DHT POWER");
  #endif 
  digitalWrite(DHT22_PMOS,POWER_OFF);   // Turn off. GPIO default is low ->  will let mic ON

  // Write log
  // writeMsgToPath(systemLogPath, "DHT init successful!");
}


void turnOnDhtPower(){
  // power on
  #ifdef DHT22_DEBUG
    Serial.println("Turn on DHT POWER");
  #endif 
  digitalWrite(DHT22_PMOS, POWER_ON);   // Turn on.
  // delay(200);  // RTOS handle

  // init library 
  dht.begin();
}


float DHT_get_temperature(){
  if(!isDHTRecording){
    // lock DHT
    isDHTRecording = !isDHTRecording;
  
    // power on
    // RTOS handle 
    // turnOnDhtPower();

    // get value 
    const float sample_time = 1.0; // 10.0;
    float sum = 0;
    for (int i = 0; i<sample_time; i++){
      float temp = dht.readTemperature();
      #ifdef DHT22_DEBUG
        Serial.println("[DHT Temperature : " + String(temp) + "]");
        Serial.println(millis());
      #endif 
      sum += temp;
      // delay(50);
    }

    // disable pullup
    gpio_pullup_dis(DHTPIN_FOR_PULLUP_DISABLE);

    // power off 
    #ifdef DHT22_DEBUG
      Serial.println("Turn off DHT POWER");
    #endif 
    digitalWrite(DHT22_PMOS, POWER_OFF);   // Turn off. GPIO default is low ->  will let mic ON

    // release DHT
    isDHTRecording = !isDHTRecording;

    // return value
    return sum/sample_time;
  }
  else{  // DHT is using 
    // Write log
    writeMsgToPath(systemLogPath, "Now DHT sensor is using, skip this task");
    Serial.println("Now DHT sensor is using, skip this task");
  }
}


float DHT_get_Humidity(){
  if(!isDHTRecording){
    // lock DHT
    isDHTRecording = !isDHTRecording;
  
    // power on
    // turnOnDhtPower();

    // get value 
    const float sample_time = 1.0; // 10.0;
    float sum = 0;
    for (int i = 0; i<sample_time; i++){
      float temp = dht.readHumidity();
      #ifdef DHT22_DEBUG
        Serial.println("[DHT Humidity : " + String(temp) + "]");
      #endif 
      sum += temp; 
      // delay(50);
    }

    // disable pullup
    gpio_pullup_dis(DHTPIN_FOR_PULLUP_DISABLE);

    // power off 
    #ifdef DHT22_DEBUG
      Serial.println("Turn off DHT POWER");
    #endif 
    digitalWrite(DHT22_PMOS, POWER_OFF);   // Turn off. GPIO default is low ->  will let mic ON

    // release DHT
    isDHTRecording = !isDHTRecording;

    // return value
    return sum/sample_time;
  }
  else{  // DHT is using 
    // Write log
    writeMsgToPath(systemLogPath, "Now DHT sensor is using, skip this task");
  }
}


#endif
