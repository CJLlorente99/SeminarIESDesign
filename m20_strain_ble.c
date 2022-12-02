/*
 * M20_Strain_BLE_fsm.c
 *
 *  Created on: 27 nov. 2022
 *      Author: carlo
 */

#include "m20_strain_ble.h"

/*
 * Callback declaration
 */
static void tmr_callback(sl_sleeptimer_timer_handle_t* handle, void* data);
static void retrieval_callback(SPIDRV_Handle_t handle, Ecode_t transferStatus, int itemsTransferred);
static void change_mode_callback(uint8_t intNo, void* ctx);
static void ready_to_retrieve_callback(uint8_t intNo, void* ctx);

/*
 * Enumeration that defines the FSM states
 */
enum states {
  IDLE,
  SLEEPING,
  WAKING_UP,
  ASKING_FOR_DATA,
  RETRIEVING_DATA,
  SENDING_DATA,
  TO_SLEEP
};
/*
 * Local variables
 */
static uint8_t sensorsReadCheck;

/*
 * Guard function declaration
 */
static int check_enter_sleeping(fsm_t* this);
static int check_wakeup_timer(fsm_t* this);
static int check_wakeup_completed(fsm_t* this);
static int check_all_data_retrieved(fsm_t* this);
static int check_data_ready(fsm_t* this);
static int check_data_retrieved(fsm_t* this);
static int check_data_sent(fsm_t* this);
static int check_sleep_not_possible(fsm_t* this);
static int check_sleep_possible(fsm_t* this);

/*
 * Transition function declaration
 */
static void wake_up(fsm_t* this);
static void ask_for_next_data(fsm_t* this);
static void power_down_interface_send_data(fsm_t* this);
static void retrieve_data(fsm_t* this);
static void reset_timer_sleep(fsm_t* this);
static void try_to_sleep(fsm_t* this);

/*
 * Transition table
 */
static fsm_trans_t app_fsm_tt[] = {
      { IDLE, check_enter_sleeping, SLEEPING, NULL },
      { SLEEPING, check_wakeup_timer, WAKING_UP, wake_up},
      { WAKING_UP, check_wakeup_completed, ASKING_FOR_DATA, ask_for_next_data},
      { ASKING_FOR_DATA, check_all_data_retrieved, SENDING_DATA, power_down_interface_send_data},
      { ASKING_FOR_DATA, check_data_ready, RETRIEVING_DATA, retrieve_data},
      { RETRIEVING_DATA, check_data_retrieved, ASKING_FOR_DATA, ask_for_next_data},
      { SENDING_DATA, check_data_sent, TO_SLEEP, try_to_sleep},
      { TO_SLEEP, check_sleep_not_possible, TO_SLEEP, try_to_sleep},
      { TO_SLEEP, check_sleep_possible, SLEEPING, reset_timer_sleep},
      { -1, NULL, -1, NULL},
};

/*
 * Guard funtions
 */
static int
check_enter_sleeping(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->enter_sleeping_flag;
}

static int
check_wakeup_timer(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->wakeup_timer_flag;
}

static int
check_wakeup_completed(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->wakeup_completed_flag;
}

static int
check_all_data_retrieved(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return (p_this->num_data_retrieved == 4);
}

static int
check_data_ready(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->data_ready_flag;
}

static int
check_data_retrieved(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->data_retrieved_flag = sensorsReadCheck;
  p_this->num_data_retrieved++;
  return p_this->data_retrieved_flag;
}

static int
check_data_sent(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->data_sent_flag;
}

static int
check_sleep_not_possible(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return !p_this->sleep_possible_flag;
}

static int
check_sleep_possible(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->sleep_possible_flag;
}

/*
 * FSM transition functions
 */

static void
wake_up(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->wakeup_timer_flag = 0;
  // Stop timer
  sl_status_t sc;
  sc = sl_sleeptimer_stop_timer(p_this->tmr);
  if(sc == SL_STATUS_OK){
      app_log_info("Timer stopped correctly");
  }
  // Bridge on pin high
  // TODO
  // Check registers from sensors through SPI
  int nBytes = 10;
  uint8_t registers[10];
  Ecode_t ec;
  ec = SPIDRV_MReceiveB(p_this->spi_handle, (void*)registers, nBytes); // Should take around 10us
  if(ec == ECODE_EMDRV_SPIDRV_OK){
      app_log_info("Sensor register read correctly");
      p_this->wakeup_completed_flag = 1;
  }
}

