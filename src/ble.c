/**
 * @file      ble.h
 * @brief     Defines functions of our BLE module for the EFM32
 * @author    James Bohn
 * @date      Oct 6, 2023
 * @attirbute Found some useful API calls from the SOC thermometer project
 */

/// @attribute The SoC thermometer example project

#include "ble.h"
#include "gatt_db.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "log.h"

#include "data.h"
#include "scheduler.h"

#define ADVERTISING_INTERVAL_MIN_MS   (250)
#define ADVERTISING_INTERVAL_MAX_MS   (250)
#define CONNECTION_INTERVAL_MIN_MS    (75)
#define CONNECTION_INTERVAL_MAX_MS    (75)
#define SLAVE_LATENCY_MS              (300)
#define SLAVE_LATENCY_CNT             ((SLAVE_LATENCY_MS/CONNECTION_INTERVAL_MAX_MS) - 1)

// must be at least (1 + SLAVE_LATENCY_CNT) * (2 * CONNECTION_INTERVAL_MS)
#define SUPERVISION_TIMEOUT_MS        ((2 + SLAVE_LATENCY_CNT) * (2 * CONNECTION_INTERVAL_MAX_MS))

#define CUSTOM_PACKET_SIZE            (64)

// BLE private data
static ble_data_struct_t ble_data = {0};

/// @brief provide global access to BLE state static variable by returning a pointer
/// @return ble_data_struct_t* - pointer to stored BLE data
ble_data_struct_t* bleGetStruct()
{
  return &ble_data;
} // bleGetStruct

