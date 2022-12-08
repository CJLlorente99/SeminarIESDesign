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

//#define BOARD_SENSYTHING ST_1_3

// TODO: Search config file in example and check is the same as this project's

ADS1220::ADS1220() 								// Constructors
{

}

/*
 * Private Methods
 */

void ADS1220::writeRegister(uint8_t address, uint8_t value)
{
  Ecode_t ecode;
  uint8_t tx_buffer[2];
  tx_buffer[0] = WREG|(address<<2);
  tx_buffer[1] = value;

  ecode = SPIDRV_MTransmitB(handle, tx_buffer, sizeof(tx_buffer));
  if(ecode == ECODE_OK){
      app_log_info("Write register operation completed successfully");
  }
}

void ADS1220::writeAllRegister(ads1220_settings_t settings)
{
  Ecode_t ecode;
  uint8_t tx_buffer[5];
  tx_buffer[0] = WREG|0b11; // Write 4 Bytes starting at adress 0x00
  tx_buffer[1] = settings.reg0;
  tx_buffer[2] = settings.reg1;
  tx_buffer[3] = settings.reg2;
  tx_buffer[4] = settings.reg3;

  ecode = SPIDRV_MTransmitB(handle, tx_buffer, sizeof(tx_buffer));
  if(ecode == ECODE_OK){
      app_log_info("Write register operation completed successfully");
  }
}

uint8_t ADS1220::readRegister(uint8_t address)
{
    Ecode_t ecode;
    uint8_t tx_buffer[4];
    uint8_t rx_buffer[4];

    tx_buffer[0] = RREG|(address<<2);
    tx_buffer[1] = SPI_MASTER_DUMMY;
    tx_buffer[2] = SPI_MASTER_DUMMY;
    tx_buffer[3] = SPI_MASTER_DUMMY;

    ecode = SPIDRV_MTransferB(handle, tx_buffer, rx_buffer, 4);
    if(ecode == ECODE_OK){
        app_log_info("Read register operation completed successfully");
    }
    return rx_buffer[0];
}

void ADS1220::readAllRegister(ads1220_settings_t* settings)
{
    Ecode_t ecode;
    uint8_t tx_buffer[5];

    tx_buffer[0] = RREG|0b11; // Read 4 Bytes starting at address 0x00
    tx_buffer[1] = SPI_MASTER_DUMMY;
    tx_buffer[2] = SPI_MASTER_DUMMY;
    tx_buffer[3] = SPI_MASTER_DUMMY;
    tx_buffer[4] = SPI_MASTER_DUMMY;

    ecode = SPIDRV_MTransferB(handle, tx_buffer, settings, 5);
    if(ecode == ECODE_OK){
        app_log_info("Read register operation completed successfully");
    }
}

/*
 * Public Methods
 */

void ADS1220::begin()
{
  // CS and SPI configuration is made through the Simplify Studio wizard
  sl_udelay_wait(100000);
  ads1220_reset();
  sl_udelay_wait(100000);

  settings.reg0 = 0b01101110; //Settings: AINP=AIN1, AINN=AIN0, Gain 128, PGA enabled
  settings.reg1 = 0b11010000; //Settings: DR=2000 SPS, Mode=Turbo, Conv mode=Single, Temp Sensor disabled, Current Source off
  settings.reg2 = 0b01000000; //Settings: Vref External, No 50/60Hz rejection, power open, IDAC off
  settings.reg3 = 0b00000000; //Settings: IDAC1 disabled, IDAC2 disabled, DRDY pin only

  writeAllRegister(settings);

  sl_udelay_wait(100000);

  readAllRegister(&settingsR);

  app_log_info("Config_Reg : 0x%hhu 0x%hhu 0x%hhu 0x%hhu", settingsR.reg0, settingsR.reg1, settingsR.reg2, settingsR.reg3);

  sl_udelay_wait(100000);
}

