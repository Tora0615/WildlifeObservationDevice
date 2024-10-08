#include <stdint.h>
/*---- include guard ----*/
#ifndef SETTING_H
#define SETTING_H

/*=================== macro ==================*/
//== value define ==
#define SECONDS_OF_A_DAY 86400    // a day : 86400 sec
#define skipEvalution true        // skip evalution mode
#define INMP_CPU 0                // RTOS Task setting
#define OTHER_TASK_CPU 1
#define SPI_SPEED SD_SCK_MHZ(10)
#define ADC_BIT_12 12
#define FULL_BATTERY_VOLTAGE 4.2    // battery : 4.2
#define LOW_BATTERY_VOLTAGE 3.5     // warning
#define EMPTY_BATTERY_VOLTAGE 3.2   // less then this, hault the system. (protect board : 2.54 +- 0.1 / battery : 2.5, DC_DC module : out 3V  / rt5707wsc need at least ?v to step down to 3v)
#define DOUBLE_PMOS_SWITCH
#ifdef DOUBLE_PMOS_SWITCH
  #define POWER_ON  HIGH
  #define POWER_OFF LOW
#else
  #define POWER_ON  LOW
  #define POWER_OFF HIGH
#endif
#define JUST_START 0
#define SD_STARTED 1
#define RTC_STARTED 2
#define SCHEDULE_FILE_EXIST 3
#define TASK_ADDED 4
#define ALL_INIT_FINISHED 5
//== pin define ==
// LED
#define LED_BLUE 17   // 34, input only ....
#define LED_GREEN 17  // 35, input only ....
#define LED_RED 17 
// RTC
#define RTC_PMOS 4        // 3V_RTC_I2C_SWITCH
//--- RTC is on i2c, no pin to set 
// SD (And SPI)
#define SD_PMOS 16
#define CHIP_SELECT 5
// DS18B20 (And one wire)
#define DS18B20_PMOS 32   // 3V_sensor_switch (DS18 & DHT)
#define ONE_WIRE_BUS 25   // DS18_DATA (DS18B20) (also wifi)
// DHT
#define DHT22_PMOS 32     // 3V_sensor_switch (DS18 & DHT)
#define DHTPIN 33         // DHT_DATA
#define DHTTYPE DHT22
#define DHTPIN_FOR_PULLUP_DISABLE GPIO_NUM_33
// Battery (ADC)
#define BATTERY_MONITOR_PIN A0  // GPIO 36 / VP 
// I2S
#define I2S_NUM       (i2s_port_t)(0)
#define I2S_SCK_IO    (27)    // SCK, clock
#define I2S_WS_IO     (26)    // WS, choose LR
#define I2S_DI_IO     (14)    // SD, I2S data output 
#define MODE_R_PIN    (15)    // 15 pin weak pull-up / Set LR of that micorphone / High - R
#define MODE_L_PIN    (2)     // 2 pin weak pull-down / Set LR of that micorphone / Low - L
#define SOUND_PMOS    (13)    // 3V_I2S_SWITCH

#define PMOS_CHARGING_TIME 1000 // ms, 2500 for sometimes DS18(fake), caused by don't have value 


//====================== compile setting ===========================
// #define USE_DEEP_SLEEP         // use deep sleep mode
// #define KEEP_SET_TIME_FILE     // for test change RTC time by file
// #define SD_FAT16_32            // SD Use setting 
#define SD_EXFAT
// #define SD_HYBRID_FAT1632_EXFAT
// #define USE_FAKE_TIME          // use fake test time setting 
// #define HAND_MADE_ENV          // just for some small module test
#define HTML_ZH                // for language of html msg

//====================== debug setting ============================
// #define DS18B20_DEBUG
// #define DHT22_DEBUG
// #define RTC_DEBUG
/*--- myScheduler ---*/
// #define CHECK_DAY_CHANGE_DEBUG
// #define CHECK_IS_NEED_TO_RUN_TASK   // setting of "now, current index, task code, isExecuted?""
// #define RECORD_SOUND_DEBUG
#define RECORD_BATTERY_DEBUG
#define GET_DS18B20_TEMP_DEBUG
#define DHT_GET_TEMPERATURE_DEBUG   // value
#define DHT_GET_HUMIDITY_DEBUG      // value
// #define RTOS_DETIAL
// #define STACK_DEBUG
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
#define GOTOSLEEP_DEBUG
// #define FEED_DOG_DEBUG

