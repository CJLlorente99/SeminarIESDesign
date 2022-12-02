/*
 * sl_bt_m20_strain_sensor.c
 *
 *  Created on: 30 nov. 2022
 *      Author: carlo
 */

#include "sl_bt_m20_strain_sensor.h"

sl_status_t sl_bt_torque_send_data(uint8_t* number){
  sl_status_t sc;
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_strain_1_characteristic, 0, 4*sizeof(uint16_t), number);
  if (sc == SL_STATUS_OK) {
      app_log_info("Attribute written: 0x%d", (int)*number);
    }
  return sc;
}