static void
ask_for_next_data(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->wakeup_completed_flag = 0;
  p_this->data_retrieved_flag = 0;
  p_this->data_ready_flag = 0; // for safety
  // Select next sensor to be read
  uint8_t nextSensor = p_this->num_data_retrieved;
  // Send info to ADS1220 (mux)
  if(nextSensor < 3){
      // Ask for strain data
      // TODO
  } else if(nextSensor == 3){
      // Ask for temp data
      // TODO
  }
}

static void
retrieve_data(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->data_ready_flag = 0;
  // Retrieve data through SPI
  int nBytes = sizeof(uint16_t);
  Ecode_t ec;
  ec = SPIDRV_MReceive(p_this->spi_handle, (void*)p_this->sensor_data[p_this->num_data_retrieved], nBytes, retrieval_callback);
  if(ec == ECODE_EMDRV_SPIDRV_OK){
      app_log_info("SPI data retrieval success");
  }
}

static void
power_down_interface_send_data(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->num_data_retrieved = 0;
  sl_status_t sc;
  // Power down
  // TODO
  // Convert each measurement into physical units
  // TODO
  sc = sl_bt_torque_send_data((uint8_t*)p_this->sensor_data);
  if(sc == SL_STATUS_OK){
      app_log_info("Attribute send: 0x%d", p_this->sensor_data[0]);
      app_log_info("Attribute send: 0x%d", p_this->sensor_data[1]);
      app_log_info("Attribute send: 0x%d", p_this->sensor_data[2]);
      app_log_info("Attribute send: 0x%d", p_this->sensor_data[3]);
      p_this->data_sent_flag = 1;
  }
}

static void
try_to_sleep(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->data_sent_flag = 0;
  // sl_status_t sc;
  // Can I go to sleep?
  // sl_power_manager_is_ok_to_sleep()
  p_this->sleep_possible_flag = 1;
}

static void
reset_timer_sleep(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->sleep_possible_flag = 0;
  // Start timer
  sl_status_t sc;
  uint32_t timeout = 10;
  sc = sl_sleeptimer_start_timer(p_this->tmr, timeout, tmr_callback, p_this, 0, SL_SLEEPTIMER_PERIPHERAL_BURTC);
  if(sc == SL_STATUS_OK){
        app_log_info("Timer started correctly");
  }
}

/*
 * FSM initialization
 */
fsm_t*
new_app_fsm(app_fsm_t* user_data){
  // Initialized data
  user_data->sensor_data[0] = 0;
  user_data->sensor_data[1] = 0;
  user_data->sensor_data[2] = 0;
  user_data->sensor_data[3] = 0;

  // Timer handle initialization
  sl_sleeptimer_timer_handle_t* tmr = malloc(sizeof(sl_sleeptimer_timer_handle_t));
  user_data->tmr = tmr;

  // GPIO Interruptions
  GPIOINT_CallbackRegisterExt(SL_EMLIB_GPIO_INIT_CHANGEMODE_PIN, change_mode_callback, user_data);
  GPIOINT_CallbackRegisterExt(SL_EMLIB_GPIO_INIT_DATAREADY_PIN, ready_to_retrieve_callback, user_data);

  return fsm_new(IDLE, app_fsm_tt, user_data);
}

/*
 * Callbacks
 */
static void
tmr_callback(sl_sleeptimer_timer_handle_t* handle, void* data)
{
  app_fsm_t* p_this = data;
  p_this->wakeup_timer_flag = 1;
  // Callback should wake up from sleep mode too (only if EM4)
}

static void
retrieval_callback(SPIDRV_Handle_t handle, Ecode_t transferStatus, int itemsTransferred){
  if(transferStatus == ECODE_EMDRV_SPIDRV_OK){
      sensorsReadCheck = 1;
      app_log_info("All sensors information retrieved");
  }
}

static void
change_mode_callback(uint8_t intNo, void* ctx){
  app_fsm_t* p_this = (app_fsm_t*)ctx;
  p_this->change_mode_flag = 1;
}

static void
ready_to_retrieve_callback(uint8_t intNo, void* ctx){
  app_fsm_t* p_this = (app_fsm_t*)ctx;
  p_this->data_ready_flag = 1;
}
