#include "myScheduler.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // disable brownout detector
  Serial.begin(115200);  
  // To indicate setup status, it will allway on if failed
  initLEDs();
  showInitStatusLED(JUST_START);
  
  // system basic part init 
  RTCInit();
  showInitStatusLED(RTC_STARTED);
  batteryMonitorInit(false);  // init without write msg to sd


  // Before trigger battery protect (3.2v <---> 3v), just show LED and sleep
  if (getBatteryVoltage() < EMPTY_BATTERY_VOLTAGE){ 
    digitalWrite(LED_RED, HIGH); 
    const int LOW_BAT_SLEEP_MIN = 5;
    esp_sleep_enable_timer_wakeup(LOW_BAT_SLEEP_MIN * 60 * 1000 * ms_TO_uS_FACTOR);
    while(1){
      Serial.println("Sleep for 5 min");
      delay(10);
      esp_light_sleep_start();
    }
  }

  checkFirmwareUpdate();  // server open here, rely on rtc lib and battery 


  SDInit();
  showInitStatusLED(SD_STARTED);
  
  // for test
  // #ifdef USE_FAKE_TIME
  //   setTestTime();
  // #endif
  
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
    isFirstBoot = false;
  }else {
    Serial.println("!! Still alive : skip to read task index from file !!");
    writeMsgToPath(systemLogPath, "Still alive : skip to read task index from file");
  }
  
  // system advance part inint
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