void ADS1220::spi_command(uint8_t data_in)
{
  Ecode_t ecode;
  uint8_t tx_buffer[2];

  tx_buffer[0] = 0x00;
  tx_buffer[1] = data_in;

  ecode = SPIDRV_MTransmitB(handle, tx_buffer, 2);
  if(ecode == ECODE_OK){
      app_log_info("Command send correctly: 0x%hhu", &data_in);
  }
}

void ADS1220::ads1220_reset()
{
  spi_command(RESET);
}

void ADS1220::start_conv()
{
  spi_command(START);
}

void ADS1220::pga_on(void)
{
  settingsR.reg0 &= ~_BV(0);
  writeRegister(CONFIG_REG0_ADDRESS,settingsR.reg0);
}

void ADS1220::pga_off(void)
{
  settingsR.reg0 |= _BV(0);
  writeRegister(CONFIG_REG0_ADDRESS,settingsR.reg0);
}

void ADS1220::set_conv_mode_continuous(void)
{
  settingsR.reg1 |= _BV(2);
  writeRegister(CONFIG_REG1_ADDRESS,settingsR.reg1);
}

void ADS1220::set_conv_mode_single_shot(void)
{
  settingsR.reg1 &= ~_BV(2);
  writeRegister(CONFIG_REG1_ADDRESS,settingsR.reg1);
}

void ADS1220::set_data_rate(int datarate)
{
  settingsR.reg1 &= ~REG_CONFIG1_DR_MASK;
  settingsR.reg1 |= datarate;
  writeRegister(CONFIG_REG1_ADDRESS,settingsR.reg1);
}

void ADS1220::select_mux_channels(int channels_conf)
{
  settingsR.reg0 &= ~REG_CONFIG0_MUX_MASK;
  settingsR.reg0 |= channels_conf;
  writeRegister(CONFIG_REG0_ADDRESS,settingsR.reg0);
}

void ADS1220::set_pga_gain(int pgagain)
{
  settingsR.reg0 &= ~REG_CONFIG0_PGA_GAIN_MASK;
  settingsR.reg0 |= pgagain ;
  writeRegister(CONFIG_REG0_ADDRESS,settingsR.reg0);
}

void ADS1220::temp_sense_on()
{
  settingsR.reg1 &= ~REG_CONFIG1_TS_MASK;
  settingsR.reg1 |= 0x02 ;
  writeRegister(CONFIG_REG1_ADDRESS,settingsR.reg1);
}

void ADS1220::temp_sense_off()
{
  settingsR.reg1 &= ~REG_CONFIG1_TS_MASK;
  settingsR.reg1 |= 0x00 ;
  writeRegister(CONFIG_REG1_ADDRESS, settingsR.reg1);
}

void ADS1220::get_config_reg(ads1220_settings_t* settings)
{
  readAllRegister(&settingsR);

  settings->reg0 = settingsR.reg0;
  settings->reg0 = settingsR.reg1;
  settings->reg0 = settingsR.reg2;
  settings->reg0 = settingsR.reg3;
}

void ADS1220::set_config_reg(ads1220_settings_t settings){
  writeAllRegister(settings);
}

int32_t ADS1220::read_data_samples()
{
  Ecode_t ecode;
  static uint8_t rx_buffer[3];
  int32_t mResult32 = 0;

  for (int i = 0; i < 3; i++)
  {
    ecode = SPIDRV_MReceiveB(handle, &rx_buffer[i], 1);
    if(ecode == ECODE_OK){
        app_log_info("Data sample received: 0x%hhu", &rx_buffer[i]);
    }
  }

  mResult32 = rx_buffer[0];
  mResult32 = (mResult32 << 8) | rx_buffer[1];
  mResult32 = (mResult32 << 8) | rx_buffer[2]; // Converting 3 bytes to a 24 bit int
  mResult32 = ( mResult32 << 8 );
  mResult32 = ( mResult32 >> 8 ); // Converting 24 bit two's complement to 32 bit two's complement

  return mResult32;
}
