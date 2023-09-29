/*
 * scheduler.c
 *
 *  Created on: Sep 21, 2023
 *      Author: jbohn
 */

#include "scheduler.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "em_core.h"
#include "i2c.h"

static uint32_t pendingEvents;

/// @brief return enum value of the next event to be processed
/// @return schedEvt_e - next event to be processed
schedEvt_e schedGetNextEvent()
{
  schedEvt_e ret = 0;

  for(int i = 0; i < 32; ++i)
  {
    if(pendingEvents & (1 << i))
    {
      ret = (schedEvt_e) 1 << i;
      break;
    }
  }

  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  pendingEvents &= ~ret;
  CORE_EXIT_CRITICAL();

  return ret;
}

/// @brief set sched flag for LETIMER0_UF event
void schedSetEventLETIMER0_UF()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  pendingEvents |= evtLETIMER0_UF;
  CORE_EXIT_CRITICAL();
} // schedSetEventLETIMER0_UF()

/// @brief set sched flag for LETIMER0_COMP1 event
void schedSetEventLETIMER0_COMP1()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  pendingEvents |= evtLETIMER0_COMP1;
  CORE_EXIT_CRITICAL();
} // schedSetEventLETIMER0_COMP1()

/// @brief set sched flag for I2C0_TransferComplete event
void schedSetEventI2C0_TransferComplete()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  pendingEvents |= evtI2C0_TransferComplete;
  CORE_EXIT_CRITICAL();
} // schedSetEventI2C0_TransferComplete()

typedef enum {
  TEMP_STATE0_IDLE,
  TEMP_STATE1_WARMUP,
  TEMP_STATE2_MEASUREMENT_WRITE,
  TEMP_STATE3_MEASUREMENT_WAIT,
  TEMP_STATE4_RETRIEVE,
  TEMP_NUM_STATES
} tempState_e;

void temperature_state_machine(schedEvt_e evt)
{
  static tempState_e    nextState = TEMP_STATE0_IDLE;
         tempState_e    currentState;

   uint32_t temp_raw = 0;
   int32_t temp_c = 0;

  currentState = nextState;

  switch (currentState)
  {
    case TEMP_STATE0_IDLE:
      nextState = TEMP_STATE0_IDLE;

      if(evt == evtLETIMER0_UF)
      {
        gpioSensorEnableSetOn();
        timerWaitUs_irq(80000);
        nextState = TEMP_STATE1_WARMUP;
      }

      break;
    case TEMP_STATE1_WARMUP:
      nextState = TEMP_STATE1_WARMUP;

      if(evt == evtLETIMER0_COMP1)
      {
        sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
        i2cWrite(I2C_SI7021_ADDR, I2C_SI7021_CMD_MEAURE_TEMP_NO_HOLD);
        nextState = TEMP_STATE2_MEASUREMENT_WRITE;
      }

      break;
    case TEMP_STATE2_MEASUREMENT_WRITE:
      nextState = TEMP_STATE2_MEASUREMENT_WRITE;

      if(evt == evtI2C0_TransferComplete)
      {
        sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
        timerWaitUs_irq(20000);
        nextState = TEMP_STATE3_MEASUREMENT_WAIT;
      }

      break;
    case TEMP_STATE3_MEASUREMENT_WAIT:
      nextState = TEMP_STATE3_MEASUREMENT_WAIT;

      if(evt == evtLETIMER0_COMP1)
      {
        sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
        i2cRead(I2C_SI7021_ADDR, 2);
        nextState = TEMP_STATE4_RETRIEVE;
      }

      break;
    case TEMP_STATE4_RETRIEVE:
      nextState = TEMP_STATE4_RETRIEVE;

      if(evt == evtI2C0_TransferComplete)
      {
        sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
        temp_raw = (i2cGetReadData() >> 8) | ((i2cGetReadData() & 0xFF) << 8);

        gpioSensorEnableSetOff();

        //     round to int   |     do float math
        temp_c = (int32_t) (((175.72*(float)temp_raw)/65536) - 46.85);
        LOG_INFO("Current temp: %d C", temp_c);
        nextState = TEMP_STATE0_IDLE;
      }

      break;
    default:
      LOG_ERROR("Invaid temperature state machine state: %d", (int32_t)currentState);
      break;
  }
}
