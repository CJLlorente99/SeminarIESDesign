//////////////////////////////////////////////////////////////////////////////////////////
//
//    Arduino library for the ADS1220 24-bit ADC breakout board
//
//    Author: Ashwin Whitchurch
//    Copyright (c) 2018 ProtoCentral
//
//    Based on original code from Texas Instruments
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//   For information on how to use, visit https://github.com/Protocentral/ADS1220/
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <ADS1220.h>

ads1220_t* init_ads1220(SPIDRV_Handle_t handle) // Constructor
{
  ads1220_t* ads1220 = malloc(sizeof(ads1220_t));

  ads1220->handle = handle;

  // Assign functions
  ads1220->begin = (void*) begin;
  ads1220->start_conv = (void*) start_conv;
  ads1220->ads1220_reset = (void*) ads1220_reset;
  ads1220->spi_command = (void*) spi_command;
  ads1220->get_config_reg = (void*) get_config_reg;
  ads1220->set_config_reg = (void*) set_config_reg;
  ads1220->pga_off = (void*) pga_off;
  ads1220->pga_on = (void*) pga_on;
  ads1220->set_conv_mode_continuous = (void*) set_conv_mode_continuous;
  ads1220->set_data_rate = (void*) set_data_rate;
  ads1220->set_pga_gain = (void*) set_pga_gain;
  ads1220->select_mux_channels = (void*) select_mux_channels;
  ads1220->set_conv_mode_single_shot = (void*) set_conv_mode_single_shot;
  ads1220->temp_sense_on = (void*) temp_sense_on;
  ads1220->temp_sense_off = (void*) temp_sense_off;
  ads1220->get_config_reg = (void*) get_config_reg;
  ads1220->read_data_samples = (void*) read_data_samples;

  return ads1220;
}

/*
 * Private Methods
 */

static void
writeRegister(ads1220_t* ads1220, uint8_t address, uint8_t value)
{
  Ecode_t ecode;
  uint8_t tx_buffer[2];
  tx_buffer[0] = WREG|(address<<2);
  tx_buffer[1] = value;

  ecode = SPIDRV_MTransmitB(ads1220->handle, tx_buffer, sizeof(tx_buffer));
  if(ecode == ECODE_OK){
      app_log_info("Write register operation completed successfully\n");
  }
}

static void
writeAllRegister(ads1220_t* ads1220, ads1220_settings_t settings)
{
  Ecode_t ecode;
  uint8_t tx_buffer[5];
  tx_buffer[0] = WREG|0b11; // Write 4 Bytes starting at adress 0x00
  tx_buffer[1] = settings.reg0;
  tx_buffer[2] = settings.reg1;
  tx_buffer[3] = settings.reg2;
  tx_buffer[4] = settings.reg3;

  ecode = SPIDRV_MTransmitB(ads1220->handle, tx_buffer, sizeof(tx_buffer));
  if(ecode == ECODE_OK){
      app_log_info("Write register operation completed successfully\n");
  }
}

static uint8_t
readRegister(ads1220_t* ads1220, uint8_t address)
{
    Ecode_t ecode;
    uint8_t tx_buffer[4];
    uint8_t rx_buffer[4];

    tx_buffer[0] = RREG|(address<<2);
    tx_buffer[1] = SPI_MASTER_DUMMY;
    tx_buffer[2] = SPI_MASTER_DUMMY;
    tx_buffer[3] = SPI_MASTER_DUMMY;

    ecode = SPIDRV_MTransferB(ads1220->handle, tx_buffer, rx_buffer, 4);
    if(ecode == ECODE_OK){
        app_log_info("Read register operation completed successfully\n");
    }
    return rx_buffer[1];
}

static void
readAllRegister(ads1220_t* ads1220, ads1220_settings_t* settings)
{
    Ecode_t ecode;
    uint8_t tx_buffer[5];

    tx_buffer[0] = RREG|0b11; // Read 4 Bytes starting at address 0x00
    tx_buffer[1] = SPI_MASTER_DUMMY;
    tx_buffer[2] = SPI_MASTER_DUMMY;
    tx_buffer[3] = SPI_MASTER_DUMMY;
    tx_buffer[4] = SPI_MASTER_DUMMY;

    ecode = SPIDRV_MTransferB(ads1220->handle, tx_buffer, settings, 5);
    if(ecode == ECODE_OK){
        app_log_info("Read register operation completed successfully\n");
    }
}

