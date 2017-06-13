/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup ble_sdk_app_beacon_main main.c
 * @{
 * @ingroup ble_sdk_app_beacon
 * @brief Beacon Transmitter Sample Application main file.
 *
 * This file contains the source code for a beacon transmitter sample application.
 */

#include <stdbool.h>
#include <stdint.h>
#include "ble_conn_params.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "pstorage_mod.h"
#include "app_gpiote.h"
#include "app_timer.h"
#include "app_button.h"
#include "pca20006.h"
#include "ble_bcs.h"

#define LED_R_MSK  (1UL << LED_RED)
#define LED_G_MSK  (1UL << LED_GREEN)
#define LED_B_MSK  (1UL << LED_BLUE)

#define BOOTLOADER_BUTTON_PIN           BUTTON_0                                    /**< Button used to enter DFU mode. */
#define CONFIG_MODE_BUTTON_PIN          BUTTON_1                                    /**< Button used to enter config mode. */
#define APP_GPIOTE_MAX_USERS            2  
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)  

#define CONFIG_MODE_LED_MSK            (LED_R_MSK | LED_G_MSK)                      /**< Blinking yellow when device is in config mode */
#define BEACON_MODE_LED_MSK            (LED_R_MSK | LED_B_MSK)                      /**< Blinking purple when device is advertising as beacon */

#define ASSERT_LED_PIN_NO      LED_RED
#define ADVERTISING_LED_PIN_NO LED_BLUE
#define CONNECTED_LED_PIN_NO   LED_GREEN

#define APP_CFG_NON_CONN_ADV_TIMEOUT  0                                             /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables timeout. */
//#define NON_CONNECTABLE_ADV_INTERVAL  MSEC_TO_UNITS(760, UNIT_0_625_MS)            /**< The advertising interval for non-connectable advertisement (852 ms). This value can vary between 100ms to 10.24s). */
#define NON_CONNECTABLE_ADV_INTERVAL  MSEC_TO_UNITS(851, UNIT_0_625_MS) 
/**/

// -----------------------------------
#define DEVICE_NAME                     "Beacon Config"                             /**< Name of device. Will be included in the advertising data. */

#define APP_ADV_INTERVAL                MSEC_TO_UNITS(851, UNIT_0_625_MS)           /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      30                                          /**< The advertising timeout (in units of seconds). */

