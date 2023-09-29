/**
 * @file      timers.c
 * @brief     Initializes and manages timers for the EFM32, (minus interrupt handlers)
 * @author    James Bohn
 * @date      Sep 15, 2023
 */

#include "timers.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "em_letimer.h"
#include "app.h"
#include "irq.h"

#if LOWEST_ENERGY_MODE == 3
#define BASE_CLOCK_FRQ      (1000)
#else
#define BASE_CLOCK_FRQ      (32768)
#endif

#define ACTUAL_CLOCK_FRQ    (BASE_CLOCK_FRQ/LETIMER_PRESCALER_VAL)

#define MAX_TIMER_WAIT_POLLED_US    (8000000)
#define MAX_TIMER_WAIT_IRQ_US       (LETIMER_PERIOD_MS * 1000)

/**
 * @brief Initialize LETIMER0, should be called during system startup
 */
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
//  LETIMER_CompareSet(LETIMER0, 1, ACTUAL_CLOCK_FRQ*LETIMER_ON_TIME_MS/1000);

  LETIMER_IntClear(LETIMER0, 0xFFFFFFFF);
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);

  LETIMER_Enable(LETIMER0, true);
} // initLETIMER0()

/// @brief converts a number of microseconds to timer ticks
/// @param uint32_t us - microseconds to convert
/// @return uint32_t - number of ticks associated with us, rounded up
static uint32_t timerUsToTicks(uint32_t us)
{
  // cast back to small | cast up since mult. can be big |   round up
  return (uint32_t) ((((uint64_t) us * ACTUAL_CLOCK_FRQ) + (1000000-1))/ 1000000);
} // timerUsToTicks(uint32_t us)

/// @brief use polling to delay for a specified number of us
/// @param uint32_t us_wait - number of microseconds to wait
void timerWaitUs_polled(uint32_t us_wait)
{
  // clamp values too large
  if(us_wait > MAX_TIMER_WAIT_POLLED_US){
      us_wait = MAX_TIMER_WAIT_POLLED_US;
  }
  else if(us_wait == 0)
  {
      return;
  }

  uint16_t new_cnt, diff;
  uint32_t ticks_to_wait = timerUsToTicks(us_wait);
  uint16_t prev_cnt = LETIMER_CounterGet(LETIMER0);

  while(1)
  {
    new_cnt = LETIMER_CounterGet(LETIMER0);

    if(new_cnt == prev_cnt)
    {
      continue;
    }

    // handler underflow
    if(new_cnt > prev_cnt)
    {
      //    ticks bf uf |              ticks after uf             | count the actual uf as a tick
      diff = prev_cnt + (LETIMER_CompareGet(LETIMER0, 0) - new_cnt) + 1;
    }
    else
    {
      diff =  prev_cnt - new_cnt;
    }

    if(diff >= ticks_to_wait)
    {
      return;
    }
    else
    {
      ticks_to_wait -= diff;
      prev_cnt = new_cnt;
    }
  }
} // timerWaitUs(uint32_t us_wait)

/// @brief delay for a specified number of us, using an interrupt to wakeup and
///          set an event flag
/// @param uint32_t us_wait - number of microseconds to wait
void timerWaitUs_irq(uint32_t us_wait)
{

  // clamp values too large
  if(us_wait > MAX_TIMER_WAIT_IRQ_US)
  {
      us_wait = MAX_TIMER_WAIT_IRQ_US;
  }
  else if(us_wait == 0)
  {
      return;
  }


  uint32_t ticks_to_wait = timerUsToTicks(us_wait);
  uint16_t cur_cnt = LETIMER_CounterGet(LETIMER0);
  uint16_t int_cnt;

  // Handle underflow
  if(ticks_to_wait > cur_cnt)
  {
    //               cur reload val           |     ticks after uf      | count uf as tick
    int_cnt = LETIMER_CompareGet(LETIMER0, 0) - (ticks_to_wait - cur_cnt) + 1;
  }
  else
  {
    int_cnt = cur_cnt - ticks_to_wait;
  }

  LETIMER_CompareSet(LETIMER0, 1, int_cnt);
  LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP1);
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
}

/// @brief get/calculate number of milliseconds since systems startup
/// @return uint64_t - number of milliseconds since systems startup
uint64_t timerMilliseconds()
{
  uint32_t ticks_since_uf = LETIMER_CompareGet(LETIMER0, 0) - LETIMER_CounterGet(LETIMER0);
  //                  time from UFs               |               time since last UF
  return (irqTimerUFCntGet() * LETIMER_PERIOD_MS) + (ticks_since_uf * 1000)/ACTUAL_CLOCK_FRQ;
}
