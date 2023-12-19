/*
  gpio.c
 
   Created on: Dec 12, 2018
       Author: Dan Walkes
   Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.
   Updated by James Bohn Sep 1, 2023. Completed assignment 1 of ECEN 5823

   March 17
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.
   
   Jan 24, 2023
   Dave Sluiter: Cleaned up gpioInit() to make it less confusing for students regarding
                 drive strength setting. 

 *
 * @student    James Bohn, james.bohn@colorado.edu
 *
 
 */

#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>

#include "gpio.h"
#include "em_cmu.h"

#define LED_PORT            (gpioPortC)
#define LED_PIN             (0x6)

#define SENSOR_ENABLE_PORT  (gpioPortF)
#define SENSOR_ENABLE_PIN   (7)

#define GEN_GPIO_PORT       (gpioPortB)
#define GEN_GPIO1_PIN       (11)
#define GEN_GPIO2_PIN       (12)
#define GEN_GPIO3_PIN       (13)

#define GPIO_ACCEL_PORT     (gpioPortA)
#define GPIO_ACCEL_INT1_PIN (5) // cross referenced in irq.c (sry)
#define GPIO_ACCEL_INT2_PIN (4) // cross referenced in irq.c (sry)

// Set GPIO drive strengths and modes of operation
void gpio_init(){
  CMU_ClockEnable(cmuClock_GPIO, true);

	GPIO_PinModeSet(LED_PORT, LED_PIN, gpioModePushPull, false);
  GPIO_PinModeSet(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN, gpioModePushPull, false);
  GPIO_PinModeSet(GEN_GPIO_PORT, GEN_GPIO1_PIN, gpioModePushPull, false);
  GPIO_PinModeSet(GEN_GPIO_PORT, GEN_GPIO2_PIN, gpioModePushPull, false);
  GPIO_PinModeSet(GEN_GPIO_PORT, GEN_GPIO3_PIN, gpioModePushPull, false);

  GPIO_PinModeSet(GPIO_ACCEL_PORT, GPIO_ACCEL_INT1_PIN, gpioModeInput, 0);
  GPIO_ExtIntConfig(GPIO_ACCEL_PORT, GPIO_ACCEL_INT1_PIN, GPIO_ACCEL_INT1_PIN, true, false, true);

  GPIO_PinModeSet(GPIO_ACCEL_PORT, GPIO_ACCEL_INT2_PIN, gpioModeInput, 0);
  GPIO_ExtIntConfig(GPIO_ACCEL_PORT, GPIO_ACCEL_INT2_PIN, GPIO_ACCEL_INT2_PIN, true, false, true);

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
} // gpioInit()

void gpio_led_on(){
	GPIO_PinOutSet(LED_PORT, LED_PIN);
}

void gpio_led_off(){
  GPIO_PinOutClear(LED_PORT, LED_PIN);
}

void gpio_sensor_enable_on(){
  GPIO_PinOutSet(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN);
}

void gpio_sensor_enable_off(){
  GPIO_PinOutClear(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN);
}

void gpio_gen1_on(){
  GPIO_PinOutSet(GEN_GPIO_PORT, GEN_GPIO1_PIN);
}

void gpio_gen1_off(){
  GPIO_PinOutClear(GEN_GPIO_PORT, GEN_GPIO1_PIN);
}

void gpio_gen2_on(){
  GPIO_PinOutSet(GEN_GPIO_PORT, GEN_GPIO2_PIN);
}

void gpio_gen2_off(){
  GPIO_PinOutClear(GEN_GPIO_PORT, GEN_GPIO2_PIN);
}

void gpio_gen3_on(){
  GPIO_PinOutSet(GEN_GPIO_PORT, GEN_GPIO3_PIN);
}

void gpio_gen3_off(){
  GPIO_PinOutClear(GEN_GPIO_PORT, GEN_GPIO3_PIN);
}

void gpio_accel_int1_enable(){
  GPIO_ExtIntConfig(GPIO_ACCEL_PORT, GPIO_ACCEL_INT1_PIN, GPIO_ACCEL_INT1_PIN, true, false, true);
}

void gpio_accel_int1_disable(){
  GPIO_ExtIntConfig(GPIO_ACCEL_PORT, GPIO_ACCEL_INT1_PIN, GPIO_ACCEL_INT1_PIN, true, false, false);
}

void gpio_accel_int2_enable(){
  GPIO_ExtIntConfig(GPIO_ACCEL_PORT, GPIO_ACCEL_INT2_PIN, GPIO_ACCEL_INT2_PIN, true, false, true);
}

void gpio_accel_int2_disable(){
  GPIO_ExtIntConfig(GPIO_ACCEL_PORT, GPIO_ACCEL_INT2_PIN, GPIO_ACCEL_INT2_PIN, true, false, false);
}
