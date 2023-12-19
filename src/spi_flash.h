/*
 * spi_flashdriver.h
 *
 *  Created on: Nov 20, 2023
 *      Author: ayswariya
 */

#ifndef SRC_SPI_FLASHDRIVER_H_
#define SRC_SPI_FLASHDRIVER_H_

#include "stdint.h"

void spi_init(void);

void spi_write_enable(void);
void spi_write_byte(uint32_t address, uint8_t data);
void spi_write_page(uint32_t address, const uint8_t *data, uint32_t size);
uint8_t spi_read_byte(uint32_t address);
uint8_t spi_read_SR(void);
void spi_read_block(uint32_t address, uint8_t *data, uint32_t size);
void erase_sector(uint32_t address);
void erase_32Kblock(uint32_t address);
void erase_64kblock(uint32_t address);
void chip_erase(void);


#endif /* SRC_SPI_FLASHDRIVER_H_ */
