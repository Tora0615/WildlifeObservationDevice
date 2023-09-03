#include "battery.h"
#include "rtc.h"

// uint32_t rtc_time

void setup() {

  Serial.begin(115200);


  // system init 
  // time < test or have ACTIVATECODE, pass

  // get RTC time
  
  batteryMonitorInit();
  
}

void loop() {

  Serial.println("Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)");
  delay(1000);
}



/*
SPI
MOSI : 23
MISO : 19 
SCK : 18
SS : 5
*/