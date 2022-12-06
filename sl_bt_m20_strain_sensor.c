/*
 * sl_bt_m20_strain_sensor.c
 *
 *  Created on: 30 nov. 2022
 *      Author: carlo
 */

#include "sl_bt_m20_strain_sensor.h"

sl_status_t sl_bt_torque_send_data(uint8_t* number, size_t size){
  sl_status_t sc;
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_strain_1_characteristic, 0, size/4, number);
  if (sc == SL_STATUS_OK) {
      app_log_info("Attribute written: 0x%f", (float)*number);
    }

  number += size/4;
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_strain_2_characteristic, 0, size/4, number);
    if (sc == SL_STATUS_OK) {
        app_log_info("Attribute written: 0x%f", (float)*number);
      }

  number += size/4;
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_strain_3_characteristic, 0, size/4, number);
    if (sc == SL_STATUS_OK) {
        app_log_info("Attribute written: 0x%f", (float)*number);
      }

  number += size/4;
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_temp_1_characteristic, 0, size/4, number);
    if (sc == SL_STATUS_OK) {
        app_log_info("Attribute written: 0x%f", (float)*number);
      }
  return sc;
}
