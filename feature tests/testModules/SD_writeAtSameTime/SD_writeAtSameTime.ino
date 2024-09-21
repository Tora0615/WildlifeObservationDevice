#include <TaskScheduler.h>
#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#define SPI_SPEED SD_SCK_MHZ(4)
#define CHIP_SELECT 5
#define F_NAME "onlyFile.txt"

const int8_t DISABLE_CHIP_SELECT = -1;

// SD_FAT_TYPE 3 (FAT16/FAT32 and exFAT)
SdFs sd;
FsFile onlyFile;


// Callback methods prototypes
void t1Callback();
void t2Callback();
void t3Callback();

//Tasks
Task t4();
Task t1(1000, 3, &t1Callback);
Task t2(1000, 3, &t2Callback);
Task t3(1000, 3, &t3Callback);

Scheduler runner;

/*
實測可以好幾個thread開同一個檔案來寫入
*/



void t1Callback() {
  if (!onlyFile.open(F_NAME, O_WRONLY | O_CREAT | O_APPEND)) {     // open need char array, not string. So use c_str to convert
    Serial.println(" --> task01 : open " + String(F_NAME) + " failed");
  }else{
    Serial.println(" --> task01 : " + String(F_NAME) + " opened");
    onlyFile.println("this is the msg from task01");
  }
}

void t2Callback() {
  if (!onlyFile.open(F_NAME, O_WRONLY | O_CREAT | O_APPEND)) {     // open need char array, not string. So use c_str to convert
    Serial.println(" --> task02 : open " + String(F_NAME) + " failed");
  }else{
    Serial.println(" --> task02 : " + String(F_NAME) + " opened");
    onlyFile.println("this is the msg from task02");
  }
}

void t3Callback() {
  if (!onlyFile.open(F_NAME, O_WRONLY | O_CREAT | O_APPEND)) {     // open need char array, not string. So use c_str to convert
    Serial.println(" --> task03 : open " + String(F_NAME) + " failed");
  }else{
    Serial.println(" --> task03 : " + String(F_NAME) + " opened");
    onlyFile.println("this is the msg from task03");
  }
}

void setup () {
  Serial.begin(115200);

  // init SD mod
  if (!sd.begin(CHIP_SELECT, SPI_SPEED)) {
    Serial.println("SD card init error");
    while(1){
      delay(1000);
    }
  }


  

  Serial.println("Scheduler TEST");
  runner.init();
  Serial.println("Initialized scheduler");
  
  runner.addTask(t1);
  Serial.println("added t1");
  runner.addTask(t2);
  Serial.println("added t2");
  runner.addTask(t3);
  Serial.println("added t3");

  delay(1000);
  
  t1.enable();
  Serial.println("Enabled t1");
  t2.enable();
  Serial.println("Enabled t2");
  t3.enable();
  Serial.println("Enabled t3");
}


void loop () {
  runner.execute();
}