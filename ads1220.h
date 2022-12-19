
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
//   For information on how to use, visit https://github.com/Protocentral/Protocentral_ADS1220/
//
/////////////////////////////////////////////////////////////////////////////////////////


#include "spidrv.h"
#include "app_log.h"
#include "gpiointerrupt.h"
#include "sl_emlib_gpio_init_dataReady_config.h"
#include "sl_udelay.h"

//ADS1220 SPI commands
#define SPI_MASTER_DUMMY    0xFF
#define RESET               0x06   //Send the RESET command (06h) to make sure the ADS1220 is properly reset after power-up
#define START               0x08    //Send the START/SYNC command (08h) to start converting in continuous conversion mode
#define POWERDOWN			0x02   //Powerdown ADS1220
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
#define REG_CONFIG1_TS_MASK		  0x02

#define DR_40SPS    0x00
#define DR_90SPS    0x20
#define DR_180SPS   0x40
#define DR_350SPS   0x60
#define DR_660SPS   0x80
#define DR_1200SPS  0xA0
#define DR_2000SPS  0xC0

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

#define _BV(bit) (1<<(bit))

typedef struct ads1220_settings_s ads1220_settings_t;
typedef struct ads1220_s ads1220_t;

ads1220_t* init_ads1220(SPIDRV_Handle_t handle);
void writeRegister(ads1220_t* ads1220, uint8_t address, uint8_t value);
void writeAllRegister(ads1220_t* ads1220, ads1220_settings_t settings);
uint8_t readRegister(ads1220_t* ads1220, uint8_t address);
void readAllRegister(ads1220_t* ads1220, ads1220_settings_t* settings);
int begin(ads1220_t* ads1220);
void spi_command(ads1220_t* ads1220,uint8_t data_in);
void ads1220_reset(ads1220_t* ads1220);
void start_conv(ads1220_t* ads1220);
void pga_on(ads1220_t* ads1220);
void pga_off(ads1220_t* ads1220);
void set_conv_mode_continuous(ads1220_t* ads1220);
void set_conv_mode_single_shot(ads1220_t* ads1220);
void set_data_rate(ads1220_t* ads1220, int datarate);
void select_mux_channels(ads1220_t* ads1220, int channels_conf);
void set_pga_gain(ads1220_t* ads1220, int pgagain);
void temp_sense_on(ads1220_t* ads1220);
void temp_sense_off(ads1220_t* ads1220);
void get_config_reg(ads1220_t* ads1220, ads1220_settings_t* settings);
void set_config_reg(ads1220_t* ads1220, ads1220_settings_t settings);
int32_t read_data_samples(ads1220_t* ads1220);

struct ads1220_settings_s{
  uint8_t reg0;
  uint8_t reg1;
  uint8_t reg2;
  uint8_t reg3;
};

struct ads1220_s{
  // Private
  ads1220_settings_t settings;
  ads1220_settings_t settingsR;
  SPIDRV_Handle_t handle;

  void (*writeRegister)(ads1220_t* ads1220, uint8_t address, uint8_t value);
  void (*writeAllRegister)(ads1220_t* ads1220, ads1220_settings_t settings);
  uint8_t (*readRegister)(ads1220_t* ads1220, uint8_t address);
  void (*readAllRegister)(ads1220_t* ads1220, ads1220_settings_t* settings);

  // Public
  int (*begin)(ads1220_t* ads1220);
  void (*start_conv)(ads1220_t* ads1220);
  void (*ads1220_reset)(ads1220_t* ads1220);

  void (*spi_command)(ads1220_t* ads1220, unsigned char data_in);

  void (*get_config_reg)(ads1220_t* ads1220, ads1220_settings_t* settings);
  void (*set_config_reg)(ads1220_t* ads1220, ads1220_settings_t settings);

  void (*pga_off)(ads1220_t* ads1220);
  void (*pga_on)(ads1220_t* ads1220);
  void (*set_conv_mode_continuous)(ads1220_t* ads1220);
  void (*set_data_rate)(ads1220_t* ads1220, int datarate);
  void (*set_pga_gain)(ads1220_t* ads1220, int pgagain);
  void (*select_mux_channels)(ads1220_t* ads1220, int channels_conf);
  void (*set_conv_mode_single_shot)(ads1220_t* ads1220);
  void (*temp_sense_on)(ads1220_t* ads1220);
  void (*temp_sense_off)(ads1220_t* ads1220);
  int32_t (*read_data_samples)(ads1220_t* ads1220);
};
