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

/*
 * Enumeration that defines the FSM states
 */
enum states {
  SLEEPING,
  WAKING_UP,
  ASKING_FOR_DATA,
  RETRIEVING_DATA,
  SENDING_DATA,
  TO_SLEEP
};


/*
 * Guard function declaration
 */
static int check_wakeup_timer(fsm_t* this);
static int check_wakeup_completed(fsm_t* this);
static int check_all_data_retrieved(fsm_t* this);
static int check_data_sent_continuous(fsm_t* this);
static int check_data_ready(fsm_t* this);
static int check_data_retrieved(fsm_t* this);
static int check_data_sent_not_continuous(fsm_t* this);
static int check_sleep_not_possible(fsm_t* this);
static int check_sleep_possible(fsm_t* this);
static int check_continuous_mode(fsm_t* this);

/*
 * Transition function declaration
 */
static void wake_up(fsm_t* this);
static void ask_for_next_data(fsm_t* this);
static void ask_again(fsm_t* this);
static void power_down_interface_send_data(fsm_t* this);
static void retrieve_data(fsm_t* this);
static void reset_timer_sleep(fsm_t* this);
static void try_to_sleep(fsm_t* this);
static void reset_no_timer(fsm_t* this);

/*
 * Transition table
 */
static fsm_trans_t app_fsm_tt[] = {
      { SLEEPING, check_wakeup_timer, WAKING_UP, wake_up},
      { WAKING_UP, check_wakeup_completed, ASKING_FOR_DATA, ask_for_next_data},
      { ASKING_FOR_DATA, check_all_data_retrieved, SENDING_DATA, power_down_interface_send_data},
      { ASKING_FOR_DATA, check_data_ready, RETRIEVING_DATA, retrieve_data},
      { RETRIEVING_DATA, check_data_retrieved, ASKING_FOR_DATA, ask_for_next_data},
      { SENDING_DATA, check_data_sent_continuous, WAKING_UP, ask_again},
      { SENDING_DATA, check_data_sent_not_continuous, TO_SLEEP, try_to_sleep},
      { TO_SLEEP, check_sleep_not_possible, TO_SLEEP, try_to_sleep},
      { TO_SLEEP, check_sleep_possible, SLEEPING, reset_timer_sleep},
      { TO_SLEEP, check_continuous_mode, SLEEPING, reset_no_timer},
      { -1, NULL, -1, NULL},
};

/*
 * Guard funtions
 */
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
//  p_this->data_retrieved_flag = sensorsReadCheck;
  if(p_this->data_retrieved_flag == 1) p_this->num_data_retrieved++;
  return p_this->data_retrieved_flag;
}

static int
check_data_sent_not_continuous(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->data_sent_flag && !p_this->change_mode_flag;
}

static int
check_data_sent_continuous(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->data_sent_flag && p_this->change_mode_flag;
}

static int
check_sleep_not_possible(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return !p_this->sleep_possible_flag && !p_this->change_mode_flag;
}

static int
check_sleep_possible(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->sleep_possible_flag && !p_this->change_mode_flag;
}

static int
check_continuous_mode(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->change_mode_flag;
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

  p_this->wakeup_completed_flag = 1;
}

static void
ask_for_next_data(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->wakeup_completed_flag = 0;
  p_this->data_retrieved_flag = 0;
  p_this->data_ready_flag = 0; // for safety

  p_this->data_ready_flag = 1;
}

static void
ask_again(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->data_sent_flag = 0;
  // Sensor should already be powered up
}

static void
retrieve_data(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->data_ready_flag = 0;

  p_this->sensor_data[p_this->num_data_retrieved]++;

  p_this->data_retrieved_flag = 1;
}

static void
power_down_interface_send_data(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->num_data_retrieved = 0;
  sl_status_t sc;

  // Send data through BLE
  sc = sl_bt_torque_send_data((uint8_t*)p_this->sensor_data, 4*sizeof(uint16_t));
  if(sc == SL_STATUS_OK){
      app_log_info("Attribute send: 0x%hu", p_this->sensor_data[0]);
      app_log_info("Attribute send: 0x%hu", p_this->sensor_data[1]);
      app_log_info("Attribute send: 0x%hu", p_this->sensor_data[2]);
      app_log_info("Attribute send: 0x%hu", p_this->sensor_data[3]);
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
  uint32_t timeout = 5000;
  sc = sl_sleeptimer_start_timer(p_this->tmr, timeout, tmr_callback, p_this, 0, SL_SLEEPTIMER_PERIPHERAL_BURTC);
  if(sc == SL_STATUS_OK){
        app_log_info("Timer started correctly");
  }
}

static void
reset_no_timer(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->sleep_possible_flag = 0;
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

  // Initialize flags
  user_data->wakeup_timer_flag = 1;
  user_data->wakeup_completed_flag = 0;
  user_data->data_ready_flag = 0;
  user_data->data_retrieved_flag = 0;
  user_data->data_sent_flag = 0;
  user_data->sleep_possible_flag = 0;
  user_data->num_data_retrieved = 0;
  user_data->change_mode_flag = 0;

  // Timer handle initialization
  sl_sleeptimer_timer_handle_t* tmr = malloc(sizeof(sl_sleeptimer_timer_handle_t));
  user_data->tmr = tmr;

  return fsm_new(SLEEPING, app_fsm_tt, user_data);
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
