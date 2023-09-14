#ifndef MYDS18B20_H
#define MYDS18B20_H


// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
#include "sd_operation.h"

// Data wire pin
#define ONE_WIRE_BUS 25

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensor(&oneWire);


void DS18B20Init(){
  // Start up the library
  sensor.begin();
  // Write log
  writeMsgToPath(systemLogPath, "DS18B20 init successful");
}

float GetDS18B20Temp(){
  sensor.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensor.getTempCByIndex(0);

  // Check if reading was successful
  if(tempC != DEVICE_DISCONNECTED_C){
    return tempC;
  }else{
    Serial.println("Error: Could not read temperature data");
  }
}




#endif