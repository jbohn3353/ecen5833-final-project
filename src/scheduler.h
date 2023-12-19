/**
 * @file      scheduler.h
 * @brief     Declares functions to our scheduler for the EFM32
 * @author    James Bohn
 * @date      Sep 20, 2023
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

typedef enum {
  evtNone                   = 0,
  evtLETIMER0_UF            = (1 << 0),
  evtLETIMER0_COMP1         = (1 << 1),
  evtI2C0_TransferComplete  = (1 << 2),
  evtI2C1_TransferComplete  = (1 << 3),
  evtGPIO_Accel_Act         = (1 << 4),
  evtGPIO_Accel_Inact       = (1 << 5),
  evtLEUART0_frame          = (1 << 6),
  evtLEUART0_delim          = (1 << 7),
  evtLEUART0_txc            = (1 << 8),
} schedEvt_e;

void sched_set_LETIMER0_uf();
void sched_set_LETIMER0_comp1();
void sched_set_I2C0_transfer_complete();
void sched_set_I2C1_transfer_complete();
void sched_set_GPIO_accel_act();
void sched_set_GPIO_accel_inact();
void sched_set_LEUART0_frame();
void sched_set_LEUART0_delim();
void sched_set_LEUART0_txc();

#endif /* SRC_SCHEDULER_H_ */
