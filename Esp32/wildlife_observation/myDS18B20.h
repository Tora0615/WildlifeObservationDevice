/*---- include guard ----*/
#ifndef MYDS18B20_H
#define MYDS18B20_H

/*---- macro or define ----*/

/*---- official lib ----*/ 
#include <OneWire.h>
#include <DallasTemperature.h>

/*---- other involve lib  ----*/
#include "utills.h"

/*---- classes, variables or function define  ----*/
OneWire oneWire(ONE_WIRE_BUS);        // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensor(&oneWire);   // Pass our oneWire reference to Dallas Temperature. 


/*-------- function implement --------*/
void DS18B20Init(){
  // Power part (32 Pin)
  pinMode(DS18B20_PMOS, OUTPUT);
  #ifdef DS18B20_DEBUG
    Serial.println("Turn off DS18B20 POWER");
  #endif 
  digitalWrite(DS18B20_PMOS, POWER_OFF);   // Turn off. GPIO default is low ->  will let mic ON

  // Start up the library
  sensor.begin();
  // Write log
  writeMsgToPath(systemLogPath, "DS18B20 init successful!");
}


void turnOnDs18b20Power(){
  // power on
  #ifdef DS18B20_DEBUG
    Serial.println("Turn on DS18B20 POWER");
  #endif 
  digitalWrite(DS18B20_PMOS, POWER_ON);   // Turn on.
  // delay(100);  // by task_scheduler
}



float getDS18B20Temp(){
  if(!isDS18B20Recording){
    // lock DS18B20
    isDS18B20Recording = !isDS18B20Recording;
  
    // power on
    // turnOnDs18b20Power();  // by task_scheduler

    // get data 
    int retryCount = 0;
    float tempC;
    while(1){
      sensor.requestTemperatures(); // Send the command to get temperatures
      tempC = sensor.getTempCByIndex(0);
      if(tempC != DEVICE_DISCONNECTED_C || retryCount == 10){
        break;
      }
      retryCount += 1;
    }

    // power off 
    #ifdef DS18B20_DEBUG
      Serial.println("Turn off DS18B20 POWER");
    #endif 
    digitalWrite(DS18B20_PMOS, POWER_OFF);   // Turn off. GPIO default is low ->  will let mic ON

    // release DS18B20
    isDS18B20Recording = !isDS18B20Recording;

    // Check if reading was successful and return 
    if(tempC != DEVICE_DISCONNECTED_C){
      return tempC;
    }else{
      #ifdef DS18B20_DEBUG
        Serial.println("Error: Could not read temperature data");
      #endif 
      // Write log
      writeMsgToPath(systemLogPath, "Error: Could not read temperature data");
      return 0.0;
    }
  }
  else{  // DHT is using 
    // Write log
    writeMsgToPath(systemLogPath, "Now DS18B20 sensor is using, skip this task");
  }
}




#endif