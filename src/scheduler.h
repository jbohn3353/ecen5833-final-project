/**
 * @file      scheduler.h
 * @brief     Declares functions to our scheduler for the EFM32
 * @author    James Bohn
 * @date      Sep 20, 2023
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

#include "sl_bt_api.h"


typedef enum {
  evtNone = 0,
  evtLETIMER0_UF = 1,
  evtLETIMER0_COMP1 = 2,
  evtI2C0_TransferComplete = 4,

} schedEvt_e;

void schedSetEventLETIMER0_UF();
void schedSetEventLETIMER0_COMP1();
void schedSetEventI2C0_TransferComplete();

void run_state_machines(sl_bt_msg_t *evt);

#endif /* SRC_SCHEDULER_H_ */
