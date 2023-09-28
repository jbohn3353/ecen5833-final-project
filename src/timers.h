/**
 * @file      timers.h
 * @brief     Declares functions to manage timers for the EFM32
 * @author    James Bohn
 * @date      Sep 15, 2023
 */

#ifndef SRC_TIMERS_H
#define SRC_TIMERS_H

#include <stdint.h>

// Function Prototypes
void initLETIMER0();
void timerWaitUs_polled(uint32_t us_wait);
void timerWaitUs_irq(uint32_t us_wait);

#endif /* SRC_TIMERS_H */
