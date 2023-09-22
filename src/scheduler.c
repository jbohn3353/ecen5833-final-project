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

