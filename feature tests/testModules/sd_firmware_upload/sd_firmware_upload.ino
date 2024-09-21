#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#include <Update.h>
#define SPI_SPEED SD_SCK_MHZ(10)
#define CHIP_SELECT 5
const int8_t DISABLE_CHIP_SELECT = -1;


SdExFat sd;
ExFile updateBin;
char *BINPATH = "update.bin";
char *UPDATEDBINPATH = "updated.bin";


void SDInit(){
  if(!sd.begin(CHIP_SELECT, SPI_SPEED)){
    Serial.println("SD card init error : NO SD card");
    while(1){
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  SDInit();
  updateBin.open(BINPATH);
  if(updateBin){
    if(!updateBin.isDirectory()){
      size_t updateSize = updateBin.size();
      if(updateSize <= 0){
        Serial.println("File empty");
      }else{
        Serial.println("update.bin size : " + String(updateSize));
      }

      if(!Update.begin(updateSize)){
        Serial.println("Don't have enough space to update");
      }
      size_t written = Update.writeStream(updateBin);
      Serial.println("Have wrote " + String(written) + " bytes into OTA section.");

      if(!Update.end()){
        Serial.println("update failed");
      }

      updateBin.close();
      sd.rename(BINPATH, UPDATEDBINPATH);
      ESP.restart();

    }else{
      Serial.println("update.bin is not a file");
    }
  }
  Serial.println("Uploaded by SD");
}

void loop() {
  // put your main code here, to run repeatedly:

}
