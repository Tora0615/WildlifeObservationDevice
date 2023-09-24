#include "setting.h"   // put at the first line
#include "taskFileReader.h"
#include "battery.h"
#include "rtc_timer.h"
#include "sd_operation.h"
#include "myDS18B20.h"
#include "utills.h"
#include "myDHT.h"
#include "myINMP441.h"
#include "myScheduler.h"


void setup() {
  // To indicate setup status, it will allway on if failed
  turnOnLed();

  Serial.begin(115200);  

  // system basic part init 
  SDInit();
  RTCInit();

  // get current time as stander time
  sys_RTC_time_offset = GetHowManySecondsHasPassedTodayFromRtc();
  sys_millis_time_offset = millis();

  // create first folder and file in the 
  today = getDate();
  checkAndCreateFolder(today);
  systemLogPath = today + "/SYSLOG.txt";
  checkAndCreateFile(systemLogPath);
  sensorDataPath = today + "/SENSOR_DATA.txt";
  checkAndCreateFile(sensorDataPath);
  

  // print reboot msg if not first boot
  bool notfirstBoot = sd.exists(systemLogPath);
  if (notfirstBoot) writeMsgToPath(systemLogPath, "== reboot (or wake up) ==");

  // check schedule and setting doc
  checkScheduleFileExist();
  addAllTaskFromFile();
  findTheMatchedArrayReadIndex();

  // system advance part inint
  batteryMonitorInit();
  DS18B20Init();
  DHT_init();

  // shine and close
  showInitFinishedLED();

  // top level task add
  runner.init();
  runner.addTask(t_checkDayChange);
  t_checkDayChange.enable();
  runner.addTask(t_checkIsNeedToRunTask);
  t_checkIsNeedToRunTask.enable();
}



void loop() {

  // checkDayChange();
  // Serial.println(getDate() + "_" + secMapTo24Hour(getPassedSecOfToday()));

  // Serial.println("today passed sec : " + String(getPassedSecOfToday()));
  // Serial.println("Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)");
  // Serial.println("DS18B20 : " + String(getDS18B20Temp()));
  // Serial.println("DHT temperature : " + String(DHT_get_temperature()) + " / DHT Humidity : " + String(DHT_get_Humidity()));

  // writeMsgToPath(sensorDataPath, getDate() + "_" + secMapTo24Hour(getPassedSecOfToday()));
  // writeMsgToPath(sensorDataPath, "Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)");
  // writeMsgToPath(sensorDataPath, "DS18B20 : " + String(getDS18B20Temp()));
  // writeMsgToPath(sensorDataPath, "DHT temperature : " + String(DHT_get_temperature()) + " / DHT Humidity : " + String(DHT_get_Humidity()));
  



  // recordWithDualChannel(10, "/10secDual2X.wav", 2);

  // Serial.println(" ");


  runner.execute(); 
}













/*
SPI
MOSI : 23
MISO : 19 
SCK : 18
SS : 5
*/