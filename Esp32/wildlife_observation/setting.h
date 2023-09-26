#ifndef SETTING_H
#define SETTING_H

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
#define PERCENTAGE_DEBUG
#define INMP_COMMON_DEBUG

#define INMP_CPU 0
#define OTHER_TASK_CPU 1

/*--- SD ---*/


const String SCHEDULE_FILE = "schedule.txt";
String today;
String systemLogPath;
String sensorDataPath;

// some status flag
bool isRecording = false;
bool isDHTRecording = false;
bool isDS18B20Recording = false;


// RTOS for SD
SemaphoreHandle_t xSemaphore_SD = xSemaphoreCreateMutex();

#define SD_USE_NORMAL

#endif