//#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(500, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.5 seconds). */
//#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(1000, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (1 second). */
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(30, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(300, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (1 second). */


#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time between each call to sd_ble_gap_conn_param_update after the first (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_TIMEOUT               30                                          /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                  0                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

// -----------------------------------------

#define APP_BEACON_INFO_LENGTH        0x17                              /**< Total length of information advertised by the beacon. */
#define APP_ADV_DATA_LENGTH           0x15                              /**< Length of manufacturer specific data in the advertisement. */
#define APP_DEVICE_TYPE               0x02                              /**< 0x02 refers to beacon. */
//#define APP_MEASURED_RSSI             0xBB                              /**< The beacon's measured RSSI at 1 meter distance in dBm. */
#define APP_MEASURED_RSSI             0xAC 



//#define APP_COMPANY_IDENTIFIER        0x004c 
#define APP_COMPANY_IDENTIFIER        0x0059                            /**< Company identifier for Nordic Semiconductor. as per www.bluetooth.org. */
#define APP_MAJOR_VALUE               0x01, 0x02                        /**< Major value used to identify beacons. */ 
#define APP_MINOR_VALUE               0x03, 0x04                        /**< Minor value used to identify beacons. */ 

/*******

#define APP_BEACON_UUID               0x01, 0x12, 0x23, 0x34, \

**************/


//#define APP_BEACON_UUID               0x01, 0x13, 0x33, 0x33, \
//                                      0x45, 0x56, 0x67, 0x78, \
//                                      0x89, 0x9a, 0xab, 0xbc, \
//                                      0xcd, 0xde, 0xef, 0xf0            /**< Proprietary UUID for beacon. */

#define APP_BEACON_UUID              0x50, 0xDC, 0xB6, 0xF6, \
                                     0x91, 0x5A, 0x41, 0x42, \
																		 0xA6, 0xFE, 0xFD, 0xA7, \
                                     0xB4, 0x41, 0x86, 0x09            /**< Proprietary UUID for beacon. */




#define DEAD_BEEF                     0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_TIMER_PRESCALER         0                                   /**< RTC prescaler value used by app_timer */
#define APP_TIMER_MAX_TIMERS        3                                   /**< One for each module + one for ble_conn_params + a few extra */
#define APP_TIMER_OP_QUEUE_SIZE     3                                   /**< Maximum number of timeout handlers pending execution */

#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(app_timer_event_t)       /**< Maximum size of scheduler events. Note that scheduler BLE stack events do not contain any data, as the events are being pulled from the stack in the event handler. */
#define SCHED_QUEUE_SIZE                10                              /**< Maximum number of events in the scheduler queue. */

#define LED_PWM_PERIOD_TICKS APP_TIMER_TICKS(20, APP_TIMER_PRESCALER)   /**< LED Softblink PWM period in app_timer ticks. */ 
#define LED_PWM_DYTY_CYCLE_MAX      20                                  /**< LED Softblink PWM duty cycle max in %. */
#define LED_PWM_DYTY_CYCLE_MIN       0                                  /**< LED Softblink PWM duty cycle min in %. */
#define LED_PWM_OFF_PAUSE_MS      4000                                  /**< LED Softblink PWM pause between blinks in ms. */

#define MAGIC_FLASH_BYTE 0x42                                           /**< Magic byte used to recognise that flash has been written */

typedef enum
{
    beacon_mode_config,
    beacon_mode_normal
}beacon_mode_t;

typedef struct
{
    uint8_t  magic_byte;
    uint8_t  beacon_uuid[16];    
    uint8_t  major_value[2];
    uint8_t  minor_value[2];
    uint8_t  measured_rssi;
}flash_db_layout_t;

typedef union
{
    flash_db_layout_t data;
    uint32_t padding[CEIL_DIV(sizeof(flash_db_layout_t), 4)];
}flash_db_t;

flash_db_t           *p_flash_db;
static pstorage_handle_t    pstorage_block_id;

static ble_gap_sec_params_t m_sec_params;                               /**< Security requirements for this application. */
static uint16_t             m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
static ble_bcs_t            m_bcs;

static uint32_t        s_leds_bit_mask = 0;
static app_timer_id_t  s_leds_timer_id;

static ble_gap_adv_params_t m_adv_params;                               /**< Parameters to be passed to the stack when starting advertising. */
static uint8_t clbeacon_info[APP_BEACON_INFO_LENGTH] =                /**< Information advertised by the beacon. */
{
    APP_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this 
                         // implementation. 
    APP_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the 
                         // manufacturer specific data in this implementation.
    APP_BEACON_UUID,  // 128 bit UUID value. 
    APP_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between beacons. 
    APP_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between beacons. 
    APP_MEASURED_RSSI    // Manufacturer specific information. The beacon's measured TX power in 
                         // this implementation. 
};

/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    nrf_gpio_pin_clear(ASSERT_LED_PIN_NO);

    // This call can be used for debug purposes during application development.
    // @note CAUTION: Activating this code will write the stack to flash on an error.
    //                This function should NOT be used in a final product.
    //                It is intended STRICTLY for development/debugging purposes.
    //                The flash write will happen EVEN if the radio is active, thus interrupting
    //                any communication.
    //                Use with care. Un-comment the line below to use.
    //ble_debug_assert_handler(error_code, line_num, p_file_name);

    // On assert, the system can only recover on reset.
    NVIC_SystemReset();
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static void wait_for_flash_and_reset(void)
{
    uint32_t err_code;
    
    err_code = pstorage_access_wait();
    APP_ERROR_CHECK(err_code);
    
    NVIC_SystemReset();
}

static void storage_access_complete_handler(void)
{
    
}

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    uint32_t err_code;
    uint32_t count;
    
    pstorage_sys_event_handler(sys_evt);
    // Check if storage access is in progress.
    err_code = pstorage_access_status_get(&count);
    if ((err_code == NRF_SUCCESS) && (count == 0))
    {
        storage_access_complete_handler();
    }    
}

static void leds_timer_handler(void *p_context)
{
    static bool leds_on = false;
    static uint32_t duty_cycle = 0;
    static bool counting_up = true;
    uint32_t new_timeout_ticks;
    uint32_t err_code;
    
    if (leds_on)
    {   
        NRF_GPIO->OUTSET = s_leds_bit_mask; // Turn off LEDs
        leds_on = false;
        
        if (counting_up)
        {
            duty_cycle++;
            if(duty_cycle >= LED_PWM_DYTY_CYCLE_MAX)
            {
                // Max PWM duty cycle is reached, start decrementing.
                counting_up = false;
            }
        }
        else
        {
            duty_cycle--;
            if(duty_cycle == LED_PWM_DYTY_CYCLE_MIN)
            {
                // Min PWM duty cycle is reached, start incrementing.
                counting_up = true;
            }
        }
        
        if (duty_cycle == LED_PWM_DYTY_CYCLE_MIN)
        {
            new_timeout_ticks = APP_TIMER_TICKS(LED_PWM_OFF_PAUSE_MS, APP_TIMER_PRESCALER);
        }
        else
        {
            new_timeout_ticks = (LED_PWM_PERIOD_TICKS / 100) * (100 - duty_cycle) + 5;
        }
    }
    else
    {
        NRF_GPIO->OUTCLR = s_leds_bit_mask;
        leds_on = true;
        
        new_timeout_ticks = (LED_PWM_PERIOD_TICKS / 100) * duty_cycle + 5;
    }
    
    err_code = app_timer_start(s_leds_timer_id, new_timeout_ticks, NULL);
    APP_ERROR_CHECK(err_code);    
}

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by this application and starts softblink timer.
 */
static void leds_init(void)
{
    uint32_t err_code;
    
    nrf_gpio_cfg_output(ADVERTISING_LED_PIN_NO);
    nrf_gpio_cfg_output(CONNECTED_LED_PIN_NO);
    nrf_gpio_cfg_output(ASSERT_LED_PIN_NO);
    

    nrf_gpio_cfg_output(LED1);
    nrf_gpio_cfg_output(LED1);


    nrf_gpio_pin_set(ADVERTISING_LED_PIN_NO);
    nrf_gpio_pin_set(CONNECTED_LED_PIN_NO);
    nrf_gpio_pin_set(ASSERT_LED_PIN_NO);
    
    err_code = app_timer_create(&s_leds_timer_id, APP_TIMER_MODE_SINGLE_SHOT, leds_timer_handler);
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_start(s_leds_timer_id, LED_PWM_PERIOD_TICKS, NULL);
    APP_ERROR_CHECK(err_code);   
}

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(beacon_mode_t mode)
{
    if (mode == beacon_mode_normal)
    {
        uint32_t        err_code;
        ble_advdata_t   advdata;
        uint8_t         flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
        ble_advdata_manuf_data_t manuf_specific_data;
        
        manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;
        manuf_specific_data.data.p_data        = (uint8_t *) clbeacon_info;
        manuf_specific_data.data.size          = APP_BEACON_INFO_LENGTH;

        // Build and set advertising data.
        memset(&advdata, 0, sizeof(advdata));

        advdata.name_type               = BLE_ADVDATA_NO_NAME;
        advdata.flags.size              = sizeof(flags);
        advdata.flags.p_data            = &flags;
        advdata.p_manuf_specific_data   = &manuf_specific_data;

        err_code = ble_advdata_set(&advdata, NULL);
        APP_ERROR_CHECK(err_code);

        // Initialize advertising parameters (used when starting advertising).
        memset(&m_adv_params, 0, sizeof(m_adv_params));

        m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
        m_adv_params.p_peer_addr = NULL;                             // Undirected advertisement.
        m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
        m_adv_params.interval    = NON_CONNECTABLE_ADV_INTERVAL;
        m_adv_params.timeout     = APP_CFG_NON_CONN_ADV_TIMEOUT;
    }
    else if (mode == beacon_mode_config)
    {
        uint32_t      err_code;
        ble_advdata_t advdata;
        ble_advdata_t scanrsp;
        uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
        
        ble_uuid_t adv_uuids[] = {{BCS_UUID_SERVICE, m_bcs.uuid_type}};

        // Build and set advertising data
        memset(&advdata, 0, sizeof(advdata));
        advdata.name_type               = BLE_ADVDATA_FULL_NAME;
        advdata.include_appearance      = true;
        advdata.flags.size              = sizeof(flags);
        advdata.flags.p_data            = &flags;
        
        memset(&scanrsp, 0, sizeof(scanrsp));
        scanrsp.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
        scanrsp.uuids_complete.p_uuids  = adv_uuids;
        
        err_code = ble_advdata_set(&advdata, &scanrsp);
        APP_ERROR_CHECK(err_code);
        
        // Initialize advertising parameters (used when starting advertising).
        memset(&m_adv_params, 0, sizeof(m_adv_params));
        
        m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
        m_adv_params.p_peer_addr = NULL;
        m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
        m_adv_params.interval    = APP_ADV_INTERVAL;
        m_adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;           
    }
    else
    {
        APP_ERROR_CHECK_BOOL(false);
    }
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handeling button presses.
 */
static void button_handler(uint8_t pin_no)
{
    if(pin_no == CONFIG_MODE_BUTTON_PIN)
    { nrf_gpio_pin_set( LED1 );
        wait_for_flash_and_reset();
		   
    }
    else if (pin_no == BOOTLOADER_BUTTON_PIN)
    {nrf_gpio_pin_set( LED2 );
        wait_for_flash_and_reset();
		   
    }
    else
    {
        APP_ERROR_CHECK_BOOL(false);
    }
}

/**@brief Function for initializing the app_button module.
 */
static void buttons_init(void)
{
    // @note: Array must be static because a pointer to it will be saved in the Button handler 
    // module.
    static app_button_cfg_t buttons[] =
    {
        {CONFIG_MODE_BUTTON_PIN, false, BUTTON_PULL, button_handler},
        {BOOTLOADER_BUTTON_PIN, false, BUTTON_PULL, button_handler}
    };

    APP_BUTTON_INIT(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY, true);
}


/**@brief Function for handling the writes to the configuration characteristics of the beacon configuration service. 
 * @detail A pointer to this function is passed to the service in its init structure. 
 */
static void beacon_write_handler(ble_bcs_t * p_lbs, beacon_data_type_t type, uint8_t *data)
{
    uint32_t err_code;
    
    static flash_db_t tmp;
    
    memcpy(&tmp, p_flash_db, sizeof(flash_db_t));
    
    tmp.data.magic_byte = MAGIC_FLASH_BYTE;
    
    switch(type)
    {
        case beacon_maj_min_data:
            tmp.data.major_value[0] = data[0];
            tmp.data.major_value[1] = data[1];
            tmp.data.minor_value[0] = data[2];
            tmp.data.minor_value[1] = data[3];
            break;
        case beacon_measured_rssi_data:
            tmp.data.measured_rssi = data[0];
            break;
        case beacon_uuid_data:
            for(uint8_t i = 0; i < 16; i++)
            {
                tmp.data.beacon_uuid[i] = data[i];
            }
            break;
        default:
            break;
    }
    
    err_code = pstorage_clear(&pstorage_block_id, sizeof(flash_db_t));
    APP_ERROR_CHECK(err_code);
    
    err_code = pstorage_store(&pstorage_block_id, (uint8_t *)&tmp, sizeof(flash_db_t), 0);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function shall be used to setup all the necessary GAP (Generic Access Profile) 
 *          parameters of the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);    
    
    err_code = sd_ble_gap_device_name_set(&sec_mode, DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t err_code;
    ble_bcs_init_t init;
    
    init.beacon_write_handler = beacon_write_handler;
    init.p_beacon_info = clbeacon_info;
    
    err_code = ble_bcs_init(&m_bcs, &init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing security parameters.
 */
static void sec_params_init(void)
{
    m_sec_params.timeout      = SEC_PARAM_TIMEOUT;
    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;  
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in]   p_evt   Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
    
    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;
    
    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code = NRF_SUCCESS;
    static ble_gap_evt_auth_status_t m_auth_status;
    ble_gap_enc_info_t *             p_enc_info;
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            wait_for_flash_and_reset();
            break;
            
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, 
                                                   BLE_GAP_SEC_STATUS_SUCCESS, 
                                                   &m_sec_params);
            break;
            
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0);
            break;

        case BLE_GAP_EVT_AUTH_STATUS:
            m_auth_status = p_ble_evt->evt.gap_evt.params.auth_status;
            break;
            
        case BLE_GAP_EVT_SEC_INFO_REQUEST:
            p_enc_info = &m_auth_status.periph_keys.enc_info;
            if (p_enc_info->div == p_ble_evt->evt.gap_evt.params.sec_info_request.div)
            {
                err_code = sd_ble_gap_sec_info_reply(m_conn_handle, p_enc_info, NULL);
            }
            else
            {
                // No keys found for this device
                err_code = sd_ble_gap_sec_info_reply(m_conn_handle, NULL, NULL);
            }
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISEMENT)
            { 
                wait_for_flash_and_reset();
            }
            break;

        default:
            break;
    }

    APP_ERROR_CHECK(err_code);
}

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_bcs_on_ble_evt(&m_bcs, p_ble_evt);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;
    
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, true);//true（开启调度功能）与softdevice_sys_evt_handler_set函数对应
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);    
}

