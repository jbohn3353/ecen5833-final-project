/*
 * uart.c
 *
 *  Created on: Dec 18, 2023
 *      Author: jbohn
 *
 *      @resource shared driver with Devang + group
 */

#include "uart.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "stdbool.h"
#include "em_leuart.h"
#include "em_cmu.h"
#include "scheduler.h"
#include "string.h"

#define LEUART_BAUD     (9600)

static void uart_transmit_byte();

static void LEUART0_frame_handler();
static void LEUART0_recv_handler();
static void LEUART0_txbl_handler();
static void LEUART0_txc_handler();
static void LEUART0_delim_handler();

uint8_t rx_buf[FIELD_MAX] = {0};
uint8_t rx_i = 0;

uint8_t tx_buf[64];
uint8_t tx_i = 0;
uint8_t tx_size = 0;

uint8_t field_buf[FIELD_MAX];
uint8_t field_len = 0;

uint8_t get_field(uint8_t *buf, uint8_t buf_size){
  strncpy(buf, field_buf, buf_size);
  return field_len;
}

void uart_init(){
  CMU_ClockEnable(cmuClock_LEUART0, true);

  // leuart 0 initialization struct
  LEUART_Init_TypeDef leuart_init_struct = {
      .baudrate = LEUART_BAUD,
      .enable   = leuartDisable,
//      .enable   = leuartEnable,
      .parity   = leuartNoParity,
      .databits = leuartDatabits8,
      .stopbits = leuartStopbits1,
      .refFreq  = 0
  };

  // init letuart
  LEUART_Init(LEUART0,&leuart_init_struct);
  while(LEUART0->SYNCBUSY);


  // ENABLE AND ROUTE TX RX LOCATIONS
  GPIO_PinModeSet(gpioPortC, 7, gpioModePushPull, 1);
  GPIO_PinModeSet(gpioPortC, 8, gpioModeInput, 0);

  LEUART0->ROUTELOC0 = LEUART_ROUTELOC0_RXLOC_LOC12 | LEUART_ROUTELOC0_TXLOC_LOC12;
  LEUART0->ROUTEPEN =  LEUART_ROUTEPEN_RXPEN | LEUART_ROUTEPEN_TXPEN;

  // Set the fram identifiers
  LEUART0->STARTFRAME ='$';
  LEUART0->SIGFRAME   = '\n';
  //  LEUART0->SIGFRAME   =',';

  // Turn on RX block
  LEUART0->CMD = LEUART_CMD_RXBLOCKEN;
  while(LEUART0->SYNCBUSY);

  // unblock rx on frame start ($)
  LEUART0->CTRL |= LEUART_CTRL_SFUBRX;

  // Clear RX and TX buffers
  LEUART0->CMD = LEUART_CMD_CLEARRX|LEUART_CMD_CLEARTX;
  while(LEUART0->SYNCBUSY);

  // Turn on the uart
  LEUART_Enable(LEUART0,leuartEnable);
  while (LEUART0->SYNCBUSY);

//  LEUART_IntEnable(LEUART0,LEUART_IEN_RXDATAV);
  NVIC_EnableIRQ(LEUART0_IRQn);
}

void uart_send(uint8_t *string, uint8_t string_len){

  for(size_t i = 0; i < string_len; i++){
      tx_buf[i] = string[i];
  }
  tx_i = 0;
  tx_size = string_len - 1;

  uart_transmit_byte();

  LEUART_IntEnable(LEUART0,LEUART_IEN_TXBL);
}

static void uart_transmit_byte(){
  while(LEUART0->SYNCBUSY);
  LEUART0->TXDATA = tx_buf[tx_i++];
}

void LEUART0_IRQHandler(){
  // fetch
  uint32_t flags = LEUART_IntGetEnabled(LEUART0);

  // clear
  LEUART_IntClear(LEUART0, flags);

  // handle
  if(flags & LEUART_IF_TXBL){
    LEUART0_txbl_handler();
  }

  if(flags & LEUART_IF_TXC){
    LEUART0_txc_handler();
  }

  if(flags & LEUART_IF_STARTF){
    LEUART0_frame_handler();
  }

  if(flags & LEUART_IF_SIGF){
    LEUART0_delim_handler();
  }
  // Only run rxdata if no sigf
  else if(flags & LEUART_IF_RXDATAV){
    LEUART0_recv_handler();
  }
}


static void LEUART0_txbl_handler(){
  if(tx_i < tx_size){
      uart_transmit_byte();
  }
  else{
    LEUART_IntDisable(LEUART0,LEUART_IEN_TXBL);
    LEUART_IntEnable(LEUART0,LEUART_IEN_TXC);
    uart_transmit_byte();
  }
}

static void LEUART0_txc_handler(){
  LEUART_IntDisable(LEUART0,LEUART_IEN_TXC);
  sched_set_LEUART0_txc();
}


static void LEUART0_frame_handler(){
  LEUART_IntDisable(LEUART0, LEUART_IEN_STARTF);
  LEUART_IntEnable(LEUART0,LEUART_IEN_RXDATAV | LEUART_IEN_SIGF);
}

static void LEUART0_recv_handler(){
  uint8_t data = LEUART0->RXDATA;

  rx_buf[rx_i++] = data;
  rx_buf[rx_i] = '\0';

//  app_log("%c", data);

  if(rx_i == 255){
    LOG_ERROR("Rx buf overflow");
  }
}

static void LEUART0_delim_handler(){
  LEUART0->CMD = LEUART_CMD_RXBLOCKEN;
  while(LEUART0->SYNCBUSY);

//  LEUART_IntDisable(LEUART0,LEUART_IEN_RXDATAV | LEUART_IEN_SIGF);

  strncpy(field_buf, rx_buf, FIELD_MAX);
  field_buf[rx_i] = '\0';
  field_len = rx_i + 1;

  rx_buf[0] = '\0';
  rx_i = 0;

  sched_set_LEUART0_delim();
}
