
#include "em_cmu.h"
#include "app.h"

#include "oscillators.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#if LOWEST_ENERGY_MODE == 3
#define LETIMER0_OSC    ((CMU_Osc_TypeDef)      (cmuOsc_ULFRCO))
#define LETIMER0_REF    ((CMU_Select_TypeDef)   (cmuSelect_ULFRCO))
#else
#define LETIMER0_OSC    ((CMU_Osc_TypeDef)      (cmuOsc_LFXO))
#define LETIMER0_REF    ((CMU_Select_TypeDef)   (cmuSelect_LFXO))
#endif

void oscillatorsInit()
{
  CMU_OscillatorEnable(LETIMER0_OSC, true, true);

  CMU_ClockSelectSet(cmuClock_LFA, LETIMER0_REF);
  CMU_ClockEnable(cmuClock_LFA, true);

  CMU_ClockDivSet(cmuClock_LETIMER0, LETIMER_PRESCALER_VAL);
  CMU_ClockEnable(cmuClock_LETIMER0, true);
} // oscillatorsInit()
