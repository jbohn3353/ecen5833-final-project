/**
 * @file      timers.h
 * @brief     Declares functions to manage timers for the EFM32
 * @author    James Bohn
 * @date      Sep 15, 2023
 */

#ifndef SRC_TIMERS_H
#define SRC_TIMERS_H

#include <stdint.h>

#define LETIMER_ON_TIME_MS      (175)
#define LETIMER_PERIOD_MS       (10000)

// Function Prototypes
void timer0_init();
void timer_enable();
void timer_disable();
void timer_wait_us_polled(uint32_t us_wait);
void timer_wait_us_irq(uint32_t us_wait);

uint64_t timer_milliseconds();

#endif /* SRC_TIMERS_H */
