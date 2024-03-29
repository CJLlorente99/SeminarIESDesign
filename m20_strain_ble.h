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
#include "ads1220.h"
#include "app_log.h"
#include "spidrv.h"
#include "fsm.h"
#include "sl_emlib_gpio_init_changeMode_config.h"
#include "sl_emlib_gpio_init_dataReady_config.h"
#include "sl_emlib_gpio_init_bridgeON_config.h"
#include "sl_emlib_gpio_init_emuWakeup_config.h"
#include "gpiointerrupt.h"
#include "app.h"
#include "em_rmu.h"
#include "em_gpio.h"
#include "em_burtc.h"
#include "sl_sleeptimer.h"

typedef struct app_fsm_s app_fsm_t;

// Constants
#define PGA 128                 // Programmable Gain = 128
#define VREF 2.5                // External reference of 2.048V
#define VFSR VREF/PGA
#define FSR (((long int)1<<23)-1)

/*
 * FSM creation function declaration
 */
fsm_t* new_app_fsm(app_fsm_t* user_data, SPIDRV_Handle_t spi_handle, uint8_t* connections, int* nConnections);

/*
 * FSM user data structure definition
 */
struct app_fsm_s {
  void* user_data;
  // Timer
  // Sensor data
  int32_t sensor_data[4];
  // SPI handle
  SPIDRV_Handle_t spi_handle;
  // Sleeptimer
  sl_sleeptimer_timer_handle_t* tmr;
  // ADS1220
  ads1220_t* ads1220;
  // Flags
  uint8_t wakeup_timer_flag;
  uint8_t wakeup_completed_flag;
  uint8_t data_ready_flag;
  uint8_t data_retrieved_flag;
  uint8_t data_sent_flag;
  uint8_t sleep_possible_flag;
  int num_data_retrieved;
  uint8_t tmr_flag;
  // Flag to change between modes
  uint8_t change_mode_flag;
  // BLE data
  uint8_t* connections;
  int* nConnections;
};

#endif /* M20_STRAIN_BLE_FSM_H_ */
