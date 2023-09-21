/*
 * i2c.h
 *
 *  Created on: Sep 21, 2023
 *      Author: jbohn
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include <stdint.h>

#define I2C_SI7021_ADDR                     (0x40)
#define I2C_SI7021_CMD_MEAURE_TEMP_NO_HOLD  (0xF3)

void i2cInit();
uint32_t i2cRead(uint16_t addr, uint16_t byte_len);
void i2cWrite(uint16_t addr, uint8_t cmd_data);

#endif /* SRC_I2C_H_ */
