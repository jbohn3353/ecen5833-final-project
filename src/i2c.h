/**
 * @file      i2c.h
 * @brief     Declares functions to manage i2c for the EFM32
 * @author    James Bohn
 * @date      Sep 20, 2023
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include "stddef.h"
#include "sl_i2cspm.h"

#define I2C_MAX_READ_SIZE                   (16)

uint8_t * i2c_reg_read(I2C_TypeDef *i2c_device, uint8_t dev_addr, uint8_t reg_addr, size_t read_len, bool blocking);
uint8_t * i2c_cmd_read(I2C_TypeDef *i2c_device, uint8_t dev_addr, size_t read_len, bool blocking);
void i2c_reg_write(I2C_TypeDef *i2c_device, uint8_t dev_addr, uint8_t reg_addr, uint8_t write_data, bool blocking);
void i2c_cmd_write(I2C_TypeDef *i2c_device, uint8_t dev_addr, uint8_t write_data, bool blocking);

void i2c_get_read_data(uint8_t *buf, size_t len);

#endif /* SRC_I2C_H_ */
