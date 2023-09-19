/*!
 * @file microphoneRecordsToSDCard.ino
 * @brief 这是一个麦克风录音的例程，使用16bit 44.1kHz 单声道录取声音，运行用例将
 * @n 会录制20秒的声音到SD卡中
 * @copyright  Copyright (c) 2022 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [TangJie](jie.tang@dfrobot.com)
 * @version V1.0
 * @date 2022-02-24
 * @url https://github.com/DFRobot/DFRobot_MSM261
 */

#include <driver/i2s.h>

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


#include "myMEMS.h"

#define SAMPLE_RATE     (38000)
#define DATA_BIT        (16)
#define CHANNEL         (2)
#define I2S_SCK_IO      (27)    // SCK, clock
#define I2S_WS_IO       (26)    // WS, choose LR
#define I2S_DI_IO       (14)    // SD, I2S data output 
#define MODE_R_PIN      (15)    // 15 pin weak pull-up / Set LR of that micorphone / High - R
#define MODE_L_PIN      (2)     // 2 pin weak pull-down / Set LR of that micorphone / Low - L
#define SOUND_PMOS      (4)

const int record_time = 1800;  // 采样时间
const char filename[] = "/1800.wav";//保存的文件名称

const int headerSize = 44;
const int byteRate = SAMPLE_RATE * CHANNEL * DATA_BIT / 8;   //192000;//一秒采集的字节数量 计算方式：采样速率x声道数量x数据位数/8
const int waveDataSize = record_time * byteRate;//10秒采样字节
const int numCommunicationData = 512;//每次采集的数据量
byte header[headerSize];//WAVE文件的文件头
char communicationData[numCommunicationData];

float volume_double = 2.5;
// const int volume_noise_limit = 100; //噪音音量值

int duration;

DFRobot_Microphone microphone(I2S_SCK_IO, I2S_WS_IO, I2S_DI_IO);
File file;

void setup() {
  Serial.begin(115200);

  pinMode(MODE_L_PIN,OUTPUT);
  pinMode(MODE_R_PIN,OUTPUT);
  pinMode(SOUND_PMOS,OUTPUT);
  pinMode(I2S_DI_IO,OUTPUT);

  Serial.println("Turn off mic power");
  digitalWrite(SOUND_PMOS,HIGH);   // Turn off, gpio default is low -> this will let mic ON
  digitalWrite(I2S_DI_IO,LOW);
  
  SDInit();
  // remove old file 
  if (sd.exists(filename)){
    sd.remove(filename);
  }

  // create a new file 
  if (!soundFile.open(filename, O_WRONLY | O_CREAT )) {     // open need char array, not string. So use c_str to convert
    Serial.println(" --> open file failed");
  }
  
  
  digitalWrite(MODE_L_PIN,LOW); //将麦克风配置为接收左声道数据
  digitalWrite(MODE_R_PIN,HIGH); //将麦克风配置为接收右声道数据
  while(microphone.begin(SAMPLE_RATE, DATA_BIT) != 0){
    Serial.println(" I2S init failed");
  }
  Serial.println("I2S init success");

  /*
   * @brief 构建WAVE文件的头
   * @param header 构建WAVE的文件
   * @param totalDataLen 需要写入的数据长度
   * @param longSampleRate 采样频率
   * @param channels 声道数量
   * @param byteRate 字节速率 计算方式：采样速率x声道数量x数据位数/8
   * @param blockAlign 块对齐方式 计算方式：声道数量x数据位数/8
  void createWavHeader(byte* header, int totalDataLen, int longSampleRate, uint8_t channels, int byteRate, uint8_t blockAlign);*/
  microphone.createWavHeader(header, waveDataSize, SAMPLE_RATE, CHANNEL, byteRate, CHANNEL*DATA_BIT/8);
  
  //将wave文件的头写入文件中
  soundFile.write(header, headerSize);

  
  Serial.println("Turn on mic power");
  digitalWrite(SOUND_PMOS,LOW);    // Turn on 
  delay(2000);

  Serial.println("start");
  duration = millis();
  int I2s_duration = 0;
  int writeFile_duration = 0;
  unsigned long busyWait_duration = 0;
  float tempJ = 0;
  unsigned long intervalTimer;
  int loopTime = waveDataSize/numCommunicationData;
  int fix_interval_count = 3;

  Serial.print("interval(s) : ");
  unsigned long intervalMicros = record_time * 1000.0 * 1000.0 / loopTime;
  Serial.println(intervalMicros/1000000.0, 8);
  Serial.println("times count : " + String(loopTime));

  for (int j = 0; j < loopTime; ++j) {

    // print record detail
    // if( ((j - tempJ)/(loopTime))*100.0 > 5) {
    //   Serial.println( ((float)j/(loopTime)) *100 );
    //   tempJ = j;
    // }

    intervalTimer = micros(); 

    // read a size
    int tempreadtime = millis();
    microphone.read(communicationData, numCommunicationData);
    I2s_duration += millis() - tempreadtime;

    // oprate this size
    // for(uint32_t i = 0; i < numCommunicationData/2; i++){
    //   int16_t originalSound = communicationData[(i*2)] + ((communicationData[(i*2)+1]) * 256);  // 8 bit 平移
    //   // larger than the background val, than mulitiply the sound

    //   int16_t calculated = originalSound * volume_double;
    //   // limit the min and max
    //   if (abs(calculated) > 32767 ){
    //     if (originalSound > 0)
    //       calculated = 32767;
    //     else
    //       calculated = -32767;
    //   }
    //   communicationData[i*2] =  (byte)(calculated & 0xFF);
    //   communicationData[i*2 + 1] = (byte)((calculated >> 8) & 0xFF);

    // }

    // write this size
    int tempwritefiletime = millis();
    soundFile.write((uint8_t*)communicationData, numCommunicationData);
    writeFile_duration += millis() - tempwritefiletime;

    int busyWaitTime = micros();
    while ( micros() - intervalTimer < intervalMicros && j % fix_interval_count == 0){
      // delayMicroseconds(2);  
    }
    busyWait_duration += micros() - busyWaitTime;
    // Serial.println((micros() - intervalTimer)/1000000.0, 8);
  }
  Serial.println("use time : " + String ((millis() - duration)/1000.0, 4));
  Serial.println("I2s_duration : " + String (I2s_duration/1000.0, 4));
  Serial.println("writeFile_duration : " + String (writeFile_duration/1000.0, 4));
  Serial.println("busyWait_duration : " + String (busyWait_duration/1000000.0, 4));
  soundFile.close();
  Serial.println("finish");


  Serial.println("Turn off mic power");
  digitalWrite(SOUND_PMOS,HIGH);   // Turn off, gpio default is low -> this will let mic ON
  digitalWrite(MODE_R_PIN,LOW);     //防止漏電
  digitalWrite(I2S_DI_IO,LOW);
  i2s_driver_uninstall(I2S_NUM);
}

void loop() {
}
