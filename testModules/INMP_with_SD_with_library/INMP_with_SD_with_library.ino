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


#include "DFrobot_MSM261.h"

#define SAMPLE_RATE     (48000)
#define DATA_BIT        (16)

#define I2S_SCK_IO      (2) // (25)   // SCK, clock
#define I2S_WS_IO       (4) // (16)   // WS, choose LR
#define I2S_DI_IO       (13) // (27)   // SD, I2S data output 
#define MODE_R_PIN      (27)    // Set LR of that micorphone / High - R
#define MODE_L_PIN      (26)    // Set LR of that micorphone / Low - L
#define SOUND_PMOS            (14)

const int record_time = 10;  // 采样时间
const char filename[] = "/sound.wav";//保存的文件名称

const int headerSize = 44;
const int byteRate = 176400;//一秒采集的字节数量 计算方式：采样速率x声道数量x数据位数/8
const int waveDataSize = record_time * byteRate;//10秒采样字节
const int numCommunicationData = 512;//每次采集的数据量
byte header[headerSize];//WAVE文件的文件头
char communicationData[numCommunicationData];

const int volume_double = 20;
const int volume_noise_limit = 100; //噪音音量值


DFRobot_Microphone microphone(I2S_SCK_IO, I2S_WS_IO, I2S_DI_IO);
File file;

void setup() {
  Serial.begin(115200);

  pinMode(MODE_L_PIN,OUTPUT);
  pinMode(MODE_R_PIN,OUTPUT);
  pinMode(SOUND_PMOS,OUTPUT);

  Serial.println("Turn off mic power");
  digitalWrite(SOUND_PMOS,HIGH);   // Turn off, gpio default is low -> this will let mic ON
  
  SDInit();
  if (!soundFile.open(filename, O_WRONLY | O_CREAT )) {     // open need char array, not string. So use c_str to convert
    Serial.println(" --> open file failed");
  }
  
  
  digitalWrite(MODE_L_PIN,LOW);//将麦克风配置为接收左声道数据
  digitalWrite(MODE_R_PIN,HIGH);//将麦克风配置为接收右声道数据
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
  microphone.createWavHeader(header, waveDataSize, 44100, 2, byteRate, 4);
  
  //将wave文件的头写入文件中
  soundFile.write(header, headerSize);

  
  Serial.println("Turn on mic power");
  digitalWrite(SOUND_PMOS,LOW);    // Turn on 
  delay(1000);

  Serial.println("start");
  for (int j = 0; j < waveDataSize/numCommunicationData; ++j) {
    // read a size
    microphone.read(communicationData, numCommunicationData);
    // oprate this size
    for(uint32_t i = 0; i < numCommunicationData/2; i++){
      int16_t originalSound = communicationData[(i*2)] + (communicationData[(i*2)+1]) * 256;  // 8 bit 平移
      // larger than the background val, than mulitiply the sound
      if(abs(originalSound) > volume_noise_limit){
        int16_t calculated = originalSound * volume_double;
        // limit the min and max
        if (abs(calculated) > 32767 ){
          if (originalSound > 0)
            calculated = 32767;
          else
            calculated = -32767;
        }
        communicationData[i*2] =  (byte)(calculated & 0xFF);
        communicationData[i*2 + 1] = (byte)((calculated >> 8) & 0xFF);
      }
    }
    // write this size
    soundFile.write((uint8_t*)communicationData, numCommunicationData);
  }
  soundFile.close();
  Serial.println("finish");


  Serial.println("Turn off mic power");
  digitalWrite(SOUND_PMOS,HIGH);   // Turn off, gpio default is low -> this will let mic ON
  digitalWrite(MODE_R_PIN,LOW);     //防止漏電
  i2s_driver_uninstall(I2S_NUM);
}

void loop() {
}
