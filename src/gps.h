/*
 * gps.c
 *
 *  Created on: Dec 18, 2023
 *      Author: jbohn
 */

#ifndef SRC_GPS_C_
#define SRC_GPS_C_

#include "ble.h"

void gps_init();
void gps_state_machine(sl_bt_msg_t *evt);

#endif /* SRC_GPS_C_ */
