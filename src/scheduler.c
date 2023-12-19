/**
 * @file      scheduler.h
 * @brief     Defines functions to our scheduler for the EFM32
 * @author    James Bohn
 * @date      Sep 20, 2023
 */

#include "scheduler.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "em_core.h"
#include "sl_bt_api.h"

/// @brief set sched flag for LETIMER0_UF event
void sched_set_LETIMER0_uf(){
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtLETIMER0_UF);
  CORE_EXIT_CRITICAL();
} // schedSetEventLETIMER0_UF()

/// @brief set sched flag for LETIMER0_COMP1 event
void sched_set_LETIMER0_comp1(){
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtLETIMER0_COMP1);
  CORE_EXIT_CRITICAL();
} // schedSetEventLETIMER0_COMP1()

/// @brief set sched flag for I2C0_TransferComplete event
void sched_set_I2C0_transfer_complete(){
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtI2C0_TransferComplete);
  CORE_EXIT_CRITICAL();
} // schedSetEventI2C0_TransferComplete()

/// @brief set sched flag for I2C1_TransferComplete event
void sched_set_I2C1_transfer_complete(){
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtI2C1_TransferComplete);
  CORE_EXIT_CRITICAL();
} // schedSetEventI2C0_TransferComplete()


void sched_set_GPIO_accel_act(){
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtGPIO_Accel_Act);
  CORE_EXIT_CRITICAL();
}

void sched_set_GPIO_accel_inact(){
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtGPIO_Accel_Inact);
  CORE_EXIT_CRITICAL();
}

void sched_set_LEUART0_frame(){
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtLEUART0_frame);
  CORE_EXIT_CRITICAL();
}

void sched_set_LEUART0_delim(){
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtLEUART0_delim);
  CORE_EXIT_CRITICAL();
}

void sched_set_LEUART0_txc(){
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtLEUART0_txc);
  CORE_EXIT_CRITICAL();
}
