/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_srv_bcs Beacon Configuration Service
 * @{
 * @ingroup ble_sdk_srv
 * @brief Beacon Configuration Service module.
 *
 * @details This module implements the Beacon Configuration Service with the Battery Level characteristic.
 *          During initialization it adds the Beacon Configuration Service and Battery Level characteristic
 *          to the BLE stack datalbse. Optionally it can also add a Report Reference descriptor
 *          to the Battery Level characteristic (used when including the Beacon Configuration Service in
 *          the HID service).
 *
 *          If specified, the module will support notification of the Battery Level characteristic
 *          through the ble_lbs_battery_level_update() function.
 *          If an event handler is supplied by the application, the LEDButton Service will
 *          generate Beacon Configuration Service events to the application.
 *
 * @note The application must propagate BLE stack events to the Beacon Configuration Service module by calling
 *       ble_bcs_on_ble_evt() from the from the @ref ble_stack_handler callback.
 */

#ifndef BLE_BCS_H__
#define BLE_BCS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

//955Axxxx-0FE2-F5AA-A094-84B8D4F3E8AD
#define BCS_UUID_BASE {0xAD, 0xE8, 0xF3, 0xD4, 0xB8, 0x84, 0x94, 0xA0, 0xAA, 0xF5, 0xE2, 0x0F, 0x00, 0x00, 0x5A, 0x95}

//#define BCS_UUID_BASE {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}
#define BCS_UUID_SERVICE 0x1523
#define BCS_UUID_BEACON_MAJ_MIN_CHAR 0x1526
#define BCS_UUID_BEACON_CALIB_CHAR   0x1525
#define BCS_UUID_BEACON_ID_CHAR      0x1524

typedef enum {
    beacon_maj_min_data,
    beacon_measured_rssi_data,
    beacon_uuid_data
}beacon_data_type_t;

// Forward declaration of the ble_bcs_t type. 
typedef struct ble_bcs_s ble_bcs_t;

/**@brief Beacon Configuration Service event handler type. */
typedef void (*ble_bcs_write_handler_t) (ble_bcs_t * p_bcs, beacon_data_type_t type, uint8_t *data);

/**@brief Beacon Configuration Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_bcs_write_handler_t   beacon_write_handler;
    uint8_t                   *p_beacon_info;
} ble_bcs_init_t;


/**@brief Beacon Configuration Service structure. This contains various status information for the service. */
typedef struct ble_bcs_s
{         
    uint16_t                     service_handle;                 
    ble_gatts_char_handles_t     beacon_maj_min_char_handles;
    ble_gatts_char_handles_t     beacon_calib_char_handles;
    ble_gatts_char_handles_t     beacon_id_char_handles;
    uint8_t                      uuid_type;
    uint16_t                     conn_handle;  
    bool                         is_notifying;
    ble_bcs_write_handler_t      beacon_write_handler;
} ble_lbs_t;


/**@brief Initialize the Beacon Configuration Service.
 *
 * @param[out]  p_bcs       
 * @param[in]   p_bcs_init  
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_bcs_init(ble_bcs_t * p_bcs, const ble_bcs_init_t * p_bcs_init);

/**@brief Beacon Configuration Service BLE stack event handler.
 *
 * @details Handles all events from the BLE stack of interest to the Beacon Configuration Service.
 *
 * @param[in]   p_bcs      Beacon Configuration Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_bcs_on_ble_evt(ble_bcs_t * p_bcs, ble_evt_t * p_ble_evt);


#endif // BLE_BCS_H__

/** @} */
