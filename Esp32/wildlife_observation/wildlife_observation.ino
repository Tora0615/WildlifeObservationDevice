#include "setting.h"   // put at the first line
#include "taskFileReader.h"
#include "battery.h"
#include "rtc_timer.h"
#include "sd_operation.h"
#include "myDS18B20.h"
#include "utills.h"
#include "myDHT.h"

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  900        /* Time ESP32 will go to sleep (in seconds) */



String today;

void setup() {
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
  bool notfirstBoot = sd.exists(systemLogPath);
  checkAndCreateFile(systemLogPath);

  // print reboot msg if not first boot
  if (notfirstBoot) writeMsgToPath(systemLogPath, "== reboot (or wake up) ==");

  // check schedule and setting doc
  checkScheduleFileExist();
  addAllTaskFromFile();


  while(1) delay(1000);


  // system advance part inint
  batteryMonitorInit();
  DS18B20Init();
  DHT_init();


  // time < test or have ACTIVATECODE, pass

  

  
  showInitFinishedLED();
}



void loop() {

  checkDayChange();
  Serial.println(getDate() + "_" + secMapTo24Hour(getPassedSecOfToday()));

  Serial.println("today passed sec : " + String(getPassedSecOfToday()));
  Serial.println("Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)");
  Serial.println("DS18B20 : " + String(GetDS18B20Temp()));
  Serial.println("DHT temperature : " + String(DHT_get_temperature()) + " / DHT Humidity : " + String(DHT_get_Humidity()));

  writeMsgToPath(systemLogPath, getDate() + "_" + secMapTo24Hour(getPassedSecOfToday()));
  writeMsgToPath(systemLogPath, "Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)");
  writeMsgToPath(systemLogPath, "DS18B20 : " + String(GetDS18B20Temp()));
  writeMsgToPath(systemLogPath, "DHT temperature : " + String(DHT_get_temperature()) + " / DHT Humidity : " + String(DHT_get_Humidity()));
  

  // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  // writeMsgToPath(systemLogPath, "sleep 15 min");
  // esp_deep_sleep_start();
  Serial.println(" ");
}













/*
SPI
MOSI : 23
MISO : 19 
SCK : 18
SS : 5
*/