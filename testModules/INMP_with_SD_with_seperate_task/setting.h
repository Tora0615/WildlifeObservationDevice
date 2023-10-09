#ifndef SETTING_H
#define SETTING_H

#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#define SPI_SPEED SD_SCK_MHZ(15)
#define CHIP_SELECT 5
const int8_t DISABLE_CHIP_SELECT = -1;


#include "soc/timer_group_reg.h"
#include "soc/timer_group_struct.h"
void feedDogOfCore(byte core){
  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;   // write enable
  TIMERG0.wdt_feed = 1;                         // feed dog, any value will trigger this 
  TIMERG0.wdt_wprotect = 0;                     // write protect
}


/*----- debug part setting -----*/ 
// #define DS18B20_DEBUG
// #define DHT22_DEBUG
// #define RTC_DEBUG
/*--- myScheduler ---*/
// #define CHECK_DAY_CHANGE_DEBUG
// #define CHECK_IS_NEED_TO_RUN_TASK
// #define RECORD_SOUND_DEBUG
// #define RECORD_BATTERY_DEBUG
// #define GET_DS18B20_TEMP_DEBUG
// #define DHT_GET_TEMPERATURE_DEBUG
// #define DHT_GET_HUMIDITY_DEBUG
/*--- taskFileReader ---*/
// #define PARSE_TASK_DEBUG
// #define ADD_REPEAT_WORKS_DEBUG
// #define SORT_TASK_DEBUG
// #define ADD_ALL_TASK_FROM_FILE_DEBUG
/*--- myINMP441 ---*/
#define RECORD_TIME_DEBUG
// #define PERCENTAGE_DEBUG
#define INMP_COMMON_DEBUG
/*--- MEMS_INMP441 ---*/
// CAN'T set here, please see MEMS_INMP441.h
/*--- sd_operation ---*/
// CAN'T set here, please see that file
/*--- taskFileReader ---*/
#define SHOW_NEXT_TASK_WHEN_FIRST_START
/*--- utils ---*/ 
// CAN'T set here(?), please see that file
/*--- myScheduler ---*/
// #define GOTOSLEEP_DEBUG
// #define FEED_DOG_DEBUG


// some global variable
const String SCHEDULE_FILE = "schedule.txt";
String today;
String systemLogPath;
String sensorDataPath;
int nextTaskPreserveTime_min;

// some status flag
bool isRecording = false;
bool isDHTRecording = false;
bool isDS18B20Recording = false;
bool isTaskAllLock = false;
int isRunningTask = 0;


// RTOS for SD
SemaphoreHandle_t xSemaphore_SD = xSemaphoreCreateMutex();

// RTOS Task setting 
#define INMP_CPU 0
#define OTHER_TASK_CPU 1

// SD Use setting 
#define SD_USE_NORMAL

// use fake test time setting 
// #define USE_FAKE_TIME

#ifdef SD_USE_NORMAL
  SdFs sd;
  FsFile systemLog;
  FsFile sensorData;
#else
  SdExFat sd;
  ExFile systemLog;
  ExFile sensorData;
#endif 


#endif