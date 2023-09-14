
#include "em_letimer.h"
#include "timers.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "app.h"

#if LOWEST_ENERGY_MODE == 3
#define BASE_CLOCK_FRQ    (1000)
#else
#define BASE_CLOCK_FRQ    (32768)
#endif

#define ACTUAL_CLOCK_FRQ  BASE_CLOCK_FRQ/LETIMER_PRESCALER_VAL

void initLETIMER0()
{
  LETIMER_Init_TypeDef letimer_init;
  letimer_init.enable = false;              // start off
  letimer_init.debugRun = false;             // do not run while paused
  letimer_init.comp0Top = true;             // use comp0 to load on UF
  letimer_init.bufTop = false;              // dont use comp1 to change comp0

  letimer_init.out0Pol = 0;                 // timer outputs dont matter
  letimer_init.out1Pol = 0;
  letimer_init.ufoa0 = letimerUFOANone;
  letimer_init.ufoa1 = letimerUFOANone;

  letimer_init.repMode = letimerRepeatFree; // repeat forever
  letimer_init.topValue = 0;                // start counter @ 0

  LETIMER_Init(LETIMER0, &letimer_init);

  LETIMER_CompareSet(LETIMER0, 0, ACTUAL_CLOCK_FRQ*LETIMER_PERIOD_MS/1000);
  LETIMER_CompareSet(LETIMER0, 1, ACTUAL_CLOCK_FRQ*LETIMER_ON_TIME_MS/1000);

  LETIMER_IntClear(LETIMER0, 0xFFFFFFFF);
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1 | LETIMER_IEN_UF);

  LETIMER_Enable(LETIMER0, true);
} // initLETIMER0()
