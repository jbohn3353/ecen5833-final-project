/*
 * si7021.h
 *
 *  Created on: Dec 18, 2023
 *      Author: jbohn
 */

#ifndef SRC_TRH_H_
#define SRC_TRH_H_

#include "ble.h"

void trh_init();
void trh_state_machine(sl_bt_msg_t *evt);

#endif /* SRC_TRH_H_ */
