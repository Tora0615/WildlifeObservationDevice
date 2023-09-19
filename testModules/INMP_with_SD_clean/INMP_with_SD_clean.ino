// SD
#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"
#define SPI_SPEED SD_SCK_MHZ(15)
#define CHIP_SELECT 5

// SD_FAT_TYPE 3 (FAT16/FAT32 and exFAT)
SdFs sd;
FsFile soundFile;

void SDInit(){
  if (!sd.begin(CHIP_SELECT, SPI_SPEED)) {
    Serial.println("SD card init error");
    while(1){
      delay(1000);
    }
  }
}





#include "MEMS_INMP441.h"

#define SAMPLE_RATE     (38000)
#define DATA_BIT        (16)

// Left : 0 / Right : 1 / stereo : 2
#define CHANNEL_LEFT     (0)
#define CHANNEL_RIGHT    (1)
#define CHANNEL_STEREO   (2)

const int recordSeconds = 180;  
const char filename[] = "/180.wav";
const int numOfData = 512; // size of each collection
byte header[44];  
byte dataBuffer[numOfData];

INMP441 microphone(I2S_SCK_IO, I2S_WS_IO, I2S_DI_IO);

int duration;

void setup(){
  Serial.begin(115200);
  SDInit();


  // remove old file 
  if (sd.exists(filename)){
    sd.remove(filename);
  }
  // create a new file 
  if (!soundFile.open(filename, O_WRONLY | O_CREAT )) {     // open need char array, not string. So use c_str to convert
    Serial.println(" --> open file failed");
  }

  while(microphone.begin(SAMPLE_RATE, DATA_BIT, CHANNEL_STEREO) != 0){
    Serial.println(" I2S init failed");
  }
  Serial.println("I2S init success");
  microphone.createWavHeader(header, recordSeconds, SAMPLE_RATE, DATA_BIT, CHANNEL_STEREO);
  soundFile.write(header, 44);



  Serial.println("start");
  duration = millis();
  int I2s_duration = 0;
  int writeFile_duration = 0;
  int wavDataSize = recordSeconds * microphone.bytePerSecond; 
  for (int j = 0; j < wavDataSize/numOfData; ++j) {
    // read a size
    int tempreadtime = millis();
    microphone.read(dataBuffer, numOfData);
    I2s_duration += millis() - tempreadtime;
    // oprate this size
    // for(uint32_t i = 0; i < numOfData/2; i++){
    //   int16_t originalSound = dataBuffer[(i*2)] + ((dataBuffer[(i*2)+1]) * 256);  // 8 bit 平移
    //   // larger than the background val, than mulitiply the sound

    //   int16_t calculated = originalSound * volume_double;
    //   // limit the min and max
    //   if (abs(calculated) > 32767 ){
    //     if (originalSound > 0)
    //       calculated = 32767;
    //     else
    //       calculated = -32767;
    //   }
    //   dataBuffer[i*2] =  (byte)(calculated & 0xFF);
    //   dataBuffer[i*2 + 1] = (byte)((calculated >> 8) & 0xFF);

    // }
    // write this size
    int tempwritefiletime = millis();
    soundFile.write((uint8_t*)dataBuffer, numOfData);
    writeFile_duration += millis() - tempwritefiletime;
  }
  Serial.println("use time : " + String (millis() - duration));
  Serial.println("I2s_duration : " + String (I2s_duration));
  Serial.println("writeFile_duration : " + String (writeFile_duration));
  soundFile.close();
  Serial.println("finish");


  microphone.end();
}


void loop(){



}