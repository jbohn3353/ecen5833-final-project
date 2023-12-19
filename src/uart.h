/*
 * uart.h
 *
 *  Created on: Dec 18, 2023
 *      Author: jbohn
 */

#ifndef SRC_UART_H_
#define SRC_UART_H_

#include "stdint.h"

#define FIELD_MAX       (255)

uint8_t get_field(uint8_t *buf, uint8_t buf_size);

void uart_init();
void uart_send(uint8_t *string, uint8_t string_len);

#endif /* SRC_UART_H_ */