//------------------------------------------------------------------//
/**
 * @brief Function for handling pstorage events
 */
static void pstorage_ntf_cb(pstorage_handle_t *  p_handle,
                            uint8_t              op_code,
                            uint32_t             result,
                            uint8_t *            p_data,
                            uint32_t             data_len)
{
    APP_ERROR_CHECK(result);
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    bool config_mode = false;
		//bool config_mode = true;
    pstorage_module_param_t pstorage_param;
    
    // Initialize.
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);
    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
    buttons_init();    
    leds_init();

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
    
    err_code = app_button_is_pushed(CONFIG_MODE_BUTTON_PIN, &config_mode);//手动切换广播类型
    APP_ERROR_CHECK(err_code);    
    
    ble_stack_init();
    
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);
    
    pstorage_param.cb = pstorage_ntf_cb;
    pstorage_param.block_size = sizeof(flash_db_t);
    pstorage_param.block_count = 1;
    
    err_code = pstorage_register(&pstorage_param, &pstorage_block_id);
    APP_ERROR_CHECK(err_code);
    
    p_flash_db = (flash_db_t *)pstorage_block_id.block_id;
    
    if (p_flash_db->data.magic_byte != MAGIC_FLASH_BYTE)
    {
        flash_db_t tmp;
        
        tmp.data.magic_byte = MAGIC_FLASH_BYTE;
        memcpy(tmp.data.beacon_uuid, &clbeacon_info[2], 16);
        tmp.data.major_value[0] = clbeacon_info[18] = 0x00;
        tmp.data.major_value[1] = clbeacon_info[19] = 0x03;
				tmp.data.minor_value[0] = clbeacon_info[20] = 0x00;
        //tmp.data.minor_value[0] = clbeacon_info[20] = 0x00;
				tmp.data.minor_value[1] = clbeacon_info[21] = 0x01;//(uint8_t)NRF_FICR->ER[3];
        //tmp.data.minor_value[1] = clbeacon_info[21] = (uint8_t)NRF_FICR->ER[0];
        tmp.data.measured_rssi  = clbeacon_info[22] = APP_MEASURED_RSSI;//设置beacon??uuid,major,minor??
        
        err_code = pstorage_clear(&pstorage_block_id, sizeof(flash_db_t));
        APP_ERROR_CHECK(err_code);
        
        err_code = pstorage_store(&pstorage_block_id, (uint8_t *)&tmp, sizeof(flash_db_t), 0);
        APP_ERROR_CHECK(err_code);
        
        err_code = pstorage_access_wait();
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        memcpy(&clbeacon_info[2], p_flash_db->data.beacon_uuid, 16);
        clbeacon_info[18] = p_flash_db->data.major_value[0];
        clbeacon_info[19] = p_flash_db->data.major_value[1];
        clbeacon_info[20] = p_flash_db->data.minor_value[0];
        clbeacon_info[21] = p_flash_db->data.minor_value[1];;
        clbeacon_info[22] = p_flash_db->data.measured_rssi;
    }
    
    if(config_mode)
    {
        gap_params_init();
        services_init();
        advertising_init(beacon_mode_config);
        conn_params_init();
        sec_params_init();
        s_leds_bit_mask |= CONFIG_MODE_LED_MSK;
    }
    else
    {
        advertising_init(beacon_mode_normal);
        s_leds_bit_mask |= BEACON_MODE_LED_MSK;
    }
    
    // Start execution.
    advertising_start();

    // Enter main loop.
    for (;;)
    {
        app_sched_execute();
        power_manage();
    }
}

/**
 * @}
 */
