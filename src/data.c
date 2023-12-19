/*
 * data.c
 *
 *  Created on: Dec 18, 2023
 *      Author: jbohn
 */

#include "data.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct{
  // data for ble
  uint8_t timestamp[18]; // "mm/dd/yy hh:mm:ss\0"
  uint8_t gps[25]; // "xxx.xxxxxN xxx.xxxxxW\0"
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

void nema_2_goog(char* nema_lat, char nema_lat_d, char*  nema_lng, char nema_lng_d, uint8_t *store){

    float fract, inti_lat,inti_lng;
//    coord_t *nema_coord = get_coordinates();

    uint16_t  coord_lat;
    uint16_t  coord_lng;
    // LATITUDE
    float tmp = atof(nema_lat)/100;
    fract = modff(tmp, &inti_lat);

    float google_coord_lat = inti_lat + (fract*100)/60;
    fract = modff(google_coord_lat, &inti_lat);
//    decrypted_coord.coord_lat  = fract * 10000;
//    decrypted_coord.coord_lat *= 10;
    coord_lat  = fract * 10000;
    coord_lat *= 10;

//    sprintf(decrypted_coord.lat,"lat,%03u.%05u,%c",(uint16_t)inti,decrypted_coord.coord_lat,(nema_coord.lat_d));
//    decrypted_coord.lat = nema_coord->lat_d;
//    my_puts(decrypted_coord.lat);
//    my_puts("\n\r");

//    LOG_INFO("%s\r\n\n",decrypted_coord.lat);


    // LONGITUDE
    tmp = atof(nema_lng)/100;
    fract = modff(tmp, &inti_lng);

    float google_coord_lng = inti_lng + (fract*100)/60;
    fract = modff(google_coord_lng, &inti_lng);
    coord_lng = fract*10000;
    coord_lng *= 10;


//    sprintf(decrypted_coord.lng,"lng,%03u.%05u,%c",(uint16_t)inti,decrypted_coord.coord_lng,(nema_coord.lng_d));
//    LOG_INFO("%s\r\n\n",decrypted_coord.lng);
//    my_puts(decrypted_coord.lng);
//    my_puts("\n\r");

    sprintf((char*)store,"%03u.%05u,%c %03u.%05u,%c",(uint16_t)inti_lat,coord_lat,nema_lat_d,(uint16_t)inti_lng,coord_lng,nema_lng_d);

}


void write_gps(uint8_t *data, uint8_t len){
  // Do some processing
  uint8_t *token = data;
  uint8_t token_cnt = 0;

  uint32_t   days;
  uint32_t   months;
  uint32_t   years;
  uint32_t   hours;
  uint32_t   minutes;
  uint32_t   seconds;
  char      nmea_lat[16];
  char      nmea_lat_d;
  char      nmea_lng[16];
  char      nmea_lng_d;

  // No valid data
  if(!strstr(data, "GPRMC") || strchr(data, 'V')){
      return;
  }

  memset(nmea_lat,0,16);
  memset(nmea_lng,0,16);
  token = strtok(data, ",");

  // This won't hit the last token but we dont care
  while(token != NULL){

      // hhmmss.00
      if(token_cnt == 1){
        char buf[3];
        buf[2] = '\0';

        memcpy(buf, token, 2);
        sscanf(buf, "%u", &hours);

        memcpy(buf, token+2, 2);
        sscanf(buf, "%u", &minutes);

        memcpy(buf, token+4, 2);
        sscanf(buf, "%u", &seconds);
      }
      else if(token_cnt == 3){
//          char buf[11];
//          buf[10] = '\0';
//          memcpy(buf, token, 10);
//        sscanf(token, "%f", &nmea_lat);
          memcpy(nmea_lat, token, 12);

//        printf("%f\n",nmea_lat);

      }
      else if(token_cnt == 4){
          char buf[2];
          buf[1] = '\0';
          memcpy(buf, token, 1);
          sscanf(buf, "%c", &nmea_lat_d);
//        printf("%c\n",nmea_lat_d);
      }
      else if(token_cnt == 5){
//          char buf[11];
//          buf[10] = '\0';
//          memcpy(buf, token, 10);
//        sscanf(token, "%f", &nmea_lng);
          memcpy(nmea_lng, token, 12);
//        printf("%f\n",nmea_lng);
      }
      else if(token_cnt == 6){
         char buf[2];
         buf[1] = '\0';
         memcpy(buf, token, 1);
        sscanf(buf, "%c", &nmea_lng_d);
//        printf("%c\n",nmea_lng_d);
      }
      // ddmmyy
      else if(token_cnt == 8){
        char buf[3];
        buf[2] = '\0';

        memcpy(buf, token, 2);
        sscanf(buf, "%d", &days);

        memcpy(buf, token+2, 2);
        sscanf(buf, "%d", &months);

        memcpy(buf, token+4, 2);
        sscanf(buf, "%d", &years);
      }

      token = strtok(NULL, ",");
      token_cnt++;
  }

  // mm/dd/yy hh:mm:ss
  sprintf(stored.timestamp, "%.02d/%.02d/%.02d %.02d:%.02d:%.02d", months, days, years, hours, minutes, seconds);

//  memcpy(stored.gps, data, 20);
//  stored.gps[20] = '\0';
  nema_2_goog(nmea_lat,nmea_lat_d,nmea_lng,nmea_lng_d,stored.gps);
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



