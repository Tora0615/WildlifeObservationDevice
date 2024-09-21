#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#define SPI_SPEED SD_SCK_MHZ(4)
#define CHIP_SELECT 5

const int8_t DISABLE_CHIP_SELECT = -1;

// SD_FAT_TYPE 3 (FAT16/FAT32 and exFAT)
SdFs sd;
String name[10];

void setup() {
  Serial.begin(115200);

  // init SD mod
  if (!sd.begin(CHIP_SELECT, SPI_SPEED)) {
    Serial.println("SD card init error");
    while(1){
      delay(1000);
    }
  }

  // create 10 name
  for (int i=0; i<10; i++){
    name[i] = "test" + String(i)+".txt";
  }

  /* different FsFile */
  /* 可以同時開 10 個檔案, 只要 FsFile 變數沒有 call close, 就會繼續存在*/
  FsFile file[10];
  for (int i=0; i<10; i++){
    if (!file[i].open(name[i].c_str(), O_WRONLY | O_CREAT | O_APPEND)) {     // open need char array, not string. So use c_str to convert
      Serial.println(" --> open " + name[i] + " failed");
    }
    Serial.println(" --> " + name[i] + " opened");
    file[i].println("this is : " + name[i]);
  }

  for (int i=0; i<5; i++){
    file[i*2].close();
  }
  Serial.println("close 5 file");

  for (int i=0; i<5; i++){
    file[i].println("here");
  }

  for (int i=0; i<5; i++){
    file[i].close();
  }
  Serial.println("close another 5 file");
}

void loop() {
  // put your main code here, to run repeatedly:

}
