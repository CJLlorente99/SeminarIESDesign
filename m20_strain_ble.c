/*
 * M20_Strain_BLE_fsm.c
 *
 *  Created on: 27 nov. 2022
 *      Author: carlo
 */

#include "m20_strain_ble.h"

#define EM4WU_EM4WUEN_NUM   (3)
#define EM4WU_EM4WUEN_MASK  (1 << EM4WU_EM4WUEN_NUM)

/*
 * Callback declaration
 */
static void change_mode_callback(uint8_t intNo);
static void ready_to_retrieve_callback(uint8_t intNo);
static void sleeptimer_callback(sl_sleeptimer_timer_handle_t* handle, void* data);

/*
 * Auxiliar function declaration
 */
// TODO: Check types!!
static void convertToMicroVStrain(float* result, int32_t data);
static void convertToMicroVTemp(float* result, int32_t data);

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
 * Local variables
 */
static uint8_t sensorsReadCheck = 1;
static uint8_t* data_ready_flag = 0;
static uint8_t* change_mode_flag = 0;


/*
 * Guard function declaration
 */
static int check_wakeup_timer(fsm_t* this);
static int check_change_mode(fsm_t* this);
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
      { SLEEPING, check_change_mode, WAKING_UP, wake_up},
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

/**************************************************************************//**
 * @brief  BURTC Handler
 *****************************************************************************/
static int wakeupTimer = 0;
void BURTC_IRQHandler(void)
{
  BURTC_IntClear(BURTC_IF_COMP); // compare match
  wakeupTimer = 1;
}


/*
 * Guard funtions
 */
static int
check_wakeup_timer(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->wakeup_timer_flag;
}

static int
check_change_mode(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  return p_this->change_mode_flag;
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
  return p_this->data_ready_flag && !(p_this->num_data_retrieved == 4);
}

static int
check_data_retrieved(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->data_retrieved_flag = sensorsReadCheck;
  p_this->num_data_retrieved++;
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
  return p_this->sleep_possible_flag && !p_this->change_mode_flag && p_this->tmr_flag;
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

  GPIO_EM4DisablePinWakeup(EM4WU_EM4WUEN_MASK << _GPIO_EM4WUEN_EM4WUEN_SHIFT);

  // Determine whether reset is due to pin (switch to continuous mode) or timer (slow mode)
  EMU_UnlatchPinRetention();
  uint32_t resetCause = RMU_ResetCauseGet();
  RMU_ResetCauseClear();
  if (resetCause & EMU_RSTCAUSE_EM4){
      app_log_info("Reset cause is EM4\n");
      if (wakeupTimer == 1) {
          p_this->change_mode_flag = 0;
          app_log_info("Wake up through timer\n");
          wakeupTimer = 0;
      } else{
          p_this->change_mode_flag = 1;
          app_log_info("Wake up through GPIO\n");
      }
  }

  // Bridge on pin high
  GPIO_PinOutSet(SL_EMLIB_GPIO_INIT_BRIDGEON_PORT, SL_EMLIB_GPIO_INIT_BRIDGEON_PIN);

  // Check registers from sensors through SPI
  int status;
  ads1220_t* ads1220 = p_this->ads1220;
  status = ads1220->begin(ads1220);
  if(status == 0){
      app_log_info("Sensor register read correctly\n");
      p_this->wakeup_completed_flag = 1;
  } else{
      app_log_info("Sensor register read incorrectly\n");
  }
}

static void
ask_for_next_data(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->wakeup_completed_flag = 0;
  p_this->data_retrieved_flag = 0;
  p_this->data_ready_flag = 0; // for safety

  ads1220_t* ads1220 = p_this->ads1220;

  // Select next sensor to be read
  int nextSensor = p_this->num_data_retrieved;
  // Send info to ADS1220 (mux)
  if(nextSensor < 3){
      // Ask for strain data
      // TODO: Should change reg0
      GPIO_PinOutClear(SL_EMLIB_GPIO_INIT_BRIDGEON_PORT, SL_EMLIB_GPIO_INIT_BRIDGEON_PIN);
      ads1220->temp_sense_on(ads1220);
      ads1220->start_conv(ads1220);
  } else if(nextSensor == 3){
      // Ask for temp data
      GPIO_PinOutClear(SL_EMLIB_GPIO_INIT_BRIDGEON_PORT, SL_EMLIB_GPIO_INIT_BRIDGEON_PIN);
      ads1220->temp_sense_on(ads1220);
      ads1220->start_conv(ads1220);
  }
}

static void
ask_again(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->data_sent_flag = 0;
  p_this->wakeup_completed_flag = 1;
  // Sensor should already be powered up
}

static void
retrieve_data(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->data_ready_flag = 0;
  p_this->data_retrieved_flag = 0;

  // Retrieve data through SPI
  // TODO: think how to activate flag to change state. Now it is a blocking activity run in ADS1220 private method (so guard function returns 1 always)
  ads1220_t* ads1220 = p_this->ads1220;
  p_this->sensor_data[p_this->num_data_retrieved] = ads1220->read_data_samples(ads1220);
}

