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
void I2C1_IRQHandler();
void GPIO_EVEN_IRQHandler();
void GPIO_ODD_IRQHandler();
void LEUART0_IRQHandler();

uint64_t irq_timer_UF_cnt_get();

#endif /* SRC_IRQ_H */
