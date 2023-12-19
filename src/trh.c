/*
 * si7021.c
 *
 *  Created on: Dec 18, 2023
 *      Author: jbohn
 */

#include "trh.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "i2c.h"
#include "timers.h"
#include "gpio.h"
#include "gatt_db.h"
#include "ble.h"
#include "scheduler.h"
#include "data.h"

#define SI7021_I2C_DEV                  (I2C1)
#define SI7021_ADDR                     (0x40)
#define SI7021_CMD_MEAURE_TEMP_NO_HOLD  (0xF3)
#define SI7021_CMD_MEAURE_RH_NO_HOLD    (0xF5)

void trh_init(){

}

typedef enum {
  TEMP_STATE0_OFF,
  TEMP_STATE1_ON_IDLE,
  TEMP_STATE2_TEMP_MEASURE,
  TEMP_STATE3_TEMP_READ,
  TEMP_STATE4_RH_MEASURE,
  TEMP_STATE5_RH_READ,
  TEMP_NUM_STATES
} tempState_e;

/// @brief Runs a state machine to periodically take temperature measurements
///          from the SI7021 on the dev board
/// @param sl_bt_msg_t *evt - event from the bt stack to be acted upon
void trh_state_machine(sl_bt_msg_t *evt){
  static tempState_e    nextState = TEMP_STATE0_OFF;
         tempState_e    currentState;

  currentState = nextState;

  // Early retun if evt is not an external signal
  if(SL_BT_MSG_ID(evt->header) != sl_bt_evt_system_external_signal_id){
    return;
  }

  // Get actual data from the event and stored BT state
  schedEvt_e sig = evt->data.evt_system_external_signal.extsignals;

  switch (currentState){
    case TEMP_STATE0_OFF:
      nextState = TEMP_STATE0_OFF;

      if(sig == evtGPIO_Accel_Act)
      {
        nextState = TEMP_STATE1_ON_IDLE;
      }

      break;
    case TEMP_STATE1_ON_IDLE:
      nextState = TEMP_STATE1_ON_IDLE;

      if(sig == evtGPIO_Accel_Inact){
          nextState = TEMP_STATE0_OFF;
      }

      if(sig == evtLETIMER0_UF){
        i2c_cmd_write(SI7021_I2C_DEV, SI7021_ADDR, SI7021_CMD_MEAURE_TEMP_NO_HOLD, 0);
        timer_wait_us_irq(100000);
        nextState = TEMP_STATE2_TEMP_MEASURE;
      }

      break;
    case TEMP_STATE2_TEMP_MEASURE:
      nextState = TEMP_STATE2_TEMP_MEASURE;

      if(sig == evtGPIO_Accel_Inact){
          nextState = TEMP_STATE0_OFF;
      }

      if(sig == evtLETIMER0_COMP1){
        i2c_cmd_read(SI7021_I2C_DEV, SI7021_ADDR, 2, 0);
        nextState = TEMP_STATE3_TEMP_READ;
      }

      break;
    case TEMP_STATE3_TEMP_READ:
      nextState = TEMP_STATE3_TEMP_READ;

      if(sig == evtGPIO_Accel_Inact){
          nextState = TEMP_STATE0_OFF;
      }

      if(sig == evtI2C1_TransferComplete){
        // access and save read temp data
        uint16_t temp_raw = 0;
        float temp_c = 0;

        i2c_get_read_data((uint8_t *) &temp_raw, 2);
        temp_raw = (temp_raw >> 8) | ((temp_raw & 0xFF) << 8);

        //     do float math
        temp_c = (((175.72*(float)temp_raw)/65536.0) - 46.85);
        LOG_INFO("Current temp: %f C", temp_c);

        write_temp(temp_c);

        i2c_cmd_write(SI7021_I2C_DEV, SI7021_ADDR, SI7021_CMD_MEAURE_RH_NO_HOLD, 0);
        timer_wait_us_irq(100000);
        nextState = TEMP_STATE4_RH_MEASURE;
      }

      break;
    case TEMP_STATE4_RH_MEASURE:
      nextState = TEMP_STATE4_RH_MEASURE;

      if(sig == evtGPIO_Accel_Inact){
          nextState = TEMP_STATE0_OFF;
      }

      if(sig == evtLETIMER0_COMP1){
        i2c_cmd_read(SI7021_I2C_DEV, SI7021_ADDR, 2, 0);
        nextState = TEMP_STATE5_RH_READ;
      }

      break;
    case TEMP_STATE5_RH_READ:
      nextState = TEMP_STATE5_RH_READ;

      if(sig == evtGPIO_Accel_Inact){
          nextState = TEMP_STATE0_OFF;
      }

      if(evtI2C1_TransferComplete){
        // process and save RH data
        uint16_t rh_raw = 0;
        float rh_percent = 0;

        i2c_get_read_data((uint8_t *) &rh_raw, 2);
        rh_raw = (rh_raw >> 8) | ((rh_raw & 0xFF) << 8);

        //     do float math
        rh_percent = (((125.0*(float)rh_raw)/65536.0) - 6);
        LOG_INFO("Current rh: %f%%", rh_percent);

        if(rh_percent < 0){
            rh_percent = 0;
        }
        if(rh_percent > 100){
            rh_percent = 100;
        }

        write_rh(rh_percent);

        nextState = TEMP_STATE1_ON_IDLE;
      }

      break;
    default:
      LOG_ERROR("Invaid trh state machine state: %d", (int32_t)currentState);
      break;
  }
} // temperature_state_machine()
