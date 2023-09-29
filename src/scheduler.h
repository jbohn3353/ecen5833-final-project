/**
 * @file      scheduler.h
 * @brief     Declares functions to our scheduler for the EFM32
 * @author    James Bohn
 * @date      Sep 20, 2023
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

// Put in order for priority, highest prio = lowest value
typedef enum {
  evtNone = 0,
  evtLETIMER0_UF = 1,
  evtLETIMER0_COMP1 = 2,
  evtI2C0_TransferComplete = 4,

} schedEvt_e;

schedEvt_e schedGetNextEvent();
void schedSetEventLETIMER0_UF();
void schedSetEventLETIMER0_COMP1();
void schedSetEventI2C0_TransferComplete();

void temperature_state_machine(schedEvt_e evt);

#endif /* SRC_SCHEDULER_H_ */
