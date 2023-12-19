/*
 * gps.c
 *
 *  Created on: Dec 18, 2023
 *      Author: jbohn
 */

#include "gps.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "timers.h"
#include "uart.h"
#include "scheduler.h"
#include "em_leuart.h"
#include "data.h"

#define BOOT_TIME_MS          (30000)
#define CONFIG_CMD_MAX_SIZE   (32)

static void calc_chk_sum(uint8_t *packet, uint8_t size);

static uint64_t start_time_ms = 0;

static uint8_t config_cmds[][CONFIG_CMD_MAX_SIZE] = {
    { // UBX-CFG-MSG
      0xB5,0x62,0x06,0x17,0x14,0x00,0x00,0x23,
      0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x01,
      0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x59,0x32
    },
    { // UBX-DIS-GSA
      0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x02,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },
    { // UBX-DIS-GGA
      0xB5,0x62,0x06,0x01,0x08,0X00,0xF0,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },
    { // UBX-DIS-GSV
      0xB5,0x62,0x06,0x01,0x08,0X00,0xF0,0x03,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },
    { // UBX-DIS-GLL
      0xB5,0x62,0x06,0x01,0x08,0X00,0xF0,0x01,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },
    { // UBX-DIS-VTG
      0xB5,0x62,0x06,0x01,0x08,0X00,0xF0,0x05,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },
    { // UBX-ENA-RMC
      0xB5,0x62,0x06,0x01,0x08,0X00,0xF0,0x04,
      0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00
    }
};
static uint8_t config_cmd_sizes[] = {28, 16, 16, 16, 16, 16, 16};
static uint8_t config_count = 7;
static uint8_t config_i = 0;

void gps_init(){

}

typedef enum {
  GPS_STATE0_OFF,
  GPS_STATE1_BOOT,
  GPS_STATE2_CONFIG,
  GPS_STATE3_IDLE,
  GPS_STATE4_READ_FIELDS,
  GPS_NUM_STATES
} gps_state_e;

void gps_state_machine(sl_bt_msg_t *evt){
  static gps_state_e    nextState = GPS_STATE0_OFF;
         gps_state_e    currentState;

  currentState = nextState;

  // Early retun if evt is not an external signal
  if(SL_BT_MSG_ID(evt->header) != sl_bt_evt_system_external_signal_id){
    return;
  }

  // Get actual data from the event and stored BT state
  schedEvt_e sig = evt->data.evt_system_external_signal.extsignals;

  switch (currentState){
    case GPS_STATE0_OFF:
      nextState = GPS_STATE0_OFF;

      if(sig == evtGPIO_Accel_Act){
        start_time_ms = timer_milliseconds();

        nextState = GPS_STATE1_BOOT;
      }

      break;
    case GPS_STATE1_BOOT:
      nextState = GPS_STATE1_BOOT;

      if(sig == evtGPIO_Accel_Inact){
        nextState = GPS_STATE0_OFF;
      }

      if(sig == evtLETIMER0_UF){
        uint64_t current_time_ms = timer_milliseconds();

        if(current_time_ms - start_time_ms >= BOOT_TIME_MS){
          uart_init();

          config_i = 0;
          calc_chk_sum(config_cmds[config_i], config_cmd_sizes[config_i]);
          uart_send(config_cmds[config_i], config_cmd_sizes[config_i]);
          config_i++;

          LOG_INFO("Configuring GPS...");

          nextState = GPS_STATE2_CONFIG;
        }
        else{
          LOG_INFO("Waiting for GPS boot...");
        }
      }

      break;
    case GPS_STATE2_CONFIG:
      nextState = GPS_STATE2_CONFIG;

      if(sig == evtGPIO_Accel_Inact){
        nextState = GPS_STATE0_OFF;
      }

      if(sig == evtLEUART0_txc){
        if(config_i == config_count){
            nextState = GPS_STATE3_IDLE;
        }
        else{
          calc_chk_sum(config_cmds[config_i], config_cmd_sizes[config_i]);
          uart_send(config_cmds[config_i], config_cmd_sizes[config_i]);
          config_i++;
        }
      }

      break;
    case GPS_STATE3_IDLE:
      nextState = GPS_STATE3_IDLE;

      if(sig == evtGPIO_Accel_Inact){
        nextState = GPS_STATE0_OFF;
      }

      if(sig == evtLETIMER0_UF){
        LOG_INFO("Parsing data from GPS...");
        LEUART_IntEnable(LEUART0,LEUART_IEN_STARTF);

        nextState = GPS_STATE4_READ_FIELDS;
      }

      break;
    case GPS_STATE4_READ_FIELDS:
      nextState = GPS_STATE4_READ_FIELDS;

      if(sig == evtGPIO_Accel_Inact){
        nextState = GPS_STATE0_OFF;
      }

      if(sig == evtLEUART0_delim){
        uint8_t gps_data[FIELD_MAX];
        uint8_t gps_len = 0;

        gps_len = get_field(gps_data, FIELD_MAX);

        // Skip the \n thats at the start of the string for some reason
        LOG_INFO("%s", gps_data + 1);
        write_gps(gps_data + 1, gps_len);

        nextState = GPS_STATE3_IDLE;
      }

      break;
    default:
      LOG_ERROR("Invaid gps state machine state: %d", (int32_t)currentState);
      break;
  }
}

static void calc_chk_sum(uint8_t *packet, uint8_t size){

  uint8_t CK_A = 0;
  uint8_t CK_B = 0;
  uint8_t tmp  = 0;

  for(int i=2; i<(size-2);i++){
    tmp = packet[i];
    CK_A = CK_A + tmp;
    CK_B = CK_B + CK_A;
  }
  CK_A = CK_A & 0xFF;
  CK_B = CK_B & 0xFF;
  packet[size-1] = CK_B;
  packet[size-2] = CK_A;

//  return packet;
}
