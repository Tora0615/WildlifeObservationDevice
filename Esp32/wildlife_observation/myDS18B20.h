#include "setting.h"

#ifndef MYDS18B20_H
#define MYDS18B20_H

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
#include "sd_operation.h"

// Data wire pin
#define ONE_WIRE_BUS 25
#define DS18B20_PMOS 32

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensor(&oneWire);

void DS18B20Init(){
  // Power part (32 Pin)
  pinMode(DS18B20_PMOS, OUTPUT);
  #ifdef DS18B20_DEBUG
    Serial.println("Turn off DS18B20 POWER");
  #endif 
  digitalWrite(DS18B20_PMOS,HIGH);   // Turn off. GPIO default is low ->  will let mic ON

  // Start up the library
  sensor.begin();
  // Write log
  writeMsgToPath(systemLogPath, "DS18B20 init successful!");
}


void turnOnDs18b20Power(){
  if(!isDHTRecording && !isDS18B20Recording){
    // power on
    #ifdef DS18B20_DEBUG
      Serial.println("Turn on DS18B20 POWER");
    #endif 
    digitalWrite(DS18B20_PMOS, LOW);   // Turn on.
    // delay(100);  // by task_scheduler
  }else{
    #ifdef DS18B20_DEBUG
      Serial.println("DS18B20 POWER has already on");
    #endif 
  }
}



float getDS18B20Temp(){
  if(!isDS18B20Recording){
    // lock DS18B20
    isDS18B20Recording = !isDS18B20Recording;
  
    // power on
    // turnOnDs18b20Power();  // by task_scheduler

    // get data 
    sensor.requestTemperatures(); // Send the command to get temperatures
    float tempC = sensor.getTempCByIndex(0);

    // power off 
    #ifdef DS18B20_DEBUG
      Serial.println("Turn off DS18B20 POWER");
    #endif 
    digitalWrite(DS18B20_PMOS, HIGH);   // Turn off. GPIO default is low ->  will let mic ON

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
    }
  }
  else{  // DHT is using 
    // Write log
    writeMsgToPath(systemLogPath, "Now DS18B20 sensor is using, skip this task");
  }
}




#endif