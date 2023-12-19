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
#include "em_usart.h"
#include "em_leuart.h"
#include "sl_i2cspm.h"
#include "gpio.h"
#include "scheduler.h"

#define GPIO_ACCEL_INT1_PIN (5) // cross referenced in gpio.c (sry)
#define GPIO_ACCEL_INT2_PIN (4) // cross referenced in gpio.c (sry)

static void LETIMER0_uf_handler();
static void LETIMER0_comp1_handler();
static void GPIO_accel_int1_handler();
static void GPIO_accel_int2_handler();

static uint64_t uf_cnt = 0;

uint64_t irq_timer_UF_cnt_get(){
  return uf_cnt;
}

/**
 * @brief Gets, clears, and process LETIMER0 interrupts
 */
void LETIMER0_IRQHandler(){
  // fetch
  uint32_t flags = LETIMER_IntGetEnabled(LETIMER0);

  // clear
  LETIMER_IntClear(LETIMER0, flags);

  // handle
  if(flags & LETIMER_IF_UF){
    LETIMER0_uf_handler();
  }

  if(flags & LETIMER_IF_COMP1){
    LETIMER0_comp1_handler();
  }
} // LETIMER0_IRQHandler()

/**
 * @brief Processes LETIMER0 UF interrupts, turning LEDs off
 */
static void LETIMER0_uf_handler(){
  uf_cnt += 1;
  sched_set_LETIMER0_uf();
} // LETIMER0_UF_Handler()

/**
 * @brief Processes LETIMER0 COMP1 interrupts, announcing custom timer expiration
 */
static void LETIMER0_comp1_handler(){
  LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
  sched_set_LETIMER0_comp1();
} // LETIMER0_COMP1_Handler()

/**
 * @brief Handles I2C0 interrupts, mostly through a state machine in the i2c API
 */
void I2C0_IRQHandler(){
  I2C_TransferReturn_TypeDef transferStatus;

  transferStatus = I2C_Transfer(I2C0);

  if(transferStatus == i2cTransferDone){
      sched_set_I2C0_transfer_complete();
      NVIC_DisableIRQ(I2C0_IRQn);
      sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
  }

  if(transferStatus < 0){
    LOG_ERROR("%d", (int32_t)transferStatus);
  }
}

void I2C1_IRQHandler(){
  I2C_TransferReturn_TypeDef transferStatus;

  transferStatus = I2C_Transfer(I2C1);

  if(transferStatus == i2cTransferDone){
      sched_set_I2C1_transfer_complete();
      NVIC_DisableIRQ(I2C1_IRQn);
      sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
  }

  if(transferStatus < 0){
    LOG_ERROR("%d", (int32_t)transferStatus);
  }
}

void GPIO_ODD_IRQHandler(){
  // fetch
  uint32_t flags = GPIO_IntGetEnabled();

  // clear
  GPIO_IntClear(flags);

  // handle
  if(flags & (1 << GPIO_ACCEL_INT1_PIN)){
    GPIO_accel_int1_handler();
  }
}

static void GPIO_accel_int1_handler(){
  sched_set_GPIO_accel_act();
}

void GPIO_EVEN_IRQHandler(){
  // fetch
  uint32_t flags = GPIO_IntGetEnabled();

  // clear
  GPIO_IntClear(flags);

  // handle
  if(flags & (1 << GPIO_ACCEL_INT2_PIN)){
    GPIO_accel_int2_handler();
  }
}

static void GPIO_accel_int2_handler(){
  sched_set_GPIO_accel_inact();
}
