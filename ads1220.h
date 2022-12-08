/*
 * ads1220.h
 *
 *  Created on: 6 dic. 2022
 *      Author: carlo
 */

#ifndef ADS1220_H_
#define ADS1220_H_

#include <stdio.h>
#include <stdlib.h>

#include "spidrv.h"
#include "app_log.h"

//ADS1220 SPI commands
#define SPI_MASTER_DUMMY    0xFF
#define RESET               0x06   //Send the RESET command (06h) to make sure the ADS1220 is properly reset after power-up
#define START               0x08    //Send the START/SYNC command (08h) to start converting in continuous conversion mode
#define POWERDOWN     0x02   //Powerdown ADS1220
#define WREG  0x40
#define RREG  0x20

//Config registers
#define CONFIG_REG0_ADDRESS 0x00
#define CONFIG_REG1_ADDRESS 0x01
#define CONFIG_REG2_ADDRESS 0x02
#define CONFIG_REG3_ADDRESS 0x03

#define REG_CONFIG1_DR_MASK       0xE0
#define REG_CONFIG0_PGA_GAIN_MASK 0x0E
#define REG_CONFIG0_MUX_MASK      0xF0

#define DR_20SPS    0x00
#define DR_45SPS    0x20
#define DR_90SPS    0x40
#define DR_175SPS   0x60
#define DR_330SPS   0x80
#define DR_600SPS   0xA0
#define DR_1000SPS  0xC0

#define PGA_GAIN_1   0x00
#define PGA_GAIN_2   0x02
#define PGA_GAIN_4   0x04
#define PGA_GAIN_8   0x06
#define PGA_GAIN_16  0x08
#define PGA_GAIN_32  0x0A
#define PGA_GAIN_64  0x0C
#define PGA_GAIN_128 0x0E

#define MUX_AIN0_AIN1   0x00
#define MUX_AIN0_AIN2   0x10
#define MUX_AIN0_AIN3   0x20
#define MUX_AIN1_AIN2   0x30
#define MUX_AIN1_AIN3   0x40
#define MUX_AIN2_AIN3   0x50
#define MUX_AIN1_AIN0   0x60
#define MUX_AIN3_AIN2   0x70
#define MUX_AIN0_AVSS   0x80
#define MUX_AIN1_AVSS   0x90
#define MUX_AIN2_AVSS   0xA0
#define MUX_AIN3_AVSS   0xB0

#define MUX_SE_CH0      0x80
#define MUX_SE_CH1      0x90
#define MUX_SE_CH2      0xA0
#define MUX_SE_CH3      0xB0

// Typedefs
typedef struct ads1220_s ads1220_t;
typedef struct ads1220_settings_s ads1220_settings_t;

// Public method declaration
int init(ads1220_t* ads, SPIDRV_HandleData_t* spi_handle);

// Function declaration
Ecode_t spidrv_ADS1220_writeRegister(uint8_t address, uint8_t value);
Ecode_t spidrv_ADS1220_sendCommand(uint8_t command);
Ecode_t spidrv_ADS1220_writeAllRegister(ads1220_settings_t* settings);
Ecode_t spidrv_ADS1220_readAllRegister(ads1220_settings_t* settings);
uint8_t spidrv_ADS1220_readRegister(uint8_t address);
Ecode_t spidrv_ADS1220_readDataSample(int32_t* data);

// Struct definition
struct ads1220_settings_s{
  uint8_t reg0;
  uint8_t reg1;
  uint8_t reg2;
  uint8_t reg3;
};

struct ads1220_s{
  // Public
  SPIDRV_HandleData_t *spi_handle;
  int (*init)(ads1220_t* ads, SPIDRV_HandleData_t* spi_handle);
  // Private
  uint8_t tx_buffer[5];
  uint8_t rx_buffer[5];
  Ecode_t (*spidrv_ADS1220_writeRegister)(uint8_t address, uint8_t value);
  Ecode_t (*spidrv_ADS1220_sendCommand)(uint8_t command);
  Ecode_t (*spidrv_ADS1220_writeAllRegister)(ads1220_settings_t* settings);
  Ecode_t (*spidrv_ADS1220_readAllRegister)(ads1220_settings_t* settings);
  uint8_t (*spidrv_ADS1220_readRegister)(uint8_t address);
  Ecode_t (*spidrv_ADS1220_readDataSample)(int32_t* data);
};

#endif /* ADS1220_H_ */