/*
 * Public Methods
 */

int begin(ads1220_t* ads1220)
{
  // CS and SPI configuration is made through the Simplify Studio wizard
  sl_udelay_wait(100000);
  ads1220->ads1220_reset(ads1220);
  sl_udelay_wait(100000);

  ads1220_settings_t* settings = &(ads1220->settings);
  settings->reg0 = 0b01101110; //Settings: AINP=AIN1, AINN=AIN0, Gain 128, PGA enabled
  settings->reg1 = 0b11010000; //Settings: DR=2000 SPS, Mode=Turbo, Conv mode=Single, Temp Sensor disabled, Current Source off
  settings->reg2 = 0b01000000; //Settings: Vref External, No 50/60Hz rejection, power open, IDAC off
  settings->reg3 = 0b00000000; //Settings: IDAC1 disabled, IDAC2 disabled, DRDY pin only

//  writeAllRegister(ads1220, *settings);
  writeRegister(ads1220, CONFIG_REG0_ADDRESS, settings->reg0);
  writeRegister(ads1220, CONFIG_REG1_ADDRESS, settings->reg1);
  writeRegister(ads1220, CONFIG_REG2_ADDRESS, settings->reg2);
  writeRegister(ads1220, CONFIG_REG3_ADDRESS, settings->reg3);

  sl_udelay_wait(100000);

  ads1220_settings_t* settingsR = &(ads1220->settingsR);
//  readAllRegister(ads1220, settingsR);
  settingsR->reg0 = readRegister(ads1220, CONFIG_REG0_ADDRESS);
  settingsR->reg1 = readRegister(ads1220, CONFIG_REG1_ADDRESS);
  settingsR->reg2 = readRegister(ads1220, CONFIG_REG2_ADDRESS);
  settingsR->reg3 = readRegister(ads1220, CONFIG_REG3_ADDRESS);

  app_log_info("Config_Reg : 0x%hhu 0x%hhu 0x%hhu 0x%hhu\n", settingsR->reg0, settingsR->reg1, settingsR->reg2, settingsR->reg3);

  sl_udelay_wait(100000);

  return 0;
}

void spi_command(ads1220_t* ads1220,uint8_t data_in)
{
  Ecode_t ecode;
  uint8_t tx_buffer[2];

  tx_buffer[0] = 0x00;
  tx_buffer[1] = data_in;

  ecode = SPIDRV_MTransmitB(ads1220->handle, tx_buffer, 2);
  if(ecode == ECODE_OK){
      app_log_info("Command send correctly: 0x%hhu\n", &data_in);
  }
}

void ads1220_reset(ads1220_t* ads1220)
{
  ads1220->spi_command(ads1220, RESET);
}

void start_conv(ads1220_t* ads1220)
{
  ads1220->spi_command(ads1220, START);
}

void pga_on(ads1220_t* ads1220)
{
  ads1220_settings_t* settingsR = &(ads1220->settingsR);
  settingsR->reg0 &= ~_BV(0);
  writeRegister(ads1220, CONFIG_REG0_ADDRESS, settingsR->reg0);
}

void pga_off(ads1220_t* ads1220)
{
  ads1220_settings_t* settingsR = &(ads1220->settingsR);
  settingsR->reg0 |= _BV(0);
  writeRegister(ads1220, CONFIG_REG0_ADDRESS, settingsR->reg0);
}

void set_conv_mode_continuous(ads1220_t* ads1220)
{
  ads1220_settings_t* settingsR = &(ads1220->settingsR);
  settingsR->reg1 |= _BV(2);
  writeRegister(ads1220, CONFIG_REG1_ADDRESS, settingsR->reg1);
}

void set_conv_mode_single_shot(ads1220_t* ads1220)
{
  ads1220_settings_t* settingsR = &(ads1220->settingsR);
  settingsR->reg1 &= ~_BV(2);
  writeRegister(ads1220, CONFIG_REG1_ADDRESS, settingsR->reg1);
}

