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
  

  // print reboot msg if not first boot (sd is new)
  bool notfirstBoot = sd.exists(systemLogPath);
  if (notfirstBoot) getWakeupReason();
  if (notfirstBoot) getResetReason();

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

  // task Schduler top level task add
  runner.init();
  runner.addTask(t_checkDayChange);
  t_checkDayChange.enable();
  runner.addTask(t_checkIsNeedToRunTask);
  t_checkIsNeedToRunTask.enable();

  // use dual core by RTOS 
  createRTOSTasks();
}


void loop() {
}


/*
SPI
MOSI : 23
MISO : 19 
SCK : 18
SS : 5
*/