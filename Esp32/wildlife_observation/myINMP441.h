#include "setting.h"

#ifndef MYINMP441_H
#define MYINMP441_H

// Left : 0 / Right : 1 / stereo : 2
#define CHANNEL_LEFT     (0)
#define CHANNEL_RIGHT    (1)
#define CHANNEL_STEREO   (2)

#include "MEMS_INMP441.h"
INMP441 microphone(I2S_SCK_IO, I2S_WS_IO, I2S_DI_IO);





#include "soc/timer_group_reg.h"
#include "soc/timer_group_struct.h"
void feedDogOfThisCore(){
  // feed dog 0
  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE; // write enable
  TIMERG0.wdt_feed = 1;                     // feed dog
  TIMERG0.wdt_wprotect = 0;                 // write protect
}

void recordWithDualChannel(int recordSeconds, char *filenameWithPath, float gain_ratio){
  feedDogOfThisCore();

  if(!isRecording){
    // flag switch
    isRecording = !isRecording; 

    /*--- setting part ---*/
    // record setting 
    const uint16_t SAMPLE_RATE = 38000;
    const uint8_t  DATA_BIT = 16;
    const uint8_t  CHANNEL = CHANNEL_STEREO;
    float volume_gain = 64 * gain_ratio;  // 32 ~ 128 best
    const int numOfData = 512; // size of each collection
    byte header[44];  
    byte dataBuffer[numOfData];
    // SD setting 
    FsFile soundFile;


    /*--- execute part ---*/
    // remove old file 
    // if (sd.exists(filenameWithPath)){
    //   sd.remove(filenameWithPath);
    // }
    // create a new file 
    

    while(microphone.begin(SAMPLE_RATE, DATA_BIT, CHANNEL) != 0){
      Serial.println(" I2S init failed");
    }
    Serial.println("I2S init success");

    // create header data
    microphone.createWavHeader(header, recordSeconds, SAMPLE_RATE, DATA_BIT, CHANNEL);

    // lock sd 
    if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
      if (!soundFile.open(filenameWithPath, O_WRONLY | O_CREAT )) {     // open need char array, not string. So use c_str to convert
        Serial.println(" --> open file failed");
      }
      soundFile.write(header, 44);
      soundFile.close();
    }xSemaphoreGive( xSemaphore_SD );
    

    // RESET WATCHDOG
    feedDogOfThisCore();



    Serial.println("start");
    #ifdef RECORD_TIME_DEBUG
      int duration = millis();
      int I2s_duration = 0;
      int writeFile_duration = 0;
      unsigned long busyWait_duration = 0;
    #endif
    #ifdef PERCENTAGE_DEBUG
      float tempJ = 0;
    #endif


    int wavDataSize = recordSeconds * microphone.bytePerSecond; 
    int loopCount = wavDataSize / numOfData;
    int busy_wait_fix_factor = 4;
    unsigned long intervalTotalLen = recordSeconds * 1000.0 * 1000.0 / loopCount;
    #ifdef RECORD_TIME_DEBUG
      Serial.print("interval of each task (sec) : ");
      Serial.println(intervalTotalLen/1000000.0, 8);
      Serial.println("loopCount : " + String(loopCount));
    #endif
    for (int j = 0; j < loopCount; ++j) {
      feedDogOfThisCore();

      // print process percentage 
      #ifdef PERCENTAGE_DEBUG
        if( ((j - tempJ)/(loopCount))*100.0 > 5) {
          Serial.println( ((float)j/(loopCount)) *100 );
          tempJ = j;
        }
      #endif

      // for busy wait
      unsigned long busy_wait_intervalTimer = micros(); 

      // read a size
      #ifdef RECORD_TIME_DEBUG
        int tempreadtime = millis();
      #endif
      microphone.read(dataBuffer, numOfData);
      #ifdef RECORD_TIME_DEBUG
        I2s_duration += millis() - tempreadtime;
      #endif

      // clean noise of not use side
      // DualChannel no need

      // let it louder of this size
      microphone.louder(dataBuffer, numOfData, volume_gain);

      // write this size
      #ifdef RECORD_TIME_DEBUG
        int tempwritefiletime = millis();
      #endif

      // lock sd 
      if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
        if (!soundFile.open(filenameWithPath, O_WRONLY | O_CREAT | O_APPEND)) {     // open need char array, not string. So use c_str to convert
          Serial.println(" --> open file failed during recording");
        }
        soundFile.write((uint8_t*)dataBuffer, numOfData);
        soundFile.close();
      }xSemaphoreGive( xSemaphore_SD );

      #ifdef RECORD_TIME_DEBUG
        writeFile_duration += millis() - tempwritefiletime;
      #endif

      // busy wait
      #ifdef RECORD_TIME_DEBUG
        int busyWaitTime = micros();
      #endif
      // while (micros() - busy_wait_intervalTimer < intervalTotalLen && j % busy_wait_fix_factor == 0) {}
      #ifdef RECORD_TIME_DEBUG
        busyWait_duration += micros() - busyWaitTime;
      #endif
    }
    // soundFile.close();
    microphone.end();
    #ifdef INMP_COMMON_DEBUG
      Serial.println("finish");
    #endif

    #ifdef RECORD_TIME_DEBUG
      Serial.println("use time : " + String ((millis() - duration)/1000.0, 4));
      Serial.println("I2s_duration : " + String (I2s_duration/1000.0, 4));
      Serial.println("writeFile_duration : " + String (writeFile_duration/1000.0, 4));
      Serial.println("busyWait_duration : " + String (busyWait_duration/1000000.0, 4));
    #endif

    // flag switch
    isRecording = !isRecording; 
  }
  // if INMP was occupied 
  else{
    writeMsgToPath(systemLogPath, "Now INMP sensor is recording, skip this task");
  }
}


