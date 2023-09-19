#include "MEMS_INMP441.h"

INMP441::INMP441(uint8_t bckIoNum, uint8_t wsIoNum, uint8_t dInNum){
  _bckIoNum = bckIoNum;
  _wsIoNum  = wsIoNum;
  _dInNum   = dInNum;

  // set pin 
  pinMode(MODE_L_PIN,OUTPUT);
  pinMode(MODE_R_PIN,OUTPUT);
  pinMode(SOUND_PMOS,OUTPUT);
  pinMode(I2S_DI_IO,OUTPUT);

  // power off  
  #ifdef INMP_DEBUG
    Serial.println("Turn off mic power");
  #endif
  digitalWrite(SOUND_PMOS,HIGH);   // Turn off, gpio default is low -> this will let mic ON
  digitalWrite(I2S_DI_IO,LOW);
}


uint8_t INMP441::begin(uint16_t sampleRate, uint8_t bit, uint8_t channelSetting){
  // power on INMP
  Serial.println("Turn on mic power");
  digitalWrite(SOUND_PMOS,LOW);    // Turn on 

  // set INMP left and right
  Serial.println("Set INMP L/R");
  digitalWrite(MODE_L_PIN, LOW);  // set left microphone with low power level
  digitalWrite(MODE_R_PIN, HIGH); // set left microphone with high power level

  // wait power stable
  delay(2000);

	uint8_t state = 0;
	uint8_t ret = 0;

  i2s_channel_fmt_t CHANNEL_FORMAT;
  // Left : 0 / Right : 1 / stereo : 2
  if(channelSetting == 0){
    CHANNEL_FORMAT = I2S_CHANNEL_FMT_ONLY_LEFT;
  }else if(channelSetting == 1){
    CHANNEL_FORMAT = I2S_CHANNEL_FMT_ONLY_RIGHT;
  }else{
    CHANNEL_FORMAT = I2S_CHANNEL_FMT_RIGHT_LEFT;
  }

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),                                  
    .sample_rate = sampleRate,
    .bits_per_sample = (i2s_bits_per_sample_t)bit,
    .channel_format = CHANNEL_FORMAT,                          
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
		.intr_alloc_flags = 0,  
    .dma_buf_count = 32,  
    .dma_buf_len = 1024,  
    .use_apll = false
    };
  i2s_pin_config_t pin_config = {
    .bck_io_num = _bckIoNum,
    .ws_io_num = _wsIoNum,
    .data_out_num = -1,
    .data_in_num = _dInNum                                             
  };
	state = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  state += i2s_set_pin(I2S_NUM, &pin_config);
	if(state != 0)
		ret = state;
	else
		ret = 0;
  return ret;
}


uint32_t INMP441::read(byte* buffer,size_t len){
  uint32_t returnDataLen;
  i2s_read(I2S_NUM, (char*)buffer, len, &returnDataLen, 1);  // 1 portMAX_DELAY
  return returnDataLen;
}


void INMP441::louder(byte* buffer, size_t len, float volume_gain){
  for(uint32_t i = 0; i < len/2; i++){
    // read 2 byte and conbine 
    int16_t originalSound = buffer[(i*2)] + ((buffer[(i*2)+1]) * 256);  // 8 bit 平移
    // calculate
    int16_t calculated = originalSound * volume_gain;
    // limit the min and max
    if (abs(calculated) > 32767 ){
      if (originalSound > 0)
        calculated = 32767;
      else
        calculated = -32767;
    }
    // seperate and write back
    buffer[i*2] =  (byte)(calculated & 0xFF);
    buffer[i*2 + 1] = (byte)((calculated >> 8) & 0xFF);
  }
}


void INMP441::end(){
  Serial.println("Turn off mic power");
  digitalWrite(SOUND_PMOS,HIGH);   // Turn off, gpio default is low -> this will let mic ON
  digitalWrite(MODE_R_PIN,LOW);    // Turn off LR setpin to save power.
  digitalWrite(I2S_DI_IO,LOW);
  i2s_driver_uninstall(I2S_NUM);  
}


void INMP441::createWavHeader(byte* header, int recordSeconds, uint16_t longSampleRate, uint8_t bit, uint8_t channelSetting){

  /* 
  bytePerSecond :  How many byte per second. [Formula：SAMPLE_RATE * CHANNEL * DATA_BIT / 8 ]
  totalDataLen :  Total len of data part. Not include the wav header. [Formula：record_time(sec) * bytePerSecond]
  blockAlign : [Formula：channel num x DATA_BIT / 8]
  */  

  // init calculate 
  uint8_t channelNum;
  // Left : 0 / Right : 1 / stereo : 2
  if(channelSetting == 0 || channelSetting == 1){
    channelNum = 1;
  }else{
    channelNum = 2;
  }

  bytePerSecond = longSampleRate * channelNum * bit / 8;
  totalDataLen = recordSeconds * bytePerSecond;
  blockAlign = channelNum * bit / 8;
  

  // write header
  //// RIFF chunk 
  ////// Chunk ID
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  ////// Chunk Size
  unsigned int fileSizeMinus8 = totalDataLen + 44 - 8;
  header[4] = (byte)(fileSizeMinus8 & 0xFF);
  header[5] = (byte)((fileSizeMinus8 >> 8) & 0xFF);
  header[6] = (byte)((fileSizeMinus8 >> 16) & 0xFF);
  header[7] = (byte)((fileSizeMinus8 >> 24) & 0xFF);
  ////// Format
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';

  //// fmt sub chunk 
  ////// Subchunk 0 ID
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  ////// Subchunk 0 Size
  header[16] = 0x10;  // linear PCM
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;
  ////// Audio Format
  header[20] = 0x01;  // linear PCM
  header[21] = 0x00;
  ////// Num Channels (mono or Stereo)
  header[22] = (byte)(channelNum & 0xff);  
  header[23] = 0x00;
  ////// Sample rate
  header[24] = (byte) (longSampleRate & 0xff);
  header[25] = (byte) ((longSampleRate >> 8) & 0xff);
  header[26] = (byte) ((longSampleRate >> 16) & 0xff);
  header[27] = (byte) ((longSampleRate >> 24) & 0xff);
  ////// ByteRate (bytePerSecond)
  header[28] = (byte) (bytePerSecond & 0xff);
  header[29] = (byte) ((bytePerSecond >> 8) & 0xff);
  header[30] = (byte) ((bytePerSecond >> 16) & 0xff);
  header[31] = (byte) ((bytePerSecond >> 24) & 0xff);
  ////// Block align
  header[32] = (byte)(blockAlign & 0xff); 
  header[33] = 0x00;
  ////// Bits per sample (bit deep)
  header[34] = 0x10;  
  header[35] = 0x00;

  //// data sub chunk 
  ////// Subchunk 1 ID
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  ////// Subchunk 1 Size
  header[40] = (byte)(totalDataLen & 0xFF);
  header[41] = (byte)((totalDataLen >> 8) & 0xFF);
  header[42] = (byte)((totalDataLen >> 16) & 0xFF);
  header[43] = (byte)((totalDataLen >> 24) & 0xFF);
  ////// Data
  // data part after this 
}
