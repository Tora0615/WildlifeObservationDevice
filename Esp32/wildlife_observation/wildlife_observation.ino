#include "battery.h"
#include "rtc_timer.h"
#include "sd_operation.h"
#include "DS18B20.h"
#include "utills.h"



void setup() {

  Serial.begin(115200);


  // system init 
  // time < test or have ACTIVATECODE, pass

  
  
  batteryMonitorInit();
  RTCInit();
  SDInit();
  DS18B20Init();




  // get current time as stander time
  sys_RTC_time_offset = GetHowManySecondsHasPassedTodayFromRtc();
  sys_millis_time_offset = millis();
}

void loop() {

  Serial.println(getDate() + "_" + secMapTo24Hour(getPassedSecOfToday()));
  Serial.println("today passed sec : " + String(getPassedSecOfToday()));



  Serial.println("Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)");
  delay(5000);
}













/*
SPI
MOSI : 23
MISO : 19 
SCK : 18
SS : 5
*/