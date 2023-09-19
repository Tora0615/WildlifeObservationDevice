#ifndef MEMS_INMP441_H
#define MEMS_INMP441_H

#include "stdio.h"
#include "Arduino.h"
#include <driver/i2s.h>

#define I2S_NUM       (i2s_port_t)(0)
#define I2S_SCK_IO    (27)    // SCK, clock
#define I2S_WS_IO     (26)    // WS, choose LR
#define I2S_DI_IO     (14)    // SD, I2S data output 
#define MODE_R_PIN    (15)    // 15 pin weak pull-up / Set LR of that micorphone / High - R
#define MODE_L_PIN    (2)     // 2 pin weak pull-down / Set LR of that micorphone / Low - L
#define SOUND_PMOS    (4)


#define INMP_DEBUG 



class INMP441{
  public:

    int bytePerSecond;
    int totalDataLen;
    uint8_t blockAlign;

    /**
    * @fn INMP441
    * @brief INMP441 Constructor
    * @param bckIoNum SCK (Serial Clock) Pin, clock 
    * @param wsIoNum  WS (Word select) pin, choose LR
    * @param dInNum   SD (Serial Data) pin, I2S data output 
    */
    INMP441(uint8_t bckIoNum, uint8_t wsIoNum, uint8_t dInNum);


    /**
    * @fn begin
    * @brief initial device
    * @param sampleRate 
    * @param bit 
    * @param channelSetting (Left : 0 / Right : 1 / stereo : 2)
    * @return status of initial
    */
    uint8_t begin(uint16_t sampleRate, uint8_t bit, uint8_t channelSetting);


    /**
    * @fn read
    * @brief read data from microphone
    * @param buffer  pointer of data buffer
    * @param len     len of data buffer
    * @return len
    */
    uint32_t read(byte* buffer, size_t len);

    /**
    * @fn 
    * @brief let sound louder
    * @param buffer
    * @param len
    * @param volume_gain 
    * @return None 
    */
    void louder(byte* buffer, size_t len, float volume_gain);

    /**
    * @fn end
    * @brief uninstall I2S driver and turn off relative pins
    * @return None
    */
    void end();


    /**
    * @fn createWavHeader
    * @brief create the header of WAVE file, then write to the data buffer array
    * @param header pointer of header array 
    * @param recordSeconds recordSeconds
    * @param longSampleRate sample rate
    * @param bit data bit
    * @param channelSetting channel num (Left : 0 / Right : 1 / stereo : 2)
    * @return None
    */
    void createWavHeader(byte* header, int recordSeconds, uint16_t longSampleRate, uint8_t bit, uint8_t channelSetting);
   

  private:
    uint8_t _bckIoNum;
    uint8_t _wsIoNum;
    uint8_t _dInNum;
    uint8_t _mode;
};

#endif