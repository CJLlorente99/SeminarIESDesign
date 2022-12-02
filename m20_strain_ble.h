/*
 * M20_Strain_BLE_fsm.h
 *
 *  Created on: 27 nov. 2022
 *      Author: carlo
 */

#ifndef M20_STRAIN_BLE_FSM_H_
#define M20_STRAIN_BLE_FSM_H_

#include <stdio.h>
#include <stdlib.h>

#include "sl_bt_m20_strain_sensor.h"
#include "app_log.h"
#include "spidrv.h"
#include "fsm.h"
#include "sl_sleeptimer.h"
#include "sl_emlib_gpio_init_changeMode_config.h"
#include "sl_emlib_gpio_init_dataReady_config.h"
#include "gpiointerrupt.h"
#include "app.h"

typedef struct app_fsm_s app_fsm_t;

/*
 * FSM creation function declaration
 */
fsm_t* new_app_fsm(app_fsm_t* user_data);

/*
 * FSM user data structure definition
 */
struct app_fsm_s {
  void* user_data;
  // Timer
  sl_sleeptimer_timer_handle_t  *tmr;
  // Sensor data
  uint16_t sensor_data[4];
  // SPI handle
  SPIDRV_Handle_t spi_handle;
  // Flags
  uint8_t enter_sleeping_flag : 1;
  uint8_t wakeup_timer_flag : 1;
  uint8_t wakeup_completed_flag : 1;
  uint8_t data_ready_flag : 1;
  uint8_t data_retrieved_flag : 1;
  uint8_t data_sent_flag : 1;
  uint8_t sleep_possible_flag : 1;
  uint8_t num_data_retrieved;
  // Flag to change between FSM branches
  uint8_t change_mode_flag : 1;
};

#endif /* M20_STRAIN_BLE_FSM_H_ */
