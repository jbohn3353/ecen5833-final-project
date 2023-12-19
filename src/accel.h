/*
 * adxl343.h
 *
 *  Created on: Dec 17, 2023
 *      Author: jbohn
 */

#ifndef SRC_ACCEL_H_
#define SRC_ACCEL_H_

#include "ble.h"

void accel_init();
void accel_state_machine(sl_bt_msg_t *evt);

#endif /* SRC_ACCEL_H_ */
