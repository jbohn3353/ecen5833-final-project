/**
 * @file      irq.c
 * @brief     Defines interrupt handlers for the EFM32
 * @author    James Bohn
 * @date      Sep 15, 2023
 */

#include "irq.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "em_letimer.h"
#include "sl_i2cspm.h"
#include "gpio.h"
#include "scheduler.h"

static void LETIMER0_UF_Handler();
static void LETIMER0_COMP1_Handler();

static uint64_t uf_cnt = 0;

uint64_t irqTimerUFCntGet()
{
  return uf_cnt;
}

/**
 * @brief Gets, clears, and process LETIMER0 interrupts
 */
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
} // LETIMER0_IRQHandler()

/**
 * @brief Processes LETIMER0 UF interrupts, turning LEDs off
 */
static void LETIMER0_UF_Handler()
{
  uf_cnt += 1;
  schedSetEventLETIMER0_UF();
} // LETIMER0_UF_Handler()

/**
 * @brief Processes LETIMER0 COMP1 interrupts, announcing custom timer expiration
 */
static void LETIMER0_COMP1_Handler()
{
  LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
  schedSetEventLETIMER0_COMP1();
} // LETIMER0_COMP1_Handler()

/**
 * @brief Handles I2C0 interrupts, mostly through a state machine in the i2c API
 */
void I2C0_IRQHandler()
{
  I2C_TransferReturn_TypeDef transferStatus;

  transferStatus = I2C_Transfer(I2C0);

  if(transferStatus == i2cTransferDone)
  {
    schedSetEventI2C0_TransferComplete();
  }

  if(transferStatus < 0)
  {
    LOG_ERROR("%d", (int32_t)transferStatus);
  }
}
