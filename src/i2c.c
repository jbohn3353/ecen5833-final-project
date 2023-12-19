/**
 * @file      i2c.c
 * @brief     Defines functions to manage i2c for the EFM32
 * @author    James Bohn
 * @date      Sep 20, 2023
 */

#include "i2c.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "scheduler.h"
#include "timers.h"
#include "gpio.h"
#include "stdbool.h"
#include "string.h"

static I2C_TransferSeq_TypeDef  transferSequence;
static uint8_t                  reg_buf;
static uint8_t                  write_buf;
static uint8_t                  read_buf[I2C_MAX_READ_SIZE];

static void i2c_init(I2C_TypeDef *i2c_device);
static void i2c_transfer(I2C_TypeDef *i2c_device, bool blocking);

/// @brief reads static variable holding most recent data from i2c read
void i2c_get_read_data(uint8_t *buf, size_t len)
{
  memcpy(buf, read_buf, len);
  memset(read_buf, 0, len);
}

static void i2c_init(I2C_TypeDef *i2c_device){
  I2CSPM_Init_TypeDef I2C_Config = {
      .port             = i2c_device,
      .sclPort          = (i2c_device == I2C0) ? gpioPortA : gpioPortC,
      .sclPin           = (i2c_device == I2C0) ? 3         : 10,
      .sdaPort          = (i2c_device == I2C0) ? gpioPortA : gpioPortC,
      .sdaPin           = (i2c_device == I2C0) ? 2         : 11,
      .portLocationScl  = (i2c_device == I2C0) ? 2         : 18,
      .portLocationSda  = (i2c_device == I2C0) ? 2         : 20,
      .i2cRefFreq       = 0,
      .i2cMaxFreq       = I2C_FREQ_STANDARD_MAX,
      .i2cClhr          = i2cClockHLRStandard,
  };

  I2CSPM_Init(&I2C_Config);

  uint32_t freq = I2C_BusFreqGet(I2C0);
  freq++;
}

static void i2c_transfer(I2C_TypeDef *i2c_device, bool blocking){
  I2C_TransferReturn_TypeDef transferStatus;

  if(blocking){
    transferStatus = I2CSPM_Transfer(i2c_device, &transferSequence);
    if (transferStatus != i2cTransferDone) {
      LOG_ERROR("I2CSPM Transfer: I2C bus read");
    }
  }
  else{
    // disabled from irq when transfer complete
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

    if(i2c_device == I2C0){
      NVIC_EnableIRQ(I2C0_IRQn);
    }
    if(i2c_device == I2C1){
      NVIC_EnableIRQ(I2C1_IRQn);
    }

    transferStatus = I2C_TransferInit(i2c_device, &transferSequence);
    if (transferStatus != i2cTransferInProgress) {
      LOG_ERROR("I2C Transfer: I2C bus read");
    }
  }
}

/// @brief perform an i2c read of read_len bytes from device address dev_addr and register address reg_addr
/// @param uint8_t dev_addr - device addr to read from
/// @param uint8_t reg_addr - register addr to read from
/// @param uint16size_tt read_len - number of bytes to read, <= I2C_MAX_READ_SIZE
uint8_t * i2c_reg_read(I2C_TypeDef *i2c_device, uint8_t dev_addr, uint8_t reg_addr, size_t read_len, bool blocking){

  if(read_len > I2C_MAX_READ_SIZE){
    LOG_ERROR("I2C read too large");
    return NULL;
  }

  i2c_init(i2c_device);

  reg_buf = reg_addr;

  transferSequence.addr = dev_addr << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_WRITE_READ;
  transferSequence.buf[0].data = &reg_buf; // pointer to data to write
  transferSequence.buf[0].len = 1;
  transferSequence.buf[1].data = (uint8_t *)read_buf; // pointer to data to write
  transferSequence.buf[1].len = read_len;

  i2c_transfer(i2c_device, blocking);

  if(blocking){
    return read_buf;
  }
  else{
    return NULL;
  }
}

uint8_t * i2c_cmd_read(I2C_TypeDef *i2c_device, uint8_t dev_addr, size_t read_len, bool blocking){

  if(read_len > I2C_MAX_READ_SIZE){
    LOG_ERROR("I2C read too large");
    return NULL;
  }

  i2c_init(i2c_device);

  transferSequence.addr = dev_addr << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_READ;
  transferSequence.buf[0].data = (uint8_t *)read_buf; // pointer to data to write
  transferSequence.buf[0].len = read_len;

  i2c_transfer(i2c_device, blocking);

  if(blocking){
    return read_buf;
  }
  else{
    return NULL;
  }
}

/// @brief perform an i2c write of write_data to device address dev_addr and register address reg_addr
/// @param uint8_t dev_addr - device addr to read from
/// @param uint8_t reg_addr - register addr to read from
/// @param uint8_t write_data - the byte to be written
void i2c_reg_write(I2C_TypeDef *i2c_device, uint8_t dev_addr, uint8_t reg_addr, uint8_t write_data, bool blocking){

  i2c_init(i2c_device);

  reg_buf = reg_addr;
  write_buf = write_data;

  transferSequence.addr = dev_addr << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_WRITE_WRITE;
  transferSequence.buf[0].data = &reg_buf; // pointer to data to write
  transferSequence.buf[0].len = 1;
  transferSequence.buf[1].data = &write_buf; // pointer to data to write
  transferSequence.buf[1].len = 1;

  i2c_transfer(i2c_device, blocking);
}

void i2c_cmd_write(I2C_TypeDef *i2c_device, uint8_t dev_addr, uint8_t write_data, bool blocking){

  i2c_init(i2c_device);

  write_buf = write_data;

  transferSequence.addr = dev_addr << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_WRITE;
  transferSequence.buf[0].data = &write_buf; // pointer to data to write
  transferSequence.buf[0].len = 1;

  i2c_transfer(i2c_device, blocking);
}
