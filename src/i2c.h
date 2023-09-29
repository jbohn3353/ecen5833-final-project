/**
 * @file      i2c.h
 * @brief     Declares functions to manage i2c for the EFM32
 * @author    James Bohn
 * @date      Sep 20, 2023
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include <stdint.h>
#include "scheduler.h"

#define I2C_SI7021_ADDR                     (0x40)
#define I2C_SI7021_CMD_MEAURE_TEMP_NO_HOLD  (0xF3)

void i2cInit();
void i2cRead(uint16_t addr, uint16_t byte_len);
void i2cWrite(uint16_t addr, uint8_t cmd_data);

uint32_t i2cGetReadData();

#endif /* SRC_I2C_H_ */
