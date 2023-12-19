/*
 * adxl343.c
 *
 *  Created on: Dec 17, 2023
 *      Author: jbohn
 */

#include "accel.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "em_assert.h"

#include "i2c.h"
#include "scheduler.h"
#include "gpio.h"
#include "timers.h"

#define ADXL_I2C_DEV                    (I2C0)
#define ADXL343_ADDR                    (0x53)

#define ADXL343_REG_DEVID               (0x00)
#define ADXL343_REG_THRESH_ACT          (0x24)
#define ADXL343_REG_THRESH_INACT        (0x25)
#define ADXL343_REG_TIME_INACT          (0x26)
#define ADXL343_REG_ACT_INACT_CTL       (0x27)
#define ADXL343_REG_THRESH_FF           (0x28)
#define ADXL343_REG_TIME_FF             (0x29)
#define ADXL343_REG_BW_RATE             (0x2C)
#define ADXL343_REG_POWER_CTL           (0x2D)
#define ADXL343_REG_INT_ENABLE          (0x2E)
#define ADXL343_REG_INT_MAP             (0x2F)
#define ADXL343_REG_INT_SOURCE          (0x30)
#define ADXL343_REG_DATA_FORMAT         (0x31)
#define ADXL343_REG_DATA                (0x32) // 6 bytes, x0x1y0y1z0z1 LE

#define ACT_THRESH                      (60)   // 62.5 mg/LSB but I don't belive this

void accel_init(){
  uint8_t *data;

  data = i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_DEVID, 1, 1);
  EFM_ASSERT(*data == 0b11100101);

  // Put device in measurement mode, link to alternate act and inact
  i2c_reg_write(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_POWER_CTL, 0b00001011, 1);
  data = i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_POWER_CTL, 1, 1);
  EFM_ASSERT(*data == 0b00001011);

  // Put device in low power mode, set standard sample rate to 25 Hz
  i2c_reg_write(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_BW_RATE, 0b00011000, 1);
  data = i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_BW_RATE, 1, 1);
  EFM_ASSERT(*data == 0b00011000);

  // Set inactivity threshold to 1.5 g's? (24 * 62.5 mg/LSB)
  i2c_reg_write(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_THRESH_INACT, 60, 1);
  data = i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_THRESH_INACT, 1, 1);
  EFM_ASSERT(*data == 60);

  // Set inactivity time to 10 sec (temp)
  i2c_reg_write(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_TIME_INACT, 240, 1);
  data = i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_TIME_INACT, 1, 1);
  EFM_ASSERT(*data == 240);

  // Set activity threshold to 1.125 g's (18 * 62.5 mg/LSB)??
  i2c_reg_write(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_THRESH_ACT, 60, 1);
  data = i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_THRESH_ACT, 1, 1);
  EFM_ASSERT(*data == 60);

  // Add all axes to activity and inact interrupts
  i2c_reg_write(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_ACT_INACT_CTL, 0b01110111, 1);
  data = i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_ACT_INACT_CTL, 1, 1);
  EFM_ASSERT(*data == 0b01110111);

  // Put inactivity int on int2
  i2c_reg_write(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_INT_MAP, 0b00001000, 1);
  data = i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_INT_MAP, 1, 1);
  EFM_ASSERT(*data == 0b00001000);

  // Enable activity and inact ints
  i2c_reg_write(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_INT_ENABLE, 0b00011000, 1);
  data = i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_INT_ENABLE, 1, 1);
  EFM_ASSERT(*data == 0b00011000);

  gpio_accel_int1_enable();
  gpio_accel_int2_enable();

  // Reset int reg by reading
  data = i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_INT_SOURCE, 1, 1);
}

typedef enum {
  ACCEL_STATE0_INACTIVE,
  ACCEL_STATE1_ACTIVE,
  ACCEL_NUM_STATES
} accel_state_e;

void accel_state_machine(sl_bt_msg_t *evt){
  static accel_state_e    nextState = ACCEL_STATE0_INACTIVE;
         accel_state_e    currentState;

  currentState = nextState;

  // Early retun if evt is not an external signal
  if(SL_BT_MSG_ID(evt->header) != sl_bt_evt_system_external_signal_id){
    return;
  }

  // Get actual data from the event and stored BT state
  schedEvt_e sig = evt->data.evt_system_external_signal.extsignals;

  // Reset int reg by reading
  if(sig == evtGPIO_Accel_Inact || sig == evtGPIO_Accel_Act){
    i2c_reg_read(ADXL_I2C_DEV, ADXL343_ADDR, ADXL343_REG_INT_SOURCE, 1, 1);
  }

  switch (currentState){
    case ACCEL_STATE0_INACTIVE:
      nextState = ACCEL_STATE0_INACTIVE;

      if(sig == evtGPIO_Accel_Act){
        gpio_sensor_enable_on();
        timer_enable();
        LOG_INFO("Active...");
        nextState = ACCEL_STATE1_ACTIVE;
      }

      break;
    case ACCEL_STATE1_ACTIVE:
      nextState = ACCEL_STATE1_ACTIVE;

      if(sig == evtGPIO_Accel_Inact){
        gpio_sensor_enable_off();
        timer_disable();
        LOG_INFO("Inactive...");
        nextState = ACCEL_STATE0_INACTIVE;
      }

      break;
    default:
      LOG_ERROR("Invaid accel state machine state: %d", (int32_t)currentState);
      break;
  }
} // temperature_state_machine()

