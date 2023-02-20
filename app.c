/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"
#include "fsm.h"
#include "sl_spidrv_instances.h"
#include "m20_strain_ble.h"

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

// Setup beaconing declaration
static void bcn_setup_adv_beaconing(void);

// FSM object
static fsm_t* app_fsm;

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
// SL_WEAK is no more necessary after the method does something
SL_WEAK void app_init(void)
{
  // Unlatsch possible GPIO pads pending after reset
  EMU_UnlatchPinRetention();

  // Config EM4
  EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;
  em4Init.retainUlfrco = false;
  em4Init.retainLfrco = false;
  em4Init.retainLfxo = false;
  em4Init.em4State = emuEM4Shutoff;
  em4Init.pinRetentionMode = emuPinRetentionDisable;
  EMU_EM4Init(&em4Init);

  // Init BURTC
  CMU_ClockSelectSet(cmuClock_EM4GRPACLK, cmuSelect_ULFRCO);
  CMU_ClockEnable(cmuClock_BURTC, true);

  BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
  burtcInit.compare0Top = true; // reset counter when counter reaches compare value
  burtcInit.em4comp = true;     // BURTC compare interrupt wakes from EM4 (causes reset)
  BURTC_Init(&burtcInit);

  BURTC_CounterReset();
  BURTC_CompareSet(0, SLEEPTIME);

  BURTC_IntEnable(BURTC_IEN_COMP);    // compare match
  NVIC_EnableIRQ(BURTC_IRQn);
  BURTC_Enable(true);

  // Initialize GPIO (partially done with the wizard, in autogen)
  GPIO_ExtIntConfig(SL_EMLIB_GPIO_INIT_CHANGEMODE_PORT, SL_EMLIB_GPIO_INIT_CHANGEMODE_PIN, 1, RISINGCHANGEMODE, FALLINGCHANGEMODE, true);
  GPIO_ExtIntConfig(SL_EMLIB_GPIO_INIT_DATAREADY_PORT, SL_EMLIB_GPIO_INIT_DATAREADY_PIN, 2, RISINGMEASUREREADY, FALLINGMEASUREREADY, true);

  // Initialize and create FSM
  app_fsm_t* user_data = malloc(sizeof(app_fsm_t));
  app_fsm = new_app_fsm(user_data, sl_spidrv_exp_handle, &advertising_set_handle);

}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
    fsm_fire(app_fsm);

}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  uint8_t system_id[8];
  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Extract unique ID from BT Address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      app_assert_status(sc);

      // Pad and reverse unique ID to get System ID.
      system_id[0] = address.addr[5];
      system_id[1] = address.addr[4];
      system_id[2] = address.addr[3];
      system_id[3] = 0xFF;
      system_id[4] = 0xFE;
      system_id[5] = address.addr[2];
      system_id[6] = address.addr[1];
      system_id[7] = address.addr[0];

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                   0,
                                                   sizeof(system_id),
                                                   system_id);
      app_assert_status(sc);
      // Initialize iBeacon ADV data.
      bcn_setup_adv_beaconing();
      break;

    // In case RF sense is detected when it shouldn't
    // Maybe can be used to call callback and change retrieval mode
    case sl_bt_evt_system_external_signal_id:
      break;

    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
        break;

    /*  This event indicates that a connection was closed.
        We suppose that mode is continuous, if not, data inside the advertisement packet
        will be changed */
    case sl_bt_evt_connection_closed_id:
      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        32, // min. adv. interval (milliseconds * 1.6)
        32, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert_status(sc);
      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);

    default:
      break;
  }
}

static void bcn_setup_adv_beaconing(void)
{
  sl_status_t sc;

  PACKSTRUCT(struct {
    uint8_t flags_len;     // Length of the Flags field.
    uint8_t flags_type;    // Type of the Flags field.
    uint8_t flags;         // Flags field.
    uint8_t name_len;
    uint8_t name_type;
    uint8_t name[5];
    uint8_t mandata_len;   // Length of the Manufacturer Data field.
    uint8_t mandata_type;  // Type of the Manufacturer Data field.
    uint8_t comp_id[2];    // Company ID field.
    uint8_t strain1[4];
    uint8_t strain2[4];
    uint8_t strain3[4];
    uint8_t temp[4];
  })
  bcn_beacon_adv_data
    = {
    // Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags.
    2,            // Length of field.
    0x01,         // Type of field.
    0x04 | 0x02,  // Flags: LE General Discoverable Mode, BR/EDR is disabled.

    // Add name
    6,           // Length of field
    0x09,         // Type of field "Complete Local Name"
    ADVLOCALNAME,

    // Manufacturer specific data.
    4*sizeof(float)+4,   // Length of field.
    0xFF, // Type of field.

    // The first two data octets shall contain a company identifier code from
    // the Assigned Numbers - Company Identifiers document.
    // 0x0C3F = Not assigned
    { UINT16_TO_BYTES(0x0C3F) },

    // Info from first strain sensor
    { FLOAT_TO_BYTES(0x11111111) },

    // Info from second strain sensor
    { FLOAT_TO_BYTES(0x22222222) },

    // Info from third strain sensor
    { FLOAT_TO_BYTES(0x33333333) },

    // Info from first temperature sensor
    { FLOAT_TO_BYTES(0x44444444) },

    };

  // Create an advertising set.
  sc = sl_bt_advertiser_create_set(&advertising_set_handle);
  app_assert_status(sc);

  // Set custom advertising data.
  sc = sl_bt_legacy_advertiser_set_data(advertising_set_handle,
                                        0,
                                        sizeof(bcn_beacon_adv_data),
                                        (uint8_t *)(&bcn_beacon_adv_data));
  app_assert_status(sc);

  // Set advertising parameters. 10ms advertisement interval.
  sc =  (
    advertising_set_handle,
    32,     // min. adv. interval (milliseconds * 1.6) (min is 20 ms)
    32,     // max. adv. interval (milliseconds * 1.6) (min is 20 ms)
    0,       // adv. duration
    0);      // max. num. adv. events
  app_assert_status(sc);

  // Start advertising in user mode and disable connections.
  sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                     sl_bt_advertiser_connectable_scannable);
  app_assert_status(sc);
}
