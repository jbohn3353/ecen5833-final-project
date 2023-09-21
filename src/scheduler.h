/*
 * scheduler.h
 *
 *  Created on: Sep 21, 2023
 *      Author: jbohn
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

// Put in order for priority, highest prio = lowest value
typedef enum {
  evtLETIMER0_UF = 1,
  evtLETIMER0_COMP1 = 2,
  evtC = 4,
  evtD = 8,
} schedEvt_e;

schedEvt_e schedGetNextEvent();
void schedSetEventLETIMER0_UF();
void schedSetEventLETIMER0_COMP1();


#endif /* SRC_SCHEDULER_H_ */