/// @brief handle events provided from the ble stacks (event handler, not state machine
///        even though some data is saved)
/// @param sl_bt_msg_t *evt - BT stack event to be acted upon
void handle_ble_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch(SL_BT_MSG_ID(evt->header))
  {
    case sl_bt_evt_system_external_signal_id:;
      schedEvt_e sig = evt->data.evt_system_external_signal.extsignals;

      uint8_t packet_data[CUSTOM_PACKET_SIZE];

      if(sig == evtLETIMER0_UF){
        if(fetch_data(packet_data, CUSTOM_PACKET_SIZE)){
//          spi_write_page(ble_data.flash_write_p*0x100, packet_data, CUSTOM_PACKET_SIZE);

          sc = sl_bt_gatt_server_write_attribute_value(
                                     gattdb_pp_data,
                                     0,
                                     CUSTOM_PACKET_SIZE,
                                     (uint8_t *)&packet_data);
        }
      }

      break;
    //**************************************************************************
    // Events common to servers and clients
    //**************************************************************************

    // This event indicates the device has started and the radio is ready
    // do not call API commands before receiving this!
    //PACKSTRUCT( struct sl_bt_evt_system_boot_s
    // {
    //   uint16_t major;      /**< Major release version */
    //   uint16_t minor;      /**< Minor release version */
    //   uint16_t patch;      /**< Patch release number */
    //   uint16_t build;      /**< Build number */
    //   uint32_t bootloader; /**< Bootloader version */
    //   uint16_t hw;         /**< Hardware type */
    //   uint32_t hash;       /**< Version hash */
    // });
    case sl_bt_evt_system_boot_id:;
      //sl_bt_evt_system_boot_t boot_info = evt->data.evt_system_boot;

      ble_data.flash_write_p = 0;
      ble_data.flash_read_p = 0;

      // Returns the unique BT device address
      sc = sl_bt_system_get_identity_address(&ble_data.myAddress, &ble_data.myAddressType);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("Error with sl_bt_system_get_identity_address: %x", sc);
      }

      // Creates an advertising set to use when the slave device wants to
      // advertise its presence. The handle created by this call is only
      // to be used for advertising API calls.
      sc = sl_bt_advertiser_create_set(&ble_data.advertisingSetHandle);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("Error with sl_bt_advertiser_create_set: %x", sc);
      }

      // Sets the timing to transmit advertising packets
      sc = sl_bt_advertiser_set_timing(ble_data.advertisingSetHandle,
                                       ADVERTISING_INTERVAL_MIN_MS * 1.6,
                                       ADVERTISING_INTERVAL_MAX_MS * 1.6,
                                       0, /*default*/
                                       0 /*default*/);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("Error with sl_bt_advertiser_set_timing: %x", sc);
      }

      sc = sl_bt_advertiser_start(ble_data.advertisingSetHandle,
                                  sl_bt_advertiser_general_discoverable,
                                  sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("Error with sl_bt_advertiser_start: %x", sc);
      }

      break;

    // This event indicates that a new connection was opened.
    //PACKSTRUCT( struct sl_bt_evt_connection_opened_s
    //{
    //  bd_addr address;      /**< Remote device address */
    //  uint8_t address_type; /**< Enum @ref sl_bt_gap_address_type_t. Remote device
    //                             address type. Values:
    //                               - <b>sl_bt_gap_public_address (0x0):</b> Public
    //                                 device address
    //                               - <b>sl_bt_gap_static_address (0x1):</b> Static
    //                                 device address
    //                               - <b>sl_bt_gap_random_resolvable_address
    //                                 (0x2):</b> Resolvable private random address
    //                               - <b>sl_bt_gap_random_nonresolvable_address
    //                                 (0x3):</b> Non-resolvable private random
    //                                 address */
    //  uint8_t master;       /**< Device role in connection. Values:
    //                               - <b>0:</b> Peripheral
    //                               - <b>1:</b> Central */
    //  uint8_t connection;   /**< Handle for new connection */
    //  uint8_t bonding;      /**< Bonding handle. Values:
    //                               - <b>SL_BT_INVALID_BONDING_HANDLE (0xff):</b> No
    //                                 bonding
    //                               - <b>Other:</b> Bonding handle */
    //  uint8_t advertiser;   /**< The local advertising set that this connection was
    //                             opened to. Values:
    //                               - <b>SL_BT_INVALID_ADVERTISING_SET_HANDLE
    //                                 (0xff):</b> Invalid value or not applicable.
    //                                 Ignore this field
    //                               - <b>Other:</b> The advertising set handle */
    //});
    case sl_bt_evt_connection_opened_id:;
      sl_bt_evt_connection_opened_t open_info = evt->data.evt_connection_opened;

      ble_data.conn_open = 1;
      ble_data.conn_handle = open_info.connection;

      // Stop advertising
      sc = sl_bt_advertiser_stop(ble_data.advertisingSetHandle);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("Error with sl_bt_advertiser_stop: %x", sc);
      }

      // Send a request with a set of parameters to the master
      sc = sl_bt_connection_set_parameters(ble_data.conn_handle,
                                           (CONNECTION_INTERVAL_MIN_MS*4)/5,
                                           (CONNECTION_INTERVAL_MAX_MS*4)/5,
                                           SLAVE_LATENCY_CNT,
                                           SUPERVISION_TIMEOUT_MS/10,
                                           0,     /*default*/
                                           0xffff /*default*/);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("Error with sl_bt_connection_set_parameters: %x", sc);
      }

      break;

    // This event indicates that a connection was closed.
    //PACKSTRUCT( struct sl_bt_evt_connection_closed_s
    //{
    //  uint16_t reason;     /**< Reason of connection close */
    //  uint8_t  connection; /**< Handle of the closed connection */
    //});
    case sl_bt_evt_connection_closed_id:;
      //sl_bt_evt_connection_closed_t close_info = evt->data.evt_connection_closed;

      ble_data.conn_open = 0;

      sc = sl_bt_advertiser_start(ble_data.advertisingSetHandle,
                                  sl_bt_advertiser_general_discoverable,
                                  sl_bt_advertiser_connectable_scannable);
      if (sc != SL_STATUS_OK)
      {
        LOG_ERROR("Error with sl_bt_advertiser_start: %x", sc);
      }

      break;

    // Informational. Triggered whenever the connection parameters are changed
    // and at any time a connection is established
    //PACKSTRUCT( struct sl_bt_evt_connection_parameters_s
    //{
    //  uint8_t  connection;    /**< Connection handle */
    //  uint16_t interval;      /**< Connection interval. Time = Value x 1.25 ms */
    //  uint16_t latency;       /**< Peripheral latency (how many connection intervals
    //                               the peripheral can skip) */
    //  uint16_t timeout;       /**< Supervision timeout. Time = Value x 10 ms */
    //  uint8_t  security_mode; /**< Enum @ref sl_bt_connection_security_t. Connection
    //                               security mode. Values:
    //                                 - <b>sl_bt_connection_mode1_level1 (0x0):</b>
    //                                   No security
    //                                 - <b>sl_bt_connection_mode1_level2 (0x1):</b>
    //                                   Unauthenticated pairing with encryption
    //                                 - <b>sl_bt_connection_mode1_level3 (0x2):</b>
    //                                   Authenticated pairing with encryption
    //                                 - <b>sl_bt_connection_mode1_level4 (0x3):</b>
    //                                   Authenticated Secure Connections pairing with
    //                                   encryption using a 128-bit strength
    //                                   encryption key */
    //  uint16_t txsize;        /**< Maximum Data Channel PDU Payload size that the
    //                               controller can send in an air packet */
    //});
    case sl_bt_evt_connection_parameters_id:;
      sl_bt_evt_connection_parameters_t param_info = evt->data.evt_connection_parameters;

