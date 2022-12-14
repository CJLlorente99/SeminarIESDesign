/*
 * sl_bt_m20_strain_sensor.c
 *
 *  Created on: 30 nov. 2022
 *      Author: carlo
 */

#include "sl_bt_m20_strain_sensor.h"

void reverse(uint8_t* number, size_t size, uint8_t* rebmun);

sl_status_t sl_bt_torque_send_data(uint8_t* number, size_t size){
  sl_status_t sc;
  uint8_t rebmun[size/4];

  reverse(number, size/4, rebmun);
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_strain_1_characteristic, 0, size/4, rebmun);
  if (sc == SL_STATUS_OK) {
      app_log_info("Attribute written: 0x%hu\n", (uint16_t)*number);
    }
  sc = sl_bt_gatt_server_notify_all(gattdb_strain_1_characteristic, size/4, rebmun);
  if (sc == SL_STATUS_OK) {
      app_log_append(" Notification sent: 0x%hu\n", (uint16_t)*number);
    }

  number += size/4;
  reverse(number, size/4, rebmun);
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_strain_2_characteristic, 0, size/4, rebmun);
  if (sc == SL_STATUS_OK) {
      app_log_info("Attribute written: 0x%hu\n", (uint16_t)*number);
    }
  sc = sl_bt_gatt_server_notify_all(gattdb_strain_2_characteristic, size/4, rebmun);
  if (sc == SL_STATUS_OK) {
      app_log_append(" Notification sent: 0x%hu\n", (uint16_t)*number);
    }

  number += size/4;
  reverse(number, size/4, rebmun);
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_strain_3_characteristic, 0, size/4, rebmun);
  if (sc == SL_STATUS_OK) {
      app_log_info("Attribute written: 0x%hu\n", (uint16_t)*number);
    }
  sc = sl_bt_gatt_server_notify_all(gattdb_strain_3_characteristic, size/4, rebmun);
  if (sc == SL_STATUS_OK) {
      app_log_append(" Notification sent: 0x%hu\n", (uint16_t)*number);
    }

  number += size/4;
  reverse(number, size/4, rebmun);
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_temp_1_characteristic, 0, size/4, rebmun);
  if (sc == SL_STATUS_OK) {
      app_log_info("Attribute written: 0x%hu\n", (uint16_t)*number);
    }
  sc = sl_bt_gatt_server_notify_all(gattdb_temp_1_characteristic, size/4, rebmun);
  if (sc == SL_STATUS_OK) {
      app_log_append(" Notification sent: 0x%hu\n", (uint16_t)*number);
    }

  return sc;
}

/*
 * Reversing function
 */

void reverse(uint8_t* number, size_t size, uint8_t* rebmun){
  for (unsigned int ix = 0; ix < size; ix++) rebmun[ix] = number[size - ix - 1];
}
