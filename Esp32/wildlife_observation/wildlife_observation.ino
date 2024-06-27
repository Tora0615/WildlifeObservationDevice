#include "myScheduler.h"

void setup() {
  Serial.begin(115200);  
  // To indicate setup status, it will allway on if failed
  initLEDs();
  showInitStatusLED(JUST_START);
  
  // system basic part init 
  RTCInit();
  showInitStatusLED(RTC_STARTED);
  checkFirmwareUpdate();  // rely on rtc lib
  SDInit();
  showInitStatusLED(SD_STARTED);
  
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
  showInitStatusLED(SCHEDULE_FILE_EXIST);
  addAllTaskFromFile(); 
  showInitStatusLED(TASK_ADDED);
  if(isFirstBoot){
    Serial.println("!! First boot : start to find the matched readTaskIndex !!");
    writeMsgToPath(systemLogPath, "First boot : start to find the matched readTaskIndex");
    findTheIndexOfCurrentTime(); 
    // readSettingIndex add until have task in that index
    //// it have auto re-zero when overflow
    readTaskIndex_moveToNext();
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
  showInitStatusLED(ALL_INIT_FINISHED); 

  // for function Test
  // Serial.println("here");

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