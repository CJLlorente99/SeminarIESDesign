/*
 * ads1220.c
 *
 *  Created on: 6 dic. 2022
 *      Author: carlo
 */

#include "ads1220.h"
#include <stdbool.h>

static ads1220_t* ads1220;

// Flag to signal that transfer is complete
static volatile bool transfer_complete = false;
static volatile bool measurement_complete = false;

// Public functions definition
// Initialization method. WARNING ads should be allocated beforehand
int init(ads1220_t* ads, SPIDRV_HandleData_t* spi_handle){
  ads1220 = ads;
  ads1220->spi_handle = spi_handle;

  // Write settings into memory
  ads1220_settings_t memory = {
    .reg0 = 0b01101110, //Settings: AINP=AIN1, AINN=AIN0, Gain 128, PGA enabled
    .reg1 = 0b11010010, //Settings: DR=2000 SPS, Mode=Turbo, Conv mode=Single, Temp Sensor enabled, Current Source off
    .reg2 = 0b01000000, //Settings: Vref External, No 50/60Hz rejection, power open, IDAC off
    .reg3 = 0b00000000, //Settings: IDAC1 disabled, IDAC2 disabled, DRDY pin only
  };
  ads1220->spidrv_ADS1220_writeAllRegister(&memory);

  // Check settings into memory
  ads1220_settings_t memoryR;
  spidrv_ADS1220_readAllRegister(&memoryR);
  app_log_info("Register Set To: \n");
  app_log_info("%02x\n", memoryR.reg0);
  app_log_info("%02x\n", memoryR.reg1);
  app_log_info("%02x\n", memoryR.reg2);
  app_log_info("%02x\n", memoryR.reg3);

  if(memory.reg0 == memoryR.reg0 && memory.reg1 == memoryR.reg1 &&
      memory.reg2 == memoryR.reg2 && memory.reg3 == memoryR.reg3){
      return 0;
  }
  return 1;
}

// Write ADS1220 Register
Ecode_t spidrv_ADS1220_writeRegister(uint8_t address, uint8_t value){
  ads1220->tx_buffer[0] = WREG|(address<<2);
  ads1220->tx_buffer[1] = value;

  Ecode_t ecode;
  ecode = SPIDRV_MTransmitB(ads1220->spi_handle, ads1220->tx_buffer, 2) ;
  return ecode;
}

// Send ADS1220 Command
Ecode_t spidrv_ADS1220_sendCommand(uint8_t command){
  ads1220->tx_buffer[0] = 0x00;
  ads1220->tx_buffer[1] = command;

  Ecode_t ecode;
  ecode = SPIDRV_MTransmitB(ads1220->spi_handle, ads1220->tx_buffer, 2) ;
  return ecode;
}

// Write All ADS1220 Register
Ecode_t spidrv_ADS1220_writeAllRegister(ads1220_settings_t* settings){
  ads1220->tx_buffer[0] = WREG|0b11; // Write 4 Bytes starting at adress 0x00
  ads1220->tx_buffer[1] = settings->reg0;
  ads1220->tx_buffer[2] = settings->reg1;
  ads1220->tx_buffer[3] = settings->reg2;
  ads1220->tx_buffer[4] = settings->reg3;

  Ecode_t ecode;
  ecode = SPIDRV_MTransmitB(ads1220->spi_handle, ads1220->tx_buffer, 5) ;

  return ecode;
}

// Read All ADS1220 Register
Ecode_t spidrv_ADS1220_readAllRegister(ads1220_settings_t* settings){
  ads1220->tx_buffer[0] = RREG|0b11; // Read 4 Bytes starting at adress 0x00
  ads1220->tx_buffer[1]  =SPI_MASTER_DUMMY;
  ads1220->tx_buffer[2]  =SPI_MASTER_DUMMY;
  ads1220->tx_buffer[3]  =SPI_MASTER_DUMMY;
  ads1220->tx_buffer[4]  =SPI_MASTER_DUMMY;

  Ecode_t ecode;
  ecode = SPIDRV_MTransferB(ads1220->spi_handle, ads1220->tx_buffer, ads1220->rx_buffer, 5);
  settings->reg0 = ads1220->rx_buffer[1];
  settings->reg1 = ads1220->rx_buffer[2];
  settings->reg2 = ads1220->rx_buffer[3];
  settings->reg3 = ads1220->rx_buffer[4];

  return ecode;
}

// Read All ADS1220 Register
uint8_t spidrv_ADS1220_readRegister(uint8_t address){
  uint8_t tx_buffer[4] ;
  tx_buffer[0] = RREG|(address<<2)|0b11;
  tx_buffer[1]  =SPI_MASTER_DUMMY;
  tx_buffer[2]  =SPI_MASTER_DUMMY;
  tx_buffer[3]  =SPI_MASTER_DUMMY;
  uint8_t rx_buffer[4];

  transfer_complete = false;
  SPIDRV_MTransferB(ads1220->spi_handle, tx_buffer, rx_buffer, 4) ;

  app_log_info("Receive: %02x\n", rx_buffer[1]);
  return rx_buffer[0];
}

// Read ADS1220 Data Sample Function
Ecode_t spidrv_ADS1220_readDataSample(int32_t* data){
  Ecode_t ecode;
  uint8_t rx_buffer[3];

  ecode = SPIDRV_MReceiveB(ads1220->spi_handle, rx_buffer, 3) ;

  *data = rx_buffer[0];
  *data = (*data << 8) | rx_buffer[1];
  *data = (*data << 8) | rx_buffer[2];
  *data = (*data << 8);
  *data = (*data >> 8);

  return ecode;
}
