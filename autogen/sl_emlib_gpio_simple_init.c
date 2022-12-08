#include "sl_emlib_gpio_simple_init.h"
#include "sl_emlib_gpio_init_bridgeON_config.h"
#include "sl_emlib_gpio_init_changeMode_config.h"
#include "sl_emlib_gpio_init_dataReady_config.h"
#include "em_gpio.h"
#include "em_cmu.h"

void sl_emlib_gpio_simple_init(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(SL_EMLIB_GPIO_INIT_BRIDGEON_PORT,
                  SL_EMLIB_GPIO_INIT_BRIDGEON_PIN,
                  SL_EMLIB_GPIO_INIT_BRIDGEON_MODE,
                  SL_EMLIB_GPIO_INIT_BRIDGEON_DOUT);

  GPIO_PinModeSet(SL_EMLIB_GPIO_INIT_CHANGEMODE_PORT,
                  SL_EMLIB_GPIO_INIT_CHANGEMODE_PIN,
                  SL_EMLIB_GPIO_INIT_CHANGEMODE_MODE,
                  SL_EMLIB_GPIO_INIT_CHANGEMODE_DOUT);

  GPIO_PinModeSet(SL_EMLIB_GPIO_INIT_DATAREADY_PORT,
                  SL_EMLIB_GPIO_INIT_DATAREADY_PIN,
                  SL_EMLIB_GPIO_INIT_DATAREADY_MODE,
                  SL_EMLIB_GPIO_INIT_DATAREADY_DOUT);
}