/*=================== global variable ==================*/
const String SCHEDULE_FILE = "schedule.txt";
String today;
String systemLogPath;
String sensorDataPath;

// some status flag
bool isRecording = false;
bool isDHTRecording = false;
bool isDS18B20Recording = false;
bool isTaskAllLock = false;
bool isEvaluation = true;
int isRunningTask = 0;
bool isLowBattery = false;

// RTOS Mutex for some hardware
SemaphoreHandle_t xSemaphore_SD = xSemaphoreCreateMutex();
SemaphoreHandle_t xSemaphore_Temp_PMOS = xSemaphoreCreateMutex();

// a larger buffer befor save to SD (buffer level : DMA --> local function buffer --> 2* global larger buffer --> SD)
const int globalSDBufferByteSize = 512 * 16;  // 1 x uint8_t(byte) * 512  * 16 = 1 block of SD
uint8_t *currentAudioBuffer  = (uint8_t *)malloc(sizeof(uint8_t) * globalSDBufferByteSize);  
uint8_t *transmitAudioBuffer = (uint8_t *)malloc(sizeof(uint8_t) * globalSDBufferByteSize);

// init relative
RTC_DATA_ATTR bool isFirstBoot = true;
#define NO_SD_RETRY_LIMIT 5
RTC_DATA_ATTR int noSdRetry = 0;

/* OTA relative */
#define AP_TIMEOUT_MS 45000  // 45 sec
// This status will only keep when light/deep sleep (reset will clean the status)
RTC_DATA_ATTR bool isFirstCheckOTA = true;
bool isNeedToUpdate = false;


/* new task struct */
#define MIN_A_DAY 1440
RTC_DATA_ATTR uint8_t taskScheduleList[MIN_A_DAY] = {0};
RTC_DATA_ATTR int readTaskIndex = 0;       // read position, re-zero when day change (original is arrayReadIndex)
RTC_DATA_ATTR int readSettingIndex = 0;    // read position, re-zero when day change 
bool isCrossDay = false;
RTC_DATA_ATTR int recordSettingArrayMaxSize = 2;    // to know the malloc situation 
RTC_DATA_ATTR int recordSettingArrayUsedSize = 0;   // to know the malloc situation 
typedef struct recordSetting_t{
  int duration_time;
  char channel;
  float multiple;
}recordSetting;
RTC_DATA_ATTR recordSetting *recordSettingArray = (recordSetting*)malloc( sizeof(recordSetting) * recordSettingArrayMaxSize);

/* sleep relative */
int nextTaskPreserveTime_sec;

#endif








/*
Issue list : 
* [ok] conflict between <sdfat> and <espasyncwebserver> 
  * error : FS.h:30:25: error: invalid conversion from 'const char*' to 'oflag_t' {aka 'int'} [-fpermissive]  #define FILE_READ       >>>>>"r"<<<<<
  * same/similer issue : 
    * https://github.com/greiman/SdFat/issues/148
    * https://github.com/greiman/SdFat/issues/471
  * result : 
    * The file operate code conflict
      * change repersent type from FILE_READ (w) to O_RDONLY (int, read only)

* [ok] error of combination of SoftAP and AsyncWebServer
  * expectation : loop executing AsyncWebServer 
  * error/real situation : 
    * looks like it skiped the AsyncWebServer, then go to the softAP close function.
    * and it caused the web disconneted
  * result : 
    * Server.begin() of AsyncWebServer is an unblocking operate. Just use a while loop to stop it run away.

* [ok] 
  * error : 
    * Guru Meditation Error: Core 1 panic'ed (LoadProhibited). Exception was unhandled.
    * and it caused the web disconneted
  * guess
    * looks like we use the RTC lib without init it?
  * result
    * Just like we guest. We init RTC first, then it works.

* [ok]
  * error : 
    * RTC show correct in hand-wiring ver, but show 2165-00-00 00:165:00 in PCB one
  * guess : 
    * There has a MOS switch on PCB noard 
  * result :
    * Forgot to open that switch

*/