/*
 * data.c
 *
 *  Created on: Dec 18, 2023
 *      Author: jbohn
 */

#include "data.h"

#include "string.h"
#include "stdlib.h"

typedef struct{
  // data for ble
  uint8_t timestamp[18]; // "mm/dd/yy hh:mm:ss\0"
  uint8_t gps[22]; // "xxx.xxxxxN xxx.xxxxxW\0"
  float temp;
  float rh;

  bool gps_valid;
  bool temp_valid;
  bool rh_valid;
} stored_data_t;

static stored_data_t stored = {0};

bool fetch_data(uint8_t *packet_buf, uint8_t max_packet_size){
  if(stored.gps_valid && stored.temp_valid && stored.rh_valid){
      stored.gps_valid = false;
      stored.temp_valid = false;
      stored.rh_valid = false;

      snprintf(packet_buf, max_packet_size, "%s %s %.2f %.1f", stored.timestamp, stored.gps, stored.temp, stored.rh);
      return true;
  }

  return false;
}

void write_gps(uint8_t *data, uint8_t len){
  // Do some processing
  uint8_t *token = data;
  uint8_t token_cnt = 0;

  uint8_t days;
  uint8_t months;
  uint8_t years;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;

  // No valid data
  if(!strstr(data, "GPRMC") /*|| strchr(data, 'V')*/){
      return;
  }

  token = strtok(data, ",");

  // This won't hit the last token but we dont care
  while(token != NULL){

      // hhmmss.00
      if(token_cnt == 1){
        uint8_t buf[3];
        buf[2] = '\0';

        memcpy(buf, token, 2);
        sscanf(token, "%d", &hours);

        memcpy(buf, token+2, 2);
        sscanf(token, "%d", &minutes);

        memcpy(buf, token+4, 2);
        sscanf(token, "%d", &seconds);
      }
      // ddmmyy
      else if(token_cnt == 9){
        uint8_t buf[3];
        buf[2] = '\0';

        memcpy(buf, token, 2);
        sscanf(token, "%d", &days);

        memcpy(buf, token+2, 2);
        sscanf(token, "%d", &months);

        memcpy(buf, token+4, 2);
        sscanf(token, "%d", &years);
      }

      token = strtok(NULL, ",");
      token_cnt++;
  }

  // mm/dd/yy hh:mm:ss
  sprintf(stored.timestamp, "%.02d/%.02d/%.02d %.02d:%.02d:%.02d", months, days, years, hours, minutes, seconds);

  memcpy(stored.gps, data, 20);
  stored.gps[20] = '\0';
  stored.gps_valid = true;
}

void write_temp(float temp){
  stored.temp = temp;
  stored.temp_valid = true;
}

void write_rh(float rh){
  stored.rh = rh;
  stored.rh_valid = true;
}



