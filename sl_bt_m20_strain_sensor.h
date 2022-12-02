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

sl_status_t sl_bt_torque_send_data(uint8_t* number);

#endif /* SL_BT_M20_STRAIN_SENSOR_H_ */
