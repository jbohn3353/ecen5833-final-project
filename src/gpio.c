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


// *****************************************************************************
// Students:
// We will be creating additional functions that configure and manipulate GPIOs.
// For any new GPIO function you create, place that function in this file.
// *****************************************************************************

#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>

#include "gpio.h"


// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.
// If these links have gone bad, consult the reference manual and/or the datasheet for the MCU.
// Change to correct port and pins:
#define LED_port   (gpioPortF)
#define LED0_pin   (0x4)
#define LED1_pin   (0x5)

#define SENSOR_ENABLE_PORT  (gpioPortD)
#define SENSOR_ENABLE_PIN   (15)

static int led0State = 0;
static int led1State = 0;

// Set GPIO drive strengths and modes of operation
void gpioInit()
{

    // Set the port's drive strength. In this MCU implementation, all GPIO cells
    // in a "Port" share the same drive strength setting. 
//	GPIO_DriveStrengthSet(LED_port, gpioDriveStrengthStrongAlternateStrong); // Strong, 10mA
	GPIO_DriveStrengthSet(LED_port, gpioDriveStrengthWeakAlternateWeak); // Weak, 1mA
	
	// Set the 2 GPIOs mode of operation
	GPIO_PinModeSet(LED_port, LED0_pin, gpioModePushPull, false);
	GPIO_PinModeSet(LED_port, LED1_pin, gpioModePushPull, false);


} // gpioInit()


void gpioLed0SetOn()
{
  led0State = 1;
	GPIO_PinOutSet(LED_port, LED0_pin);
}


void gpioLed0SetOff()
{
  led0State = 0;
  GPIO_PinOutClear(LED_port, LED0_pin);
}

void gpioLed0Toggle()
{
  led0State = !led0State;
  if(led0State)
  {
    GPIO_PinOutSet(LED_port, LED0_pin);
  }
  else
  {
    GPIO_PinOutClear(LED_port, LED0_pin);
  }
}

void gpioLed1SetOn()
{
  led1State = 1;
	GPIO_PinOutSet(LED_port, LED1_pin);
}


void gpioLed1SetOff()
{
  led1State = 0;
	GPIO_PinOutClear(LED_port, LED1_pin);
}

void gpioLed1Toggle()
{
  led1State = !led1State;
  if(led1State)
  {
    GPIO_PinOutSet(LED_port, LED1_pin);
  }
  else
  {
    GPIO_PinOutClear(LED_port, LED1_pin);
  }
}

void gpioSensorEnableSetOn()
{
  GPIO_PinOutSet(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN);
}

void gpioSensorEnableSetOff()
{
  GPIO_PinOutClear(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN);
}