//      LOG_INFO("Conn interval : %u ms", (uint32_t) (param_info.interval*5)/4);
//      LOG_INFO("Slave latency: %u ms", (uint32_t) (param_info.latency + 1)*(param_info.interval*5)/4);
//      LOG_INFO("Supervision timeout: %u ms", (uint32_t) param_info.timeout*10);

      break;

    //**************************************************************************
    // Events only for servers
    //**************************************************************************

    //Indicates either:
    // A local Client Characteristic Configuration descriptor (CCCD) was
    // changed by the remote GATT client, or That a confirmation from the
    // remote GATT Client was received upon a successful reception of the
    // indication I.e. we sent an indication from our server to the client
    // with sl_bt_gatt_server_send_indication()
    //PACKSTRUCT( struct sl_bt_evt_gatt_server_characteristic_status_s
    //{
    //  uint8_t  connection;          /**< Connection handle */
    //  uint16_t characteristic;      /**< GATT characteristic handle. This value is
    //                                     normally received from the
    //                                     gatt_characteristic event. */
    //  uint8_t  status_flags;        /**< Enum @ref
    //                                     sl_bt_gatt_server_characteristic_status_flag_t.
    //                                     Describes whether Client Characteristic
    //                                     Configuration was changed or if a
    //                                     confirmation was received. Values:
    //                                       - <b>sl_bt_gatt_server_client_config
    //                                         (0x1):</b> Characteristic client
    //                                         configuration has been changed.
    //                                       - <b>sl_bt_gatt_server_confirmation
    //                                         (0x2):</b> Characteristic confirmation
    //                                         has been received. */
    //  uint16_t client_config_flags; /**< Enum @ref
    //                                     sl_bt_gatt_server_client_configuration_t.
    //                                     This field carries the new value of the
    //                                     Client Characteristic Configuration. If the
    //                                     status_flags is 0x2 (confirmation
    //                                     received), the value of this field can be
    //                                     ignored. */
    //  uint16_t client_config;       /**< The handle of client-config descriptor. */
    //});
    case sl_bt_evt_gatt_server_characteristic_status_id:;
      sl_bt_evt_gatt_server_characteristic_status_t char_status_info = evt->data.evt_gatt_server_characteristic_status;

//      // check if its a permission update and not an ack
//      if(char_status_info.status_flags == sl_bt_gatt_server_client_config)
//      {
//        // check which characteristic its for (might change to switch case later)
//        if(char_status_info.characteristic == gattdb_temperature_measurement)
//        {
//          // set flag based on what the permission change actually is (might change to switch case later)
//          if(char_status_info.client_config_flags == sl_bt_gatt_server_indication ||
//             char_status_info.client_config_flags == sl_bt_gatt_server_notification_and_indication)
//          {
//            ble_data.temp_indication_en = 1;
//          }
//          else
//          {
//            ble_data.temp_indication_en = 0;
//          }
//        }
//      }
//
//      if(char_status_info.status_flags == sl_bt_gatt_server_confirmation)
//      {
//        ble_data.indication_inflight = 0;
//      }

      break;

    // Possible event from calling sl_bt_gatt_server_send_indication() - i.e. we
    // never received a confirmation for a previously transmitted indication.
    //PACKSTRUCT( struct sl_bt_evt_gatt_server_indication_timeout_s
    //{
    //  uint8_t connection; /**< Connection handle */
    //});
    case sl_bt_evt_gatt_server_indication_timeout_id:;
      //sl_bt_evt_gatt_server_indication_timeout_t ind_timeot_info = evt->data.evt_gatt_server_indication_timeout;

      break;

    //**************************************************************************
    // Events only for clients
    //**************************************************************************

  }
} // handle_ble_event()
