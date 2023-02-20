/*
 * sl_bt_m20_strain_sensor.h
 *
 *  Created on: 30 nov. 2022
 *      Author: Carlos Llorente Cortijo
 *      Technische Universität Darmstadt
 *      ETIT-IES
 */

#ifndef SL_BT_M20_STRAIN_SENSOR_H_
#define SL_BT_M20_STRAIN_SENSOR_H_

#include "gatt_db.h"
#include "sl_bluetooth.h"
#include "app_log.h"
#include "app_assert.h"
#include "mbedtls/cipher.h"
#include "mbedtls/md.h"
#include "mbedtls/platform_util.h"

/**************************************************************************//**
 * Macros.
 ************************************************************************/
#define FLOAT_TO_BYTES(n)            ((uint8_t) (n), (uint8_t)((n) >> 8), (uint8_t)((n) >> 16), (uint8_t)((n) >> 24))
#define FLOAT_TO_BYTE0(n)            ((uint8_t) (n))
#define FLOAT_TO_BYTE1(n)            ((uint8_t) ((n) >> 8))
#define FLOAT_TO_BYTE2(n)            ((uint8_t) ((n) >> 16))
#define FLOAT_TO_BYTE3(n)            ((uint8_t) ((n) >> 24))

#define UINT16_TO_BYTES(n)            ((uint8_t) (n), (uint8_t)((n) >> 8))
#define UINT16_TO_BYTE0(n)            ((uint8_t) (n))
#define UINT16_TO_BYTE1(n)            ((uint8_t) ((n) >> 8))

#define UINT128_TO_BYTES(n)           ((uint8_t) (n), (uint8_t)((n) >> 8), (uint8_t)((n) >> 16), (uint8_t)((n) >> 24), (uint8_t)((n) >> 32), (uint8_t)((n) >> 40), (uint8_t)((n) >> 48), (uint8_t)((n) >> 56), (uint8_t)((n) >> 64), (uint8_t)((n) >> 72), (uint8_t)((n) >> 80), (uint8_t)((n) >> 88), (uint8_t)((n) >> 96), (uint8_t)((n) >> 104), (uint8_t)((n) >> 112), (uint8_t)((n) >> 120))

/**************************************************************************//**
 * Constants.
 ************************************************************************/
#define ADVLOCALNAME0 0x4d
#define ADVLOCALNAME1 0x32
#define ADVLOCALNAME2 0x30
#define ADVLOCALNAME3 0x5f
#define ADVLOCALNAME4 0x31

#define KEYVALUE1 0x66556a58
#define KEYVALUE2 0x6e327235
#define KEYVALUE3 0x66556a58
#define KEYVALUE4 0x6e327235

/**************************************************************************//**
 * Type declaration.
 ************************************************************************/
typedef struct adv_format_s adv_format_t;

/**************************************************************************//**
 * Function declaration.
 ************************************************************************/
sl_status_t sl_bt_torque_send_data(uint32_t* number, uint8_t mode, uint8_t* advertisement_handle, mbedtls_cipher_context_t* cipher);
sl_status_t sl_bt_mode_read(bool* res);
sl_status_t sl_bt_mode_continuous();
sl_status_t sl_bt_mode_periodic();
void sensorDataAESEncrypt(uint32_t* data, size_t size, uint32_t* output, mbedtls_cipher_context_t* cipher);

/**************************************************************************//**
 * Type definition.
 ************************************************************************/
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
};

#endif /* SL_BT_M20_STRAIN_SENSOR_H_ */
