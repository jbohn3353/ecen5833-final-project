/**
 * @file      ble.h
 * @brief     Declares functions and datatypes of our BLE module for the EFM32
 * @author    James Bohn
 * @date      Oct 6, 2023
 */

#ifndef SRC_BLE_H_
#define SRC_BLE_H_

#include "sl_bt_api.h"

#define UINT8_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); }

#define UINT32_TO_BITSTREAM(p, n) { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
                                    *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }

#define UINT32_TO_FLOAT(m, e) (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))

// BLE Data Structure, save all of our private BT data in here.
// Modern C (circa 2021 does it this way)
// typedef ble_data_struct_t is referred to as an anonymous struct definition
typedef struct {
  // values that are common to servers and clients
  bd_addr myAddress;
  uint8_t myAddressType;
  uint8_t conn_handle;

  // values unique for server
  // The advertising set handle allocated from Bluetooth stack.
  uint8_t advertisingSetHandle;
  uint8_t conn_open;
  uint32_t flash_write_p;
  uint32_t flash_read_p;

} ble_data_struct_t;

// Function Declarations

ble_data_struct_t * bleGetStruct();

void handle_ble_event(sl_bt_msg_t *evt);

#endif /* SRC_BLE_H_ */