void recordWithMonoChannel(int recordSeconds, char *filenameWithPath, float gain_ratio, uint8_t CHANNEL){
  feedDogOfThisCore();
  if(!isRecording){
    // flag switch
    isRecording = !isRecording; 

    /*--- setting part ---*/
    const uint16_t SAMPLE_RATE = 44100;
    const uint8_t  DATA_BIT = 16;
    float volume_gain = 64 * gain_ratio;  // 32 ~ 128 best
    const int numOfData = 512; // size of each collection
    byte header[44];  
    byte dataBuffer[numOfData];
    // SD setting 
    FsFile soundFile;

    /*--- execute part ---*/
    // remove old file 
    if (sd.exists(filenameWithPath)){
      sd.remove(filenameWithPath);
    }
    // create a new file 
    if (!soundFile.open(filenameWithPath, O_WRONLY | O_CREAT )) {     // open need char array, not string. So use c_str to convert
      #ifdef INMP_COMMON_DEBUG
        Serial.println(" --> open file failed");
      #endif
    }

    while(microphone.begin(SAMPLE_RATE, DATA_BIT, CHANNEL) != 0){
      #ifdef INMP_COMMON_DEBUG
        Serial.println(" I2S init failed");
      #endif
    }
    #ifdef INMP_COMMON_DEBUG
      Serial.println("I2S init success");
    #endif
    microphone.createWavHeader(header, recordSeconds, SAMPLE_RATE, DATA_BIT, CHANNEL);
    soundFile.write(header, 44);
    feedDogOfThisCore();

    #ifdef INMP_COMMON_DEBUG
      Serial.println("start");
    #endif

    #ifdef RECORD_TIME_DEBUG
      int duration = millis();
      int I2s_duration = 0;
      int writeFile_duration = 0;
      unsigned long busyWait_duration = 0;
    #endif
    #ifdef PERCENTAGE_DEBUG
      float tempJ = 0;
    #endif


    int wavDataSize = recordSeconds * microphone.bytePerSecond; 
    int loopCount = wavDataSize / numOfData;
    int busy_wait_fix_factor = 1;
    unsigned long intervalTotalLen = recordSeconds * 1000.0 * 1000.0 / loopCount;
    #ifdef RECORD_TIME_DEBUG
      Serial.print("interval of each task (sec) : ");
      Serial.println(intervalTotalLen/1000000.0, 8);
      Serial.println("loopCount : " + String(loopCount));
    #endif
    for (int j = 0; j < loopCount; ++j) {
      feedDogOfThisCore();

      // print process percentage 
      #ifdef PERCENTAGE_DEBUG
        if( ((j - tempJ)/(loopCount))*100.0 > 5) {
          Serial.println( ((float)j/(loopCount)) *100 );
          tempJ = j;
        }
      #endif

      // for busy wait
      unsigned long busy_wait_intervalTimer = micros(); 

      // read a size
      #ifdef RECORD_TIME_DEBUG
        int tempreadtime = millis();
      #endif
      microphone.read(dataBuffer, numOfData);
      #ifdef RECORD_TIME_DEBUG
        I2s_duration += millis() - tempreadtime;
      #endif

      // clean noise of not use side
      // TODO

      // let it louder of this size
      microphone.louder(dataBuffer, numOfData, volume_gain);

      // write this size
      #ifdef RECORD_TIME_DEBUG
        int tempwritefiletime = millis();
      #endif
      soundFile.write((uint8_t*)dataBuffer, numOfData);
      #ifdef RECORD_TIME_DEBUG
        writeFile_duration += millis() - tempwritefiletime;
      #endif

      // busy wait
      #ifdef RECORD_TIME_DEBUG
        int busyWaitTime = micros();
      #endif
      while (micros() - busy_wait_intervalTimer < intervalTotalLen && j % busy_wait_fix_factor == 0) {}
      #ifdef RECORD_TIME_DEBUG
        busyWait_duration += micros() - busyWaitTime;
      #endif
    }
    soundFile.close();
    microphone.end();
    #ifdef INMP_COMMON_DEBUG
      Serial.println("finish");
    #endif

    #ifdef RECORD_TIME_DEBUG
      Serial.println("use time : " + String ((millis() - duration)/1000.0, 4));
      Serial.println("I2s_duration : " + String (I2s_duration/1000.0, 4));
      Serial.println("writeFile_duration : " + String (writeFile_duration/1000.0, 4));
      Serial.println("busyWait_duration : " + String (busyWait_duration/1000000.0, 4));
    #endif

    // flag switch
    isRecording = !isRecording; 
  }
  // if INMP was occupied 
  else{
    writeMsgToPath(systemLogPath, "Now INMP sensor is recording, skip this task");
  }
}


#endif