void set_data_rate(ads1220_t* ads1220, int datarate)
{
  ads1220_settings_t* settingsR = &(ads1220->settingsR);
  settingsR->reg1 &= ~REG_CONFIG1_DR_MASK;
  settingsR->reg1 |= datarate;
  writeRegister(ads1220, CONFIG_REG1_ADDRESS, settingsR->reg1);
}

void select_mux_channels(ads1220_t* ads1220, int channels_conf)
{
  ads1220_settings_t* settingsR = &(ads1220->settingsR);
  settingsR->reg0 &= ~REG_CONFIG0_MUX_MASK;
  settingsR->reg0 |= channels_conf;
  writeRegister(ads1220, CONFIG_REG0_ADDRESS, settingsR->reg0);
}

void set_pga_gain(ads1220_t* ads1220, int pgagain)
{
  ads1220_settings_t* settingsR = &(ads1220->settingsR);
  settingsR->reg0 &= ~REG_CONFIG0_PGA_GAIN_MASK;
  settingsR->reg0 |= pgagain ;
  writeRegister(ads1220, CONFIG_REG0_ADDRESS, settingsR->reg0);
}

void temp_sense_on(ads1220_t* ads1220)
{
  ads1220_settings_t* settingsR = &(ads1220->settingsR);
  settingsR->reg1 &= ~REG_CONFIG1_TS_MASK;
  settingsR->reg1 |= 0x02 ;
  writeRegister(ads1220, CONFIG_REG1_ADDRESS, settingsR->reg1);
}

void temp_sense_off(ads1220_t* ads1220)
{
  ads1220_settings_t* settingsR = &(ads1220->settingsR);
  settingsR->reg1 &= ~REG_CONFIG1_TS_MASK;
  settingsR->reg1 |= 0x00 ;
  writeRegister(ads1220, CONFIG_REG1_ADDRESS, settingsR->reg1);
}

void get_config_reg(ads1220_t* ads1220, ads1220_settings_t* settings)
{
  ads1220_settings_t* settingsR = &(ads1220->settingsR);
  settingsR->reg0 = readRegister(ads1220, CONFIG_REG0_ADDRESS);
  settingsR->reg1 = readRegister(ads1220, CONFIG_REG1_ADDRESS);
  settingsR->reg2 = readRegister(ads1220, CONFIG_REG2_ADDRESS);
  settingsR->reg3 = readRegister(ads1220, CONFIG_REG3_ADDRESS);

  settings->reg0 = settingsR->reg0;
  settings->reg0 = settingsR->reg1;
  settings->reg0 = settingsR->reg2;
  settings->reg0 = settingsR->reg3;
}

void set_config_reg(ads1220_t* ads1220, ads1220_settings_t settings){
  writeRegister(ads1220, CONFIG_REG0_ADDRESS, settings.reg0);
  writeRegister(ads1220, CONFIG_REG1_ADDRESS, settings.reg1);
  writeRegister(ads1220, CONFIG_REG2_ADDRESS, settings.reg2);
  writeRegister(ads1220, CONFIG_REG3_ADDRESS, settings.reg3);
}

int32_t read_data_samples(ads1220_t* ads1220)
{
  Ecode_t ecode;
  static uint8_t rx_buffer[3];
  int32_t mResult32 = 0;

  for (int i = 0; i < 3; i++)
  {
    ecode = SPIDRV_MReceiveB(ads1220->handle, &rx_buffer[i], 1);
    if(ecode == ECODE_OK){
        app_log_info("Data sample received: 0x%hhu\n", &rx_buffer[i]);
    }
  }

  mResult32 = rx_buffer[0];
  mResult32 = (mResult32 << 8) | rx_buffer[1];
  mResult32 = (mResult32 << 8) | rx_buffer[2]; // Converting 3 bytes to a 24 bit int
  mResult32 = ( mResult32 << 8 );
  mResult32 = ( mResult32 >> 8 ); // Converting 24 bit two's complement to 32 bit two's complement

  return mResult32;
}
