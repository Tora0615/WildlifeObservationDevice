#ifndef MYDS18B20_H
#define MYDS18B20_H

#include "setting.h"

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
  writeMsgToPath(systemLogPath, "DS18B20 init successful");
}

float GetDS18B20Temp(){
  // power on
  #ifdef DS18B20_DEBUG
    Serial.println("Turn on DS18B20 POWER");
  #endif 
  digitalWrite(DS18B20_PMOS, LOW);   // Turn on.
  delay(100);

  // get data 
  sensor.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensor.getTempCByIndex(0);

  // power off 
  #ifdef DS18B20_DEBUG
    Serial.println("Turn off DS18B20 POWER");
  #endif 
  digitalWrite(DS18B20_PMOS, HIGH);   // Turn off. GPIO default is low ->  will let mic ON

  // Check if reading was successful and return 
  if(tempC != DEVICE_DISCONNECTED_C){
    return tempC;
  }else{
    Serial.println("Error: Could not read temperature data");
  }
}




#endif