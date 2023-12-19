/*
 * data.h
 *
 *  Created on: Dec 18, 2023
 *      Author: jbohn
 */

#ifndef SRC_DATA_H_
#define SRC_DATA_H_

#include "stdbool.h"
#include "stdint.h"

bool fetch_data(uint8_t *packet_buf, uint8_t max_packet_size);
void write_gps(uint8_t *data, uint8_t len);
void write_temp(float temp);
void write_rh(float rh);


#endif /* SRC_DATA_H_ */
