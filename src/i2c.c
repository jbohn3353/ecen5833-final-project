/**
 * @file      i2c.c
 * @brief     Declares functions to manage i2c for the EFM32
 * @author    James Bohn
 * @date      Sep 20, 2023
 */

#include "i2c.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 0
#include "log.h"

#include "sl_i2cspm.h"

/// @brief intializes i2c0 using the I2CSPM API
void i2cInit()
{
  I2CSPM_Init_TypeDef I2C_Config = {
      .port             = I2C0,
      .sclPort          = gpioPortC,
      .sclPin           = 10,
      .sdaPort          = gpioPortC,
      .sdaPin           = 11,
      .portLocationScl  = 14,
      .portLocationSda  = 16,
      .i2cRefFreq       = 0,
      .i2cMaxFreq       = I2C_FREQ_STANDARD_MAX,
      .i2cClhr          = i2cClockHLRStandard,
  };

  I2CSPM_Init(&I2C_Config);

  uint32_t freq = I2C_BusFreqGet(I2C0);
  freq++;
}

/// @brief perform an i2c read of byte_len bytes from address addr
/// @param uint16_t addr - device addr to read from
/// @param uint16_t byte_len - number of bytes to read, <= 4
/// @return bytes read in a single var
uint32_t i2cRead(uint16_t addr, uint16_t byte_len)
{
  I2C_TransferReturn_TypeDef transferStatus; // make this global for IRQs in A4
  I2C_TransferSeq_TypeDef transferSequence; // this one can be local
  uint32_t read_data = 0; // make this global for IRQs in A4

  transferSequence.addr = addr << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_READ;
  transferSequence.buf[0].data = (uint8_t *)&read_data; // pointer to data to write
  transferSequence.buf[0].len = byte_len;

  transferStatus = I2CSPM_Transfer(I2C0, &transferSequence);
  if (transferStatus != i2cTransferDone) {
    LOG_ERROR("I2CSPM Transfer: I2C bus read of addr=0x%x failed, state: %d", (uint32_t)addr, (int32_t) transferStatus);
  }

  return read_data;
}

/// @brief i2c cmd_data to address addr
/// @param uint16_t addr - device addr to write to
/// @param uint8_t cmd_data - data to write to addr
void i2cWrite(uint16_t addr, uint8_t cmd_data)
{
  I2C_TransferReturn_TypeDef transferStatus; // make this global for IRQs in A4
  I2C_TransferSeq_TypeDef transferSequence; // this one can be local

  transferSequence.addr = addr << 1; // shift device address left
  transferSequence.flags = I2C_FLAG_WRITE;
  transferSequence.buf[0].data = &cmd_data; // pointer to data to write
  transferSequence.buf[0].len = sizeof(cmd_data);
  transferStatus = I2CSPM_Transfer(I2C0, &transferSequence);
  if (transferStatus != i2cTransferDone) {
    LOG_ERROR("I2CSPM Transfer: I2C bus write of cmd=0x%x to addr=0x%x failed", (uint32_t)cmd_data, (uint32_t)addr);
  }
}
