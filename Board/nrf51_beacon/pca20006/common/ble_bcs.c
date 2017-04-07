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

#include "ble_bcs.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"


/**@brief Connect event handler.
 *
 * @param[in]   p_bcs       Beacon Configuration Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_bcs_t * p_bcs, ble_evt_t * p_ble_evt)
{
    p_bcs->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Disconnect event handler.
 *
 * @param[in]   p_bcs       Beacon Configuration Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_bcs_t * p_bcs, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_bcs->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Write event handler.
 *
 * @param[in]   p_bcs       Beacon Configuration Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_bcs_t * p_bcs, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    if (p_bcs->beacon_write_handler == NULL)
    {
        return;
    }
    
   if ((p_evt_write->handle == p_bcs->beacon_maj_min_char_handles.value_handle) &&
       (p_evt_write->len == 4))
   {
       p_bcs->beacon_write_handler(p_bcs, beacon_maj_min_data, p_evt_write->data);
   }
   
   if ((p_evt_write->handle == p_bcs->beacon_calib_char_handles.value_handle) &&
       (p_evt_write->len == 1))
   {
       p_bcs->beacon_write_handler(p_bcs, beacon_measured_rssi_data, p_evt_write->data);
   }

   if ((p_evt_write->handle == p_bcs->beacon_id_char_handles.value_handle) &&
       (p_evt_write->len == 16))
   {
       p_bcs->beacon_write_handler(p_bcs, beacon_uuid_data, p_evt_write->data);
   }   
}


void ble_bcs_on_ble_evt(ble_bcs_t * p_bcs, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_bcs, p_ble_evt);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_bcs, p_ble_evt);
            break;
            
        case BLE_GATTS_EVT_WRITE:
            on_write(p_bcs, p_ble_evt);
            break;
            
        default:
            break;
    }
}


/**@brief Add Beacon Configuration characteristic.
 *
 * @param[in]   p_bcs        Beacon Configuration Service structure.
 * @param[in]   p_bcs_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t beacon_maj_min_char_add(ble_bcs_t * p_bcs, const ble_bcs_init_t * p_bcs_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    

    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;
    
    ble_uuid.type = p_bcs->uuid_type;
    ble_uuid.uuid = BCS_UUID_BEACON_MAJ_MIN_CHAR;
    
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 4;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = 4;
    attr_char_value.p_value      = &p_bcs_init->p_beacon_info[18];
    
    return sd_ble_gatts_characteristic_add(p_bcs->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_bcs->beacon_maj_min_char_handles);
}

/**@brief Add Beacon Configuration characteristic.
 *
 * @param[in]   p_bcs        Beacon Configuration Service structure.
 * @param[in]   p_bcs_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t beacon_calib_char_add(ble_bcs_t * p_bcs, const ble_bcs_init_t * p_bcs_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    

    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;
    
    ble_uuid.type = p_bcs->uuid_type;
    ble_uuid.uuid = BCS_UUID_BEACON_CALIB_CHAR;
    
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 1;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = 1;
    attr_char_value.p_value      = &p_bcs_init->p_beacon_info[22];
    
    return sd_ble_gatts_characteristic_add(p_bcs->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_bcs->beacon_calib_char_handles);
}

/**@brief Add Beacon Configuration characteristic.
 *
 * @param[in]   p_bcs        Beacon Configuration Service structure.
 * @param[in]   p_bcs_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t beacon_id_char_add(ble_bcs_t * p_bcs, const ble_bcs_init_t * p_bcs_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    

    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;
    
    ble_uuid.type = p_bcs->uuid_type;
    ble_uuid.uuid = BCS_UUID_BEACON_ID_CHAR;
    
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 16;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = 16;
    attr_char_value.p_value      = &p_bcs_init->p_beacon_info[2];
    
    return sd_ble_gatts_characteristic_add(p_bcs->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_bcs->beacon_id_char_handles);
}

uint32_t ble_bcs_init(ble_bcs_t * p_bcs, const ble_bcs_init_t * p_bcs_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_bcs->conn_handle       = BLE_CONN_HANDLE_INVALID;
    p_bcs->beacon_write_handler = p_bcs_init->beacon_write_handler;
    
    // Add base UUID to softdevice's internal list. 
    ble_uuid128_t base_uuid = BCS_UUID_BASE;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_bcs->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    ble_uuid.type = p_bcs->uuid_type;
    ble_uuid.uuid = BCS_UUID_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_bcs->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    err_code = beacon_maj_min_char_add(p_bcs, p_bcs_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    err_code = beacon_calib_char_add(p_bcs, p_bcs_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = beacon_id_char_add(p_bcs, p_bcs_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }    
    
    return NRF_SUCCESS;
}

