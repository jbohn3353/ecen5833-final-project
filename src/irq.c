
#include "em_letimer.h"
#include "gpio.h"

#include "irq.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

static void LETIMER0_UF_Handler();
static void LETIMER0_COMP1_Handler();

void LETIMER0_IRQHandler()
{
  // fetch
  uint32_t flags = LETIMER_IntGetEnabled(LETIMER0);

  // clear
  LETIMER_IntClear(LETIMER0, flags);

  // handle
  if(flags & LETIMER_IF_UF)
  {
      LETIMER0_UF_Handler();
  }

  if(flags & LETIMER_IF_COMP1)
  {
      LETIMER0_COMP1_Handler();
  }
}

static void LETIMER0_UF_Handler()
{
  gpioLed0SetOff();
  gpioLed1SetOff();
}

static void LETIMER0_COMP1_Handler()
{
  gpioLed0SetOn();
  gpioLed1SetOn();
}
