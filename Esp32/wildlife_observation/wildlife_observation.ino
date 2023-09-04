#include "battery.h"
#include "rtc_timer.h"
#include "sd_operation.h"
#include "DS18B20.h"
#include "utills.h"
#include "setting.h"


String date;

void setup() {
  Serial.begin(115200);  

  // system basic part init 
  SDInit();
  RTCInit();

  // get current time as stander time
  sys_RTC_time_offset = GetHowManySecondsHasPassedTodayFromRtc();
  sys_millis_time_offset = millis();

  // create first folder and file in the 
  date = getDate();
  checkAndCreateFolder(date);
  systemLogPath = date + "/SYSLOG.txt";
  bool notfirstBoot = sd.exists(systemLogPath);
  checkAndCreateFile(systemLogPath);

  // print reboot msg if not first boot
  if (notfirstBoot) writeMsgToPath(systemLogPath, "reboot");

  // check schedule and setting doc
  checkScheduleFile();

  // system advance part inint
  batteryMonitorInit();
  DS18B20Init();


  // time < test or have ACTIVATECODE, pass

  

  

}

void loop() {

  checkDayChange();
  Serial.println(getDate() + "_" + secMapTo24Hour(getPassedSecOfToday()));
  Serial.println("today passed sec : " + String(getPassedSecOfToday()));



  Serial.println("Battery status : " + String(getBatteryVoltage()) + "v (" + String(getBatteryPercentage())+ "%)");
  delay(1800000);
}













/*
SPI
MOSI : 23
MISO : 19 
SCK : 18
SS : 5
*/