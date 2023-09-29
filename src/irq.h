/**
 * @file      irq.h
 * @brief     Defines interrupt handlers for the EFM32
 * @author    James Bohn
 * @date      Sep 15, 2023
 */

#ifndef SRC_IRQ_H
#define SRC_IRQ_H

#include <stdint.h>

// Function Prototypes
void LETIMER0_IRQHandler();
void I2C0_IRQHandler();

uint64_t irqTimerUFCntGet();

#endif /* SRC_IRQ_H */