static void
power_down_interface_send_data(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->num_data_retrieved = 0;
  sl_status_t sc;

  // Power down
  ads1220_t* ads1220 = p_this->ads1220;
  ads1220->temp_sense_off(ads1220);

  // Convert each measurement into physical units
  float result[4];
//  convertToMicroVStrain(&result[0], p_this->sensor_data[0]);
//  convertToMicroVStrain(&result[1], p_this->sensor_data[1]);
//  convertToMicroVStrain(&result[2], p_this->sensor_data[2]);
  convertToMicroVTemp(&result[0], p_this->sensor_data[0]);
  convertToMicroVTemp(&result[1], p_this->sensor_data[1]);
  convertToMicroVTemp(&result[2], p_this->sensor_data[2]);
  convertToMicroVTemp(&result[3], p_this->sensor_data[3]);

  // Send data through BLE
  sc = sl_bt_torque_send_data((uint32_t*)result, p_this->advertisement_handle);
  if(sc == SL_STATUS_OK){
      app_log_info("Attribute send: 0x%f\n", result[0]);
      app_log_info("Attribute send: 0x%f\n", result[1]);
      app_log_info("Attribute send: 0x%f\n", result[2]);
      app_log_info("Attribute send: 0x%f\n", result[3]);
      p_this->data_sent_flag = 1;
  }
}

static void
try_to_sleep(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->data_sent_flag = 0;

  bool aux;
  sl_sleeptimer_is_timer_running(p_this->tmr, &aux);
  if (!aux){
    uint32_t timeout = sl_sleeptimer_ms_to_tick(1500);
    sl_sleeptimer_start_timer(p_this->tmr, timeout, sleeptimer_callback, p_this, 1, SL_SLEEPTIMER_NO_HIGH_PRECISION_HF_CLOCKS_REQUIRED_FLAG);
  }

  p_this->sleep_possible_flag = 1;
}

static void
reset_timer_sleep(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->sleep_possible_flag = 0;
  p_this->tmr_flag = 0;

  sl_sleeptimer_stop_timer(p_this->tmr);

  GPIO_EM4EnablePinWakeup(EM4WU_EM4WUEN_MASK << _GPIO_EM4WUEN_EM4WUEN_SHIFT, 0);

  EMU_EnterEM4();
}

static void
reset_no_timer(fsm_t* this){
  app_fsm_t* p_this = this->user_data;
  p_this->sleep_possible_flag = 0;
  p_this->tmr_flag = 0;

  sl_sleeptimer_stop_timer(p_this->tmr);

  EMU_EnterEM4();
}

/*
 * FSM initialization
 */
fsm_t*
new_app_fsm(app_fsm_t* user_data, SPIDRV_Handle_t spi_handle, uint8_t* advertisement_handle){
  // Initialized data
  user_data->sensor_data[0] = 0;
  user_data->sensor_data[1] = 0;
  user_data->sensor_data[2] = 0;
  user_data->sensor_data[3] = 0;

  // Initilize handler
  user_data->spi_handle = spi_handle;
  sl_sleeptimer_timer_handle_t* tmr = malloc(sizeof(sl_sleeptimer_timer_handle_t));
  user_data->tmr = tmr;

  // Initialize flags
  user_data->wakeup_timer_flag = 1; // to activate the FSM
  user_data->wakeup_completed_flag = 0;
  user_data->data_ready_flag = 0;
  user_data->data_retrieved_flag = 0;
  user_data->data_sent_flag = 0;
  user_data->sleep_possible_flag = 0;
  user_data->num_data_retrieved = 0;
  user_data->change_mode_flag = 0;
  user_data->tmr_flag = 0;

  // BLE data
  user_data->advertisement_handle = advertisement_handle;

  // ads1220 init
  user_data->ads1220 = init_ads1220(user_data->spi_handle);

  // GPIO Interruptions
  GPIOINT_CallbackRegister(1, change_mode_callback);
  change_mode_flag = &(user_data->change_mode_flag);
  GPIOINT_CallbackRegister(2, ready_to_retrieve_callback);
  data_ready_flag = &(user_data->data_ready_flag);

  return fsm_new(SLEEPING, app_fsm_tt, user_data);
}

/*
 * Callbacks
 */

static void
change_mode_callback(uint8_t intNo){
  if (intNo == 1){
      *change_mode_flag = !(*change_mode_flag);
  }
}

static void
ready_to_retrieve_callback(uint8_t intNo){
  if (intNo == 2){
      *data_ready_flag = 1;
//      app_log_info("Data ready\n");
  }
}

static void
sleeptimer_callback(sl_sleeptimer_timer_handle_t* handle, void* data){
  app_fsm_t* p_this = data;
  p_this->tmr_flag = 1;
}

/*
 * Auxiliar functions
 */
static void
convertToMicroVStrain(float* result, int32_t data){
  *result = (float) ((data*VFSR*1000000)/FSR);
}

static void
convertToMicroVTemp(float* result, int32_t data){
  *result =  (float)(data >> 10)*0.03125;
}
