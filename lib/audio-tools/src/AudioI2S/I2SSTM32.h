#pragma once

#ifdef STM32
#include "AudioI2S/I2SConfig.h"

namespace audio_tools {

/**
 * @brief Basic I2S API - for the STM32
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class I2SBase {
  friend class I2SStream;

  public:

    /// Provides the default configuration
    I2SConfig defaultConfig(RxTxMode mode = TX_MODE) {
        I2SConfig c(mode);
        return c;
    }

    /// starts the DAC with the default config in TX Mode
    bool begin(RxTxMode mode = TX_MODE) {
      return begin(defaultConfig(mode));
    }

    /// starts the DAC 
    bool begin(I2SConfig cfg) {
      bool result = true;
      this->cfg = cfg;
      i2s.Instance = SPI2;
      if (cfg.channels=!2){
        LOGE("Unsupported channels %d - must be 2", cfg.channels);
      }

      i2s.Init.Mode = getMode(cfg);
      i2s.Init.Standard = getStandard(cfg);
      i2s.Init.DataFormat = getDataFormat(cfg);
      i2s.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
      i2s.Init.AudioFreq = cfg.sample_rate;
      i2s.Init.CPOL = I2S_CPOL_LOW;
      i2s.Init.ClockSource = I2S_CLOCK_PLL;
      i2s.Init.FullDuplexMode = cfg.rx_tx_mode==RXTX_MODE? I2S_FULLDUPLEXMODE_ENABLE: I2S_FULLDUPLEXMODE_DISABLE;
      if (HAL_I2S_Init(&i2s) != HAL_OK){
        LOGE("HAL_I2S_Init failed");
        result = false;
      }
      return result;
    }

    /// stops the I2C and unistalls the driver
    void end(){
      if (HAL_I2S_DeInit(&i2s) != HAL_OK){
        LOGE("HAL_I2S_DeInit failed");
      }
    }

    /// we assume the data is already available in the buffer
    int available() {
      return I2S_BUFFER_COUNT*I2S_BUFFER_SIZE;
    }

    /// We limit the write size to the buffer size
    int availableForWrite() {
      return I2S_BUFFER_COUNT*I2S_BUFFER_SIZE;
    }

    /// provides the actual configuration
    I2SConfig config() {
      return cfg;
    }

    /// writes the data to the I2S interface
    size_t writeBytes(const void *src, size_t size_bytes){
      size_t result = 0;
      HAL_StatusTypeDef res = HAL_I2S_Transmit(&i2s, (uint16_t*)src, size_bytes/2, HAL_MAX_DELAY);
      if(res == HAL_OK) {
        result = size_bytes;
      } else {
        LOGE("HAL_I2S_Transmit failed");
      }
            
      return result;
    }

    size_t readBytes(void *dest, size_t size_bytes){
      size_t result = 0;
      HAL_StatusTypeDef res = HAL_I2S_Receive(&i2s, (uint16_t*)dest, size_bytes, HAL_MAX_DELAY);
      if(res == HAL_OK) {
        result = size_bytes;
      } else {
        LOGE("HAL_I2S_Receive failed");
      }

      return result;
    }

  protected:
    I2SConfig cfg;
    I2S_HandleTypeDef i2s;

    uint32_t getMode(I2SConfig &cfg){
      if (cfg.is_master) {
        switch(cfg.rx_tx_mode){
          case RX_MODE:
            return I2S_MODE_MASTER_RX;
          case TX_MODE:
            return I2S_MODE_MASTER_TX;
          default:
            LOGE("RXTX_MODE not supported");
            return I2S_MODE_MASTER_TX;
        }
      } else {
        switch(cfg.rx_tx_mode){
          case RX_MODE:
            return I2S_MODE_SLAVE_RX;
          case TX_MODE:
            return I2S_MODE_SLAVE_TX;
          default:
            LOGE("RXTX_MODE not supported");
            return I2S_MODE_SLAVE_TX;
        }
      }
    }

    
    uint32_t getStandard(I2SConfig &cfg){
      uint32_t result;
      switch(cfg.i2s_format) {
          case I2S_PHILIPS_FORMAT:
          return I2S_STANDARD_PHILIPS;
        case I2S_STD_FORMAT:
        case I2S_LSB_FORMAT:
        case I2S_RIGHT_JUSTIFIED_FORMAT:
          return I2S_STANDARD_MSB;
        case I2S_MSB_FORMAT:
        case I2S_LEFT_JUSTIFIED_FORMAT:
          return I2S_STANDARD_LSB;
      }
      return I2S_STANDARD_PHILIPS;
    }

    uint32_t getDataFormat(I2SConfig &cfg) {
        switch(cfg.bits_per_sample){
        case 16:
          return I2S_DATAFORMAT_16B;
        case 24:
          return I2S_DATAFORMAT_24B;
        case 32:
          return I2S_DATAFORMAT_32B;
      }
      return I2S_DATAFORMAT_16B;

    }

};

}

#endif
