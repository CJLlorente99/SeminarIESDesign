/*
 * sl_bt_m20_strain_sensor.c
 *
 *  Created on: 30 nov. 2022
 *      Author: Carlos Llorente Cortijo
 *      Technische Universität Darmstadt
 *      ETIT-IES
 */

#include "sl_bt_m20_strain_sensor.h"

uint32_t key[4];

/**************************************************************************//**
 * Sending function.
 ************************************************************************/
sl_status_t sl_bt_torque_send_data(uint32_t* number, uint8_t mode, uint8_t* advertisement_handle, mbedtls_cipher_context_t* cipher){
  sl_status_t sc;

  // Possibly encrypt data
  uint32_t* encryptedNum = number;
  // uint32_t encryptedNum[4];
  // sensorDataAESEncrypt(number, 4*sizeof(uint32_t), encryptedNum, cipher);

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

    adv.mandata_len = (uint8_t)4*sizeof(float)+3;
    adv.mandata_type = 0xFF;


    adv.comp_id[0] = UINT16_TO_BYTE0(0x0C3F);
    adv.comp_id[1] = UINT16_TO_BYTE1(0x0C3F);

    adv.strain1[0] = FLOAT_TO_BYTE0(encryptedNum[0]);
    adv.strain1[1] = FLOAT_TO_BYTE1(encryptedNum[0]);
    adv.strain1[2] = FLOAT_TO_BYTE2(encryptedNum[0]);
    adv.strain1[3] = FLOAT_TO_BYTE3(encryptedNum[0]);

    adv.strain2[0] = FLOAT_TO_BYTE0(encryptedNum[1]);
    adv.strain2[1] = FLOAT_TO_BYTE1(encryptedNum[1]);
    adv.strain2[2] = FLOAT_TO_BYTE2(encryptedNum[1]);
    adv.strain2[3] = FLOAT_TO_BYTE3(encryptedNum[1]);

    adv.strain3[0] = FLOAT_TO_BYTE0(encryptedNum[2]);
    adv.strain3[1] = FLOAT_TO_BYTE1(encryptedNum[2]);
    adv.strain3[2] = FLOAT_TO_BYTE2(encryptedNum[2]);
    adv.strain3[3] = FLOAT_TO_BYTE3(encryptedNum[2]);

    adv.temp[0] = FLOAT_TO_BYTE0(encryptedNum[3]);
    adv.temp[1] = FLOAT_TO_BYTE1(encryptedNum[3]);
    adv.temp[2] = FLOAT_TO_BYTE2(encryptedNum[3]);
    adv.temp[3] = FLOAT_TO_BYTE3(encryptedNum[3]);

    sc = sl_bt_legacy_advertiser_set_data(*advertisement_handle, sl_bt_advertiser_advertising_data_packet,
                                     sizeof(adv_format_t), (uint8_t*)&adv);


  } else{ // If not periodic, normal advertisement and transmission via connection

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(*advertisement_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Fill GATT values
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_data, 0, sizeof(float)*4, (uint8_t*) encryptedNum);
      if (sc == SL_STATUS_OK) {
          app_log_info("Attribute written: 0x%hu\n", number[0]);
        }
      sc = sl_bt_gatt_server_notify_all(gattdb_data, sizeof(float)*4, (uint8_t*) encryptedNum);
      if (sc == SL_STATUS_OK) {
          app_log_append(" Notification sent: 0x%hu\n", number[0]);
        }
}
  return sc;
}

/**************************************************************************//**
 * Encryption function.
 ************************************************************************/
void
sensorDataAESEncrypt(uint32_t* data, size_t size, uint32_t* output, mbedtls_cipher_context_t* cipher){
  size_t olen = size;
  key[0] = KEYVALUE1;
  key[1] = KEYVALUE2;
  key[2] = KEYVALUE3;
  key[3] = KEYVALUE4;

  // Not working at the moment
  int res;
  res = mbedtls_cipher_setkey(cipher, (uint8_t*) key, 128, MBEDTLS_ENCRYPT);
  res = mbedtls_cipher_reset(cipher);
  res = mbedtls_cipher_update(cipher, (uint8_t*) data, size, (uint8_t*) output, &olen);
  res = mbedtls_cipher_finish(cipher, (uint8_t*) output, &olen);
}
