/*
 * sl_bt_m20_strain_sensor.c
 *
 *  Created on: 30 nov. 2022
 *      Author: carlo
 */

#include "sl_bt_m20_strain_sensor.h"

sl_status_t sl_bt_torque_send_data(uint32_t number[4], uint8_t mode, uint8_t* advertisement_handle){
  sl_status_t sc;

  // If we are in periodic mode, we want to use advertisement packet as means of transmitting data
  if (mode == 0){
    // Create and fill advertisement packet
    adv_format_t adv;

    adv.flags_len = (uint8_t)2;
    adv.flags_type = 0x01;
    adv.flags = 0x04 | 0x02;

    adv.name_len = 6;
    adv.name_type = 0x09;
    adv.name[0] = ADVLOCALNAME0;
    adv.name[1] = ADVLOCALNAME1;
    adv.name[2] = ADVLOCALNAME2;
    adv.name[3] = ADVLOCALNAME3;
    adv.name[4] = ADVLOCALNAME4;

    adv.mandata_len = (uint8_t)4*sizeof(float)+4;
    adv.mandata_type = 0xFF;


    adv.comp_id[0] = UINT16_TO_BYTE0(0x0C3F);
    adv.comp_id[1] = UINT16_TO_BYTE1(0x0C3F);

    adv.strain1[0] = FLOAT_TO_BYTE0(number[0]);
    adv.strain1[1] = FLOAT_TO_BYTE1(number[0]);
    adv.strain1[2] = FLOAT_TO_BYTE2(number[0]);
    adv.strain1[3] = FLOAT_TO_BYTE3(number[0]);

    adv.strain2[0] = FLOAT_TO_BYTE0(number[1]);
    adv.strain2[1] = FLOAT_TO_BYTE1(number[1]);
    adv.strain2[2] = FLOAT_TO_BYTE2(number[1]);
    adv.strain2[3] = FLOAT_TO_BYTE3(number[1]);

    adv.strain3[0] = FLOAT_TO_BYTE0(number[2]);
    adv.strain3[1] = FLOAT_TO_BYTE1(number[2]);
    adv.strain3[2] = FLOAT_TO_BYTE2(number[2]);
    adv.strain3[3] = FLOAT_TO_BYTE3(number[2]);

    adv.temp[0] = FLOAT_TO_BYTE0(number[3]);
    adv.temp[1] = FLOAT_TO_BYTE1(number[3]);
    adv.temp[2] = FLOAT_TO_BYTE2(number[3]);
    adv.temp[3] = FLOAT_TO_BYTE3(number[3]);

    adv.mode = mode;

    sc = sl_bt_legacy_advertiser_set_data(*advertisement_handle, sl_bt_advertiser_advertising_data_packet,
                                     sizeof(adv_format_t), (uint8_t*)&adv);


//                                       sl_bt_advertiser_connectable_scannable);
  } else{ // If not periodic, normal advertisement

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(*advertisement_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Fill GATT values
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_strain_1, 0, sizeof(float), &number[0]);
      if (sc == SL_STATUS_OK) {
          app_log_info("Attribute written: 0x%hu\n", number[0]);
        }
      sc = sl_bt_gatt_server_notify_all(gattdb_strain_1, sizeof(float), &number[0]);
      if (sc == SL_STATUS_OK) {
          app_log_append(" Notification sent: 0x%hu\n", number[0]);
        }

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_strain_2, 0, sizeof(float), &number[1]);
      if (sc == SL_STATUS_OK) {
          app_log_info("Attribute written: 0x%hu\n", number[1]);
        }
      sc = sl_bt_gatt_server_notify_all(gattdb_strain_2, sizeof(float), &number[1]);
      if (sc == SL_STATUS_OK) {
          app_log_append(" Notification sent: 0x%hu\n", number[1]);
        }

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_strain_3, 0, sizeof(float), &number[2]);
      if (sc == SL_STATUS_OK) {
          app_log_info("Attribute written: 0x%hu\n", number[2]);
        }
      sc = sl_bt_gatt_server_notify_all(gattdb_strain_3, sizeof(float), &number[2]);
      if (sc == SL_STATUS_OK) {
          app_log_append(" Notification sent: 0x%hu\n", number[2]);
        }

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_temp_1, 0, sizeof(float), &number[3]);
      if (sc == SL_STATUS_OK) {
          app_log_info("Attribute written: 0x%hu\n", number[3]);
        }
      sc = sl_bt_gatt_server_notify_all(gattdb_temp_1, sizeof(float), &number[3]);
      if (sc == SL_STATUS_OK) {
          app_log_append(" Notification sent: 0x%hu\n", number[3]);
        }
  }
  return sc;
}
