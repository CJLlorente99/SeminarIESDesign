/*
 * sl_bt_m20_strain_sensor.h
 *
 *  Created on: 30 nov. 2022
 *      Author: carlo
 */

#ifndef SL_BT_M20_STRAIN_SENSOR_H_
#define SL_BT_M20_STRAIN_SENSOR_H_

#include "gatt_db.h"
#include "sl_bluetooth.h"
#include "app_log.h"
#include "app_assert.h"

// Macros.
#define FLOAT_TO_BYTES(n)            ((uint8_t) (n), (uint8_t)((n) >> 8), (uint8_t)((n) >> 16), (uint8_t)((n) >> 24))
#define FLOAT_TO_BYTE0(n)            ((uint8_t) (n))
#define FLOAT_TO_BYTE1(n)            ((uint8_t) ((n) >> 8))
#define FLOAT_TO_BYTE2(n)            ((uint8_t) ((n) >> 16))
#define FLOAT_TO_BYTE3(n)            ((uint8_t) ((n) >> 24))

#define UINT16_TO_BYTES(n)            ((uint8_t) (n), (uint8_t)((n) >> 8))
#define UINT16_TO_BYTE0(n)            ((uint8_t) (n))
#define UINT16_TO_BYTE1(n)            ((uint8_t) ((n) >> 8))

#define ADVLOCALNAME0 0x4d
#define ADVLOCALNAME1 0x32
#define ADVLOCALNAME2 0x30
#define ADVLOCALNAME3 0x5f
#define ADVLOCALNAME4 0x31


typedef struct adv_format_s adv_format_t;

sl_status_t sl_bt_torque_send_data(uint32_t number[4], uint8_t mode, uint8_t* advertisement_handle);

/*
 * Advertisement data format
 */
struct adv_format_s {
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
    uint8_t mode;
};

#endif /* SL_BT_M20_STRAIN_SENSOR_H_ */
