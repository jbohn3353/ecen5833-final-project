/**
 * @file      scheduler.h
 * @brief     Defines functions to our scheduler for the EFM32
 * @author    James Bohn
 * @date      Sep 20, 2023
 */

#include "scheduler.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "em_core.h"
#include "i2c.h"
#include "timers.h"
#include "gpio.h"
#include "gatt_db.h"
#include "ble.h"

static void temperature_state_machine(sl_bt_msg_t *evt);

/// @brief set sched flag for LETIMER0_UF event
void schedSetEventLETIMER0_UF()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtLETIMER0_UF);
  CORE_EXIT_CRITICAL();
} // schedSetEventLETIMER0_UF()

/// @brief set sched flag for LETIMER0_COMP1 event
void schedSetEventLETIMER0_COMP1()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtLETIMER0_COMP1);
  CORE_EXIT_CRITICAL();
} // schedSetEventLETIMER0_COMP1()

/// @brief set sched flag for I2C0_TransferComplete event
void schedSetEventI2C0_TransferComplete()
{
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_CRITICAL();
  sl_bt_external_signal(evtI2C0_TransferComplete);
  CORE_EXIT_CRITICAL();
} // schedSetEventI2C0_TransferComplete()

void run_state_machines(sl_bt_msg_t *evt){
  temperature_state_machine(evt);
}

typedef enum {
  TEMP_STATE0_IDLE,
  TEMP_STATE1_WARMUP,
  TEMP_STATE2_MEASUREMENT_WRITE,
  TEMP_STATE3_MEASUREMENT_WAIT,
  TEMP_STATE4_RETRIEVE,
  TEMP_NUM_STATES
} tempState_e;

/// @brief Runs a state machine to periodically take temperature measurements
///          from the SI7021 on the dev board
/// @param schedEvt_e evt - an enum from the scheduler representing the event that
///          just occurred
static void temperature_state_machine(sl_bt_msg_t *evt)
{



  static tempState_e    nextState = TEMP_STATE0_IDLE;
         tempState_e    currentState;

   uint32_t temp_raw = 0;
   int32_t temp_c = 0;
   sl_status_t sc = 0;

  currentState = nextState;

  if(SL_BT_MSG_ID(evt->header) != sl_bt_evt_system_external_signal_id)
  {
    return;
  }

  schedEvt_e sig = evt->data.evt_system_external_signal.extsignals;
  ble_data_struct_t *bleDataPtr = bleGetStruct();

  switch (currentState)
  {
    case TEMP_STATE0_IDLE:
      nextState = TEMP_STATE0_IDLE;

      // Only start temp measurement if connection is currently open
      // This change is very simple and results in temperature measurements
      // that are in process finishing if the connection closes in the middle
      // of the measurement but thats fine
      if(sig == evtLETIMER0_UF        &&
         bleDataPtr->conn_open        &&
         bleDataPtr->temp_indication_en)
      {
        gpioSensorEnableSetOn();
        timerWaitUs_irq(80000);
        nextState = TEMP_STATE1_WARMUP;
      }

      break;
    case TEMP_STATE1_WARMUP:
      nextState = TEMP_STATE1_WARMUP;

      if(sig == evtLETIMER0_COMP1)
      {
        sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
        i2cWrite(I2C_SI7021_ADDR, I2C_SI7021_CMD_MEAURE_TEMP_NO_HOLD);
        nextState = TEMP_STATE2_MEASUREMENT_WRITE;
      }

      break;
    case TEMP_STATE2_MEASUREMENT_WRITE:
      nextState = TEMP_STATE2_MEASUREMENT_WRITE;

      if(sig == evtI2C0_TransferComplete)
      {
        sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
        timerWaitUs_irq(20000);
        nextState = TEMP_STATE3_MEASUREMENT_WAIT;
      }

      break;
    case TEMP_STATE3_MEASUREMENT_WAIT:
      nextState = TEMP_STATE3_MEASUREMENT_WAIT;

      if(sig == evtLETIMER0_COMP1)
      {
        sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
        i2cRead(I2C_SI7021_ADDR, 2);
        nextState = TEMP_STATE4_RETRIEVE;
      }

      break;
    case TEMP_STATE4_RETRIEVE:
      nextState = TEMP_STATE4_RETRIEVE;

      if(sig == evtI2C0_TransferComplete)
      {
        sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
        temp_raw = (i2cGetReadData() >> 8) | ((i2cGetReadData() & 0xFF) << 8);

        gpioSensorEnableSetOff();

        //     round to int   |     do float math
        temp_c = (int32_t) (((175.72*(float)temp_raw)/65536) - 46.85);
        LOG_INFO("Current temp: %d C", temp_c);

        sc = sl_bt_gatt_server_write_attribute_value(
                           gattdb_temperature_measurement,
                           0,
                           4,
                           (uint8_t *)&temp_c);
        if (sc != SL_STATUS_OK)
        {
//          LOG_ERROR("Error with sl_bt_gatt_server_write_attribute_value: %x", sc);
        }

        // We recheck the BT connection and indication statuses again to make
        // sure we dont run into any errors with the si labs functions
        if( bleDataPtr->conn_open          &&
            bleDataPtr->temp_indication_en &&
           !bleDataPtr->indication_inflight)
        {
          uint8_t   htm_temperature_buffer[5] = {0};
          uint8_t  *p = &htm_temperature_buffer[1];
          uint32_t  htm_temperature_flt;

          htm_temperature_flt = UINT32_TO_FLOAT(temp_c * 1000,  -3);
          UINT32_TO_BITSTREAM(p, htm_temperature_flt);

          sc = sl_bt_gatt_server_send_indication(bleDataPtr->conn_handle,
                                                 gattdb_temperature_measurement,
                                                 sizeof(htm_temperature_buffer),
                                                 &htm_temperature_buffer[0]);
          if (sc != SL_STATUS_OK)
          {
            LOG_ERROR("Error with sl_bt_gatt_server_send_indication: %x", sc);
          }
          else
          {
              bleDataPtr->indication_inflight = 1;
          }

        }

        nextState = TEMP_STATE0_IDLE;
      }

      break;
    default:
      LOG_ERROR("Invaid temperature state machine state: %d", (int32_t)currentState);
      break;
  }
}
