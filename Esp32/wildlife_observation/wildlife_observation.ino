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
  
  // for test
  // #ifdef USE_FAKE_TIME
  //   setTestTime();
  // #endif
  
  // check if need to change time
  checkRtcAdjustFile();

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
  if (notfirstBoot) getResetReason();

  // check schedule and setting doc
  checkScheduleFileExist(); 
  addAllTaskFromFile(); 
  if(isFirstBoot){
    Serial.println("!! First boot : start to find the matched arrayReadIndex !!");
    writeMsgToPath(systemLogPath, "First boot : start to find the matched arrayReadIndex");
    findTheMatchedArrayReadIndex(); 
    isFirstBoot = false;
  }else {
    Serial.println("!! Still alive : skip to read task index from file !!");
    writeMsgToPath(systemLogPath, "Still alive : skip to read task index from file");
  }
  
  // system advance part inint
  batteryMonitorInit(); 
  DS18B20Init(); 
  DHT_init(); 

  // shine and close
  showInitFinishedLED(); 

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