/*
 * sl_bt_m20_strain_sensor.c
 *
 *  Created on: 30 nov. 2022
 *      Author: carlo
 */

#include "sl_bt_m20_strain_sensor.h"


sl_status_t sl_bt_torque_send_data(uint32_t number[4], uint8_t* advertisement_handle){
  sl_status_t sc;

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

  sc = sl_bt_legacy_advertiser_set_data(*advertisement_handle, sl_bt_advertiser_advertising_data_packet,
                                   sizeof(adv_format_t), (uint8_t*)&adv);

  return sc;
}

/*
 * Reversing function
 */

void reverse(uint8_t* number, size_t size, uint8_t* rebmun){
  for (unsigned int ix = 0; ix < size; ix++) rebmun[ix] = number[size - ix - 1];
}
