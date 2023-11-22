#include "setting.h"

#ifndef MYINMP441_H
#define MYINMP441_H

// Left : 0 / Right : 1 / stereo : 2
#define CHANNEL_LEFT     (0)
#define CHANNEL_RIGHT    (1)
#define CHANNEL_STEREO   (2)

#include "MEMS_INMP441.h"
INMP441 microphone(I2S_SCK_IO, I2S_WS_IO, I2S_DI_IO);
TaskHandle_t tSdTransmitHandler;

// use to save the signal of buffer full & can start to save to SD
int bufferIndex = 0;
char *_filenameWithPath;


// execute a single byte
void exportSingleData(uint8_t input){
  currentAudioBuffer[bufferIndex] = input;
  bufferIndex += 1;
  if(bufferIndex == globalSDBufferByteSize){
    // switch the pointer of memery space
    std::swap(currentAudioBuffer, transmitAudioBuffer);
    // re-zero
    bufferIndex = 0;
    // send a signal via RTOS to write SD
    xTaskNotify(tSdTransmitHandler, 1, eIncrement);
  }
}

// execute local buffer 
void exportBufferData(uint8_t* dataBuffer, int numOfData){
  isRunningTask += 1;
  // there are n count (numOfData) of uint8_t(byte)
  for (int i = 0; i < numOfData; i++) {
    exportSingleData(dataBuffer[i]);
  }
  isRunningTask -= 1;
}

void recordWithDualChannel(int recordSeconds, char *filenameWithPath, float gain_ratio){
  // isRunningTask += 1;
  _filenameWithPath = filenameWithPath;

  #ifdef INMP_COMMON_DEBUG
    Serial.println("Dual record");
  #endif

  // RESET WATCHDOG
  vTaskDelay(10);

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
    #ifdef SD_USE_NORMAL
      FsFile soundFile;
    #else
      ExFile soundFile;
    #endif

    // lock SD
    if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
      /*--- execute part ---*/
      while(microphone.begin(SAMPLE_RATE, DATA_BIT, CHANNEL) != 0){
        Serial.println(" I2S init failed");
      }
      #ifdef INMP_COMMON_DEBUG
        Serial.println("I2S init success");
      #endif

      // create header data
      microphone.createWavHeader(header, recordSeconds, SAMPLE_RATE, DATA_BIT, CHANNEL);
      if (!soundFile.open(filenameWithPath, O_WRONLY | O_CREAT )) {     // open need char array, not string. So use c_str to convert
        Serial.println(" --> open file failed, dual channel , header");
      }
      soundFile.write(header, 44);
      soundFile.close(); 
    }xSemaphoreGive( xSemaphore_SD );


    // RESET WATCHDOG
    vTaskDelay(10);

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
    unsigned long intervalTotalLen = recordSeconds * 1000.0 * 1000.0 / loopCount;
    #ifdef RECORD_TIME_DEBUG
      Serial.print("interval of each task (sec) : ");
      Serial.println(intervalTotalLen/1000000.0, 8);
      Serial.println("loopCount : " + String(loopCount));
    #endif
    for (int j = 0; j < loopCount; ++j) {

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
      exportBufferData((uint8_t*)dataBuffer, numOfData);
      #ifdef RECORD_TIME_DEBUG
        writeFile_duration += millis() - tempwritefiletime;
      #endif

      // busy wait
      #ifdef RECORD_TIME_DEBUG
        int busyWaitTime = micros();
      #endif
      // jump out to other tasks with interger delay ms time first 
      vTaskDelay(intervalTotalLen / 1000 / portTICK_PERIOD_MS);           // us to ms. e.g. : 5805 uS -> 5 mS
      while (micros() - busy_wait_intervalTimer < intervalTotalLen) {     // e.g. remain 805 uS
        // Do nothing, just busy wait the remaining time. 
      }
      #ifdef RECORD_TIME_DEBUG
        busyWait_duration += micros() - busyWaitTime;
      #endif
    }
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
  isRunningTask -= 1;
}



void recordWithMonoChannel(int recordSeconds, char *filenameWithPath, float gain_ratio, uint8_t CHANNEL){

  _filenameWithPath = filenameWithPath;
  #ifdef INMP_COMMON_DEBUG
    Serial.println("Mono record");
  #endif

  // RESET WATCHDOG
  vTaskDelay(10);

  if(!isRecording){
    // flag switch
    isRecording = !isRecording; 

    /*--- setting part ---*/
    // record setting 
    const uint16_t SAMPLE_RATE = 44100;
    const uint8_t  DATA_BIT = 16;
    float volume_gain = 64 * gain_ratio;  // 32 ~ 128 best
    const int numOfData = 512; // size of each collection
    byte header[44];  
    byte dataBuffer[numOfData];

    // SD setting 
    #ifdef SD_USE_NORMAL
      FsFile soundFile;
    #else
      ExFile soundFile;
    #endif

    // lock SD
    if(xSemaphoreTake( xSemaphore_SD, portMAX_DELAY ) == pdTRUE){
      /*--- execute part ---*/
      while(microphone.begin(SAMPLE_RATE, DATA_BIT, CHANNEL) != 0){
        Serial.println(" I2S init failed");
      }
      #ifdef INMP_COMMON_DEBUG
        Serial.println("I2S init success");
      #endif

      // create header dataPsho
      microphone.createWavHeader(header, recordSeconds, SAMPLE_RATE, DATA_BIT, CHANNEL);
      if (!soundFile.open(filenameWithPath, O_WRONLY | O_CREAT )) {     // open need char array, not string. So use c_str to convert
        Serial.println(" --> open file failed, mono channel , header");
      }
      soundFile.write(header, 44);
      soundFile.close();
    }xSemaphoreGive( xSemaphore_SD );


    // RESET WATCHDOG
    vTaskDelay(10);

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
    unsigned long intervalTotalLen = recordSeconds * 1000.0 * 1000.0 / loopCount;
    #ifdef RECORD_TIME_DEBUG
      Serial.print("interval of each task (sec) : ");
      Serial.println(intervalTotalLen/1000000.0, 8);
      Serial.println("loopCount : " + String(loopCount));
    #endif
    for (int j = 0; j < loopCount; ++j) {
      
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
      exportBufferData((uint8_t*)dataBuffer, numOfData);
      #ifdef RECORD_TIME_DEBUG
        writeFile_duration += millis() - tempwritefiletime;
      #endif

      // busy wait
      #ifdef RECORD_TIME_DEBUG
        int busyWaitTime = micros();
      #endif
      // jump out to other tasks with interger delay ms time first 
      vTaskDelay(intervalTotalLen / 1000 / portTICK_PERIOD_MS);  // us to ms. e.g. : 5805 us -> 5 ms
      while (micros() - busy_wait_intervalTimer < intervalTotalLen) {
        // Do nothing, just busy wait the remaining time. 
      }
      #ifdef RECORD_TIME_DEBUG
        busyWait_duration += micros() - busyWaitTime;
      #endif
    }
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
  isRunningTask -= 1;
}

#endif