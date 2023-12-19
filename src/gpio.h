/*
   gpio.h
  
    Created on: Dec 12, 2018
        Author: Dan Walkes

    Updated by Dave Sluiter Sept 7, 2020. moved #defines from .c to .h file.
    Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

    Editor: Feb 26, 2022, Dave Sluiter
    Change: Added comment about use of .h files.

 *
 * @student    James Bohn, james.bohn@colorado.edu
 *
 
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_

// Function prototypes
void gpio_init();
void gpio_led_on();
void gpio_led_off();
void gpio_sensor_enable_on();
void gpio_sensor_enable_off();
void gpio_gen1_on();
void gpio_gen1_off();
void gpio_gen2_on();
void gpio_gen2_off();
void gpio_gen3_on();
void gpio_gen3_off();

void gpio_accel_int1_enable();
void gpio_accel_int1_disable();
void gpio_accel_int2_enable();
void gpio_accel_int2_disable();

#endif /* SRC_GPIO_H_ */
