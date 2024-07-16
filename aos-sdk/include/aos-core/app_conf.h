/*!
 * \file app_conf.h
 *
 * \brief Application configuration file for STM32WPAN Middleware
 *
 * \copyright 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_CONF_H
#define APP_CONF_H

#include "hw.h"
#include "hw_conf.h"
#include "hw_if.h"
#include "ble_bufsize.h"
#include "aos_thread_priority.h"
#include "aos_log.h"


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup app_ble_conf BLE Application configuration file for STM32WPAN Middleware
 *
 * \addtogroup app_ble_conf
 * @{
 */

/******************************************************************************
 * Application Config
 ******************************************************************************/

/**< generic parameters ******************************************************/

#define CFG_TX_POWER                      (0x19)             //!< BLE TX power 0dBm
#define MAX_BOUNDED_COUNT                 (1)                //!< Maximum number of devices that can create a bond with the device

/**
 * Define Advertising parameters
 */
#define CFG_ADV_BD_ADDRESS                (0x000000000000)   //!< BLE MAC ADDRESS
#define CFG_BLE_ADDRESS_TYPE              STATIC_RANDOM_ADDR //!< Bluetooth address types defined in ble_legacy.h
#if 0
#define CFG_FAST_CONN_ADV_INTERVAL_MIN    (0x80)             //!< Minimum fast advertisement interval 80ms
#define CFG_FAST_CONN_ADV_INTERVAL_MAX    (0xA0)             //!< Maximum fast advertisement interval 100ms
#define CFG_LP_CONN_ADV_INTERVAL_MIN      (0x640)            //!< Minimum slow advertisement interval 1s
#define CFG_LP_CONN_ADV_INTERVAL_MAX      (0xFA0)            //!< Maximum slow advertisement interval 2.5s
#else
#define CFG_FAST_CONN_ADV_INTERVAL_MIN    (500)             //!< Minimum fast advertisement interval 500ms
#define CFG_FAST_CONN_ADV_INTERVAL_MAX    (500)             //!< Maximum fast advertisement interval 500ms
#define CFG_LP_CONN_ADV_INTERVAL_MIN      (2000)            //!< Minimum slow advertisement interval 2s
#define CFG_LP_CONN_ADV_INTERVAL_MAX      (2000)            //!< Maximum slow advertisement interval 2s
#endif
/**
 * Define IO Authentication
 */
#define CFG_BONDING_MODE                 (1)                 //!< Bonding mode
#define CFG_FIXED_PIN                    (111111)            //!< Fixed pin to be used in the pairing process
#define CFG_USED_FIXED_PIN               (0)                 //!< 0 implies use fixed pin and 1 implies request for passkey (during pairing request)
#define CFG_ENCRYPTION_KEY_SIZE_MAX      (16)                //!< BLE encryption key max size
#define CFG_ENCRYPTION_KEY_SIZE_MIN      (8)                 //!< BLE encryption key min size

/**
 * Define IO capabilities
 */
#define CFG_IO_CAPABILITY_DISPLAY_ONLY       (0x00)          //!< BLE IO capability display only
#define CFG_IO_CAPABILITY_DISPLAY_YES_NO     (0x01)          //!< BLE IO capability display yes or no
#define CFG_IO_CAPABILITY_KEYBOARD_ONLY      (0x02)          //!< BLE IO capability keyboard only
#define CFG_IO_CAPABILITY_NO_INPUT_NO_OUTPUT (0x03)          //!< BLE IO capability no input no output
#define CFG_IO_CAPABILITY_KEYBOARD_DISPLAY   (0x04)          //!< BLE IO capability keyboard display

#define CFG_IO_CAPABILITY              CFG_IO_CAPABILITY_NO_INPUT_NO_OUTPUT  //!< Default BLE IO capability

/**
 * Define MITM modes
 */
#define CFG_MITM_PROTECTION_NOT_REQUIRED     (0x00)          //!< Men In The Middle protection not required
#define CFG_MITM_PROTECTION_REQUIRED         (0x01)          //!< Men In The Middle protection required

#define CFG_MITM_PROTECTION             CFG_MITM_PROTECTION_NOT_REQUIRED     //!< Default MITM protection

/**
 * Define Secure Connections Support
 */
#define CFG_SECURE_NOT_SUPPORTED             (0x00)          //!< Secure Connections not supported
#define CFG_SECURE_OPTIONAL                  (0x01)          //!< Secure Connections optional
#define CFG_SECURE_MANDATORY                 (0x02)          //!< Secure Connections mandatory

#define CFG_SC_SUPPORT                 CFG_SECURE_MANDATORY   //!< Default secure connections

/**
 * Define Keypress Notification Support
 */
#define CFG_KEYPRESS_NOT_SUPPORTED           (0x00)          //!< Keypress notification not supported
#define CFG_KEYPRESS_SUPPORTED               (0x01)          //!< Keypress notification supported

#define CFG_KEYPRESS_NOTIFICATION_SUPPORT    CFG_KEYPRESS_NOT_SUPPORTED      //!< Default keypress notification support

/**
 * Numeric Comparison Answers
 */
#define YES (0x01)      //!< YES mean true
#define NO  (0x00)      //!< No mean false

/**
 * Device name configuration for Generic Access Service
 */
#define CFG_GAP_DEVICE_NAME             "ABW-MODULE"           //!< Device name in generic access service
#define CFG_GAP_DEVICE_NAME_LENGTH      (10)                   //!< Device name length in generic access service

/**
 * Define PHY
 */
#define ALL_PHYS_PREFERENCE                             0x00   //!< Preference all PHYS
#define RX_2M_PREFERRED                                 0x02   //!< Prefer 2M PHY for RX
#define TX_2M_PREFERRED                                 0x02   //!< Prefer 2M PHY for TX
#define TX_1M                                           0x01   //!< TX with PHY 1M
#define TX_2M                                           0x02   //!< TX with PHY 2M
#define RX_1M                                           0x01   //!< RX with PHY 1M
#define RX_2M                                           0x02   //!< RX with PHY 2M


#define CFG_BLE_IRK     {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0}   //!< Identity root key used to derive LTK and CSRK

#define CFG_BLE_ERK     {0xFE, 0xDC, 0xBA, 0x09, 0x87, 0x65, 0x43, 0x21, 0xFE, 0xDC, 0xBA, 0x09, 0x87, 0x65, 0x43, 0x21}   //!< Encryption root key used to derive LTK and CSRK

/**
 * SMPS supply
 * SMPS not used when Set to 0
 * SMPS used when Set to 1
 */
#define CFG_USE_SMPS    0      //!< SMPS supply

/**< specific parameters */
/*****************************************************/

/**
* AD Element - Group B Feature
*/
/* LSB - Second Byte */
#define CFG_FEATURE_OTA_REBOOT                  (0x20)   //!< Support OTA Bit Mask
/* USER CODE BEGIN Specific_Parameters */

#define CONN_L(x) ((int)((x)/0.625f))				//!< Convert in step of 0.625
#define CONN_P(x) ((int)((x)/1.25f))				//!< Conver in setp of 1.25

/*!
 * \brief  L2CAP Connection Update request parameters used for test only with smart Phone
 */
#define L2CAP_REQUEST_NEW_CONN_PARAM             0		//!< Request for new connection parameter

#define L2CAP_INTERVAL_MIN              CONN_P(1000) 	//!< Min interval = 1s
#define L2CAP_INTERVAL_MAX              CONN_P(1000) 	//!< Max interval = 1s
#define L2CAP_SLAVE_LATENCY             0x0000			//!< Slave latency
#define L2CAP_TIMEOUT_MULTIPLIER        600 			//!< Timeout multiplier. 0x1F4

/* USER CODE END Specific_Parameters */

/******************************************************************************
 * BLE Stack
 ******************************************************************************/
/**
 * Maximum number of simultaneous connections that the device will support.
 * Valid values are from 1 to 8
 */
#define CFG_BLE_NUM_LINK            2   //!< Maximum number of simultaneous connections that the device will support

/**
 * Maximum number of Services that can be stored in the GATT database.
 * Note that the GAP and GATT services are automatically added so this parameter should be 2 plus the number of user services
 */
#define CFG_BLE_NUM_GATT_SERVICES   18   //!< Maximum number of Services that can be stored in the GATT database

/**
 * Maximum number of Attributes
 * (i.e. the number of characteristic + the number of characteristic values + the number of descriptors, excluding the services)
 * that can be stored in the GATT database.
 * Note that certain characteristics and relative descriptors are added automatically during device initialization
 * so this parameters should be 9 plus the number of user Attributes
 */
#define CFG_BLE_NUM_GATT_ATTRIBUTES 100   //!< Maximum number of Attributes that can be stored in the GATT database (48 for standard services and 52 for custom services)

/**
 * Maximum supported ATT_MTU size
 * This parameter is ignored by the CPU2 when CFG_BLE_OPTIONS is set to 1"
 */
#define CFG_BLE_MAX_ATT_MTU             (156)   //!< Maximum supported ATT_MTU size

/**
 * Size of the storage area for Attribute values
 *  This value depends on the number of attributes used by application. In particular the sum of the following quantities (in octets) should be made for each attribute:
 *  - attribute value length
 *  - 5, if UUID is 16 bit; 19, if UUID is 128 bit
 *  - 2, if server configuration descriptor is used
 *  - 2*DTM_NUM_LINK, if client configuration descriptor is used
 *  - 2, if extended properties is used
 *  The total amount of memory needed is the sum of the above quantities for each attribute.
 * This parameter is ignored by the CPU2 when CFG_BLE_OPTIONS is set to 1"
 */
#define CFG_BLE_ATT_VALUE_ARRAY_SIZE    (1976)   //!< Size of the storage area for Attribute values

/**
 * Prepare Write List size in terms of number of packet
 * This parameter is ignored by the CPU2 when CFG_BLE_OPTIONS is set to 1"
 */
#define CFG_BLE_PREPARE_WRITE_LIST_SIZE         BLE_PREP_WRITE_X_ATT(CFG_BLE_MAX_ATT_MTU)   //!< Prepare Write List size in terms of number of packet

/**
 * Number of allocated memory blocks
 * This parameter is overwritten by the CPU2 with an hardcoded optimal value when the parameter when CFG_BLE_OPTIONS is set to 1
 */
#define CFG_BLE_MBLOCK_COUNT            (BLE_MBLOCKS_CALC(CFG_BLE_PREPARE_WRITE_LIST_SIZE, CFG_BLE_MAX_ATT_MTU, CFG_BLE_NUM_LINK))   //!< Number of allocated memory blocks

/**
 * Enable or disable the Extended Packet length feature. Valid values are 0 or 1.
 */
#define CFG_BLE_DATA_LENGTH_EXTENSION   1   //!< Enable or disable the Extended Packet length feature


#define CFG_BLE_SLAVE_SCA   500             //!< Sleep clock accuracy in Slave mode (ppm value)

/**
 * Sleep clock accuracy in Master mode
 * 0 : 251 ppm to 500 ppm
 * 1 : 151 ppm to 250 ppm
 * 2 : 101 ppm to 150 ppm
 * 3 : 76 ppm to 100 ppm
 * 4 : 51 ppm to 75 ppm
 * 5 : 31 ppm to 50 ppm
 * 6 : 21 ppm to 30 ppm
 * 7 : 0 ppm to 20 ppm
 */
#define CFG_BLE_MASTER_SCA   0              //!< Sleep clock accuracy in Master mode

/**
 *  Source for the low speed clock for RF wake-up
 *  1 : external high speed crystal HSE/32/32
 *  0 : external low speed crystal: LSE
 */
#define CFG_BLE_LSE_SOURCE  0               //!< Source for the low speed clock for RF wake-up


#define CFG_BLE_HSE_STARTUP_TIME  0x148     //!< Start up time of the high speed (16 or 32 MHz) crystal oscillator in units of 625/256 us (~2.44 us)


#define CFG_BLE_MAX_CONN_EVENT_LENGTH  (0xFFFFFFFF)    //!< Maximum duration of the connection event when the device is in Slave mode in units of 625/256 us (~2.44 us)

/**
 * Viterbi Mode
 * 1 : enabled
 * 0 : disabled
 */
#define CFG_BLE_VITERBI_MODE  1    //!< Viterbi Mode

/**
 * BLE stack Options flags to be configured with:
 * - SHCI_C2_BLE_INIT_OPTIONS_LL_ONLY
 * - SHCI_C2_BLE_INIT_OPTIONS_LL_HOST
 * - SHCI_C2_BLE_INIT_OPTIONS_NO_SVC_CHANGE_DESC
 * - SHCI_C2_BLE_INIT_OPTIONS_WITH_SVC_CHANGE_DESC
 * - SHCI_C2_BLE_INIT_OPTIONS_DEVICE_NAME_RO
 * - SHCI_C2_BLE_INIT_OPTIONS_DEVICE_NAME_RW
 * - SHCI_C2_BLE_INIT_OPTIONS_EXT_ADV
 * - SHCI_C2_BLE_INIT_OPTIONS_NO_EXT_ADV
 * - SHCI_C2_BLE_INIT_OPTIONS_CS_ALGO2
 * - SHCI_C2_BLE_INIT_OPTIONS_NO_CS_ALGO2
 * - SHCI_C2_BLE_INIT_OPTIONS_POWER_CLASS_1
 * - SHCI_C2_BLE_INIT_OPTIONS_POWER_CLASS_2_3
 * which are used to set following configuration bits:
 * (bit 0): 1: LL only
 *          0: LL + host
 * (bit 1): 1: no service change desc.
 *          0: with service change desc.
 * (bit 2): 1: device name Read-Only
 *          0: device name R/W
 * (bit 3): 1: extended advertizing supported       [NOT SUPPORTED]
 *          0: extended advertizing not supported   [NOT SUPPORTED]
 * (bit 4): 1: CS Algo #2 supported
 *          0: CS Algo #2 not supported
 * (bit 7): 1: LE Power Class 1
 *          0: LE Power Class 2-3
 * other bits: reserved (shall be set to 0)
 */
#define CFG_BLE_OPTIONS  (SHCI_C2_BLE_INIT_OPTIONS_LL_HOST | \
						  SHCI_C2_BLE_INIT_OPTIONS_WITH_SVC_CHANGE_DESC | \
						  SHCI_C2_BLE_INIT_OPTIONS_DEVICE_NAME_RW | \
						  SHCI_C2_BLE_INIT_OPTIONS_NO_EXT_ADV | \
						  SHCI_C2_BLE_INIT_OPTIONS_NO_CS_ALGO2 | \
						  SHCI_C2_BLE_INIT_OPTIONS_POWER_CLASS_2_3)    //!< BLE stack Options flags

#define CFG_BLE_MAX_COC_INITIATOR_NBR   (32)    //!< Maximum number of connection-oriented channels in initiator mode

#define CFG_BLE_MIN_TX_POWER            (-40)   //!< Minimum transmit power in dBm supported by the Controller

#define CFG_BLE_MAX_TX_POWER            (6)     //!< Maximum transmit power in dBm supported by the Controller

/**
 * BLE Rx model configuration flags to be configured with:
 * - SHCI_C2_BLE_INIT_RX_MODEL_AGC_RSSI_LEGACY
 * - SHCI_C2_BLE_INIT_RX_MODEL_AGC_RSSI_BLOCKER
 * which are used to set following configuration bits:
 * (bit 0): 1: agc_rssi model improved vs RF blockers
 *          0: Legacy agc_rssi model
 * other bits: reserved (shall be set to 0)
 */

#define CFG_BLE_RX_MODEL_CONFIG         SHCI_C2_BLE_INIT_RX_MODEL_AGC_RSSI_LEGACY    //!< BLE Rx model configuration flags

/******************************************************************************
 * Transport Layer
 ******************************************************************************/
/**
 * Queue length of BLE Event
 * This parameter defines the number of asynchronous events that can be stored in the HCI layer before
 * being reported to the application. When a command is sent to the BLE core coprocessor, the HCI layer
 * is waiting for the event with the Num_HCI_Command_Packets set to 1. The receive queue shall be large
 * enough to store all asynchronous events received in between.
 * When CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE is set to 27, this allow to store three 255 bytes long asynchronous events
 * between the HCI command and its event.
 * This parameter depends on the value given to CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE. When the queue size is to small,
 * the system may hang if the queue is full with asynchronous events and the HCI layer is still waiting
 * for a CC/CS event, In that case, the notification TL_BLE_HCI_ToNot() is called to indicate
 * to the application a HCI command did not receive its command event within 30s (Default HCI Timeout).
 */
#define CFG_TLBLE_EVT_QUEUE_LENGTH 5    //!< Queue length of BLE Event
/**
 * This parameter should be set to fit most events received by the HCI layer. It defines the buffer size of each element
 * allocated in the queue of received events and can be used to optimize the amount of RAM allocated by the Memory Manager.
 * It should not exceed 255 which is the maximum HCI packet payload size (a greater value is a lost of memory as it will
 * never be used)
 * It shall be at least 4 to receive the command status event in one frame.
 * The default value is set to 27 to allow receiving an event of MTU size in a single buffer. This value maybe reduced
 * further depending on the application.
 */
#define CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE 255    //!< The buffer size of each element allocated in the queue of received events, Set to 255 with the memory manager and the mailbox

#define TL_BLE_EVENT_FRAME_SIZE ( TL_EVT_HDR_SIZE + CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE )    //!< BLE event frame size
/******************************************************************************
 * UART interfaces
 ******************************************************************************/

/**
 * Select UART interfaces
 */
#define CFG_DEBUG_TRACE_UART        hw_uart1    //!< Config debug on UART port
#define CFG_CONSOLE_MENU            hw_uart1    //!< Console menu
/******************************************************************************
 * USB interface
 ******************************************************************************/

#define CFG_USB_INTERFACE_ENABLE    0    //!< Enable/Disable USB interface

/******************************************************************************
 * IPCC interface
 ******************************************************************************/

/**
 * The IPCC is dedicated to the communication between the CPU2 and the CPU1
 * and shall not be modified by the application
 * The two following definitions shall not be modified
 */
#define HAL_IPCC_TX_IRQHandler(...)  HW_IPCC_Tx_Handler( )    //!< TX IPCC
#define HAL_IPCC_RX_IRQHandler(...)  HW_IPCC_Rx_Handler( )    //!< RX IPCC

/******************************************************************************
 * Low Power
 ******************************************************************************/
/**
 *  When set to 1, the low power mode is enable
 *  When set to 0, the device stays in RUN mode
 */
#define CFG_LPM_SUPPORTED    1    //!< Low Power Mode support

/******************************************************************************
 * RTC interface
 ******************************************************************************/
#define HAL_RTCEx_WakeUpTimerIRQHandler(...)  HW_TS_RTC_Wakeup_Handler( )    //!< RTC interface

/******************************************************************************
 * Timer Server
 ******************************************************************************/
/**
 *  CFG_RTC_WUCKSEL_DIVIDER:  This sets the RTCCLK divider to the wakeup timer.
 *  The lower is the value, the better is the power consumption and the accuracy of the timerserver
 *  The higher is the value, the finest is the granularity
 *
 *  CFG_RTC_ASYNCH_PRESCALER: This sets the asynchronous prescaler of the RTC. It should as high as possible ( to output
 *  clock as low as possible) but the output clock should be equal or higher frequency compare to the clock feeding
 *  the wakeup timer. A lower clock speed would impact the accuracy of the timer server.
 *
 *  CFG_RTC_SYNCH_PRESCALER: This sets the synchronous prescaler of the RTC.
 *  When the 1Hz calendar clock is required, it shall be sets according to other settings
 *  When the 1Hz calendar clock is not needed, CFG_RTC_SYNCH_PRESCALER should be set to 0x7FFF (MAX VALUE)
 *
 *  CFG_RTCCLK_DIVIDER_CONF:
 *  Shall be set to either 0,2,4,8,16
 *  When set to either 2,4,8,16, the 1Hhz calendar is supported
 *  When set to 0, the user sets its own configuration
 *
 *  The following settings are computed with LSI as input to the RTC
 */

#define CFG_RTCCLK_DIVIDER_CONF 0    //!< RTC clock divider configuration

#if (CFG_RTCCLK_DIVIDER_CONF == 0)
/**
 * Custom configuration
 * It does not support 1Hz calendar
 * It divides the RTC CLK by 16
 */

#define CFG_RTCCLK_DIV  (16)                //!< RTC clock divider
#define CFG_RTC_WUCKSEL_DIVIDER (0)         //!< RTCCLK divider to the wakeup timer
#define CFG_RTC_ASYNCH_PRESCALER (0x0F)     //!< Asynchronous prescaler of the RTC
#define CFG_RTC_SYNCH_PRESCALER (0x7FFF)    //!< Synchronous prescaler of the RTC

#else

#if (CFG_RTCCLK_DIVIDER_CONF == 2)
/**
 * It divides the RTC CLK by 2
 */
#define CFG_RTC_WUCKSEL_DIVIDER (3)    //!< RTCCLK divider to the wakeup timer
#endif

#if (CFG_RTCCLK_DIVIDER_CONF == 4)
/**
 * It divides the RTC CLK by 4
 */
#define CFG_RTC_WUCKSEL_DIVIDER (2)    //!< RTCCLK divider to the wakeup timer
#endif

#if (CFG_RTCCLK_DIVIDER_CONF == 8)
/**
 * It divides the RTC CLK by 8
 */
#define CFG_RTC_WUCKSEL_DIVIDER (1)   //!< RTCCLK divider to the wakeup timer
#endif

#if (CFG_RTCCLK_DIVIDER_CONF == 16)
/**
 * It divides the RTC CLK by 16
 */
#define CFG_RTC_WUCKSEL_DIVIDER (0)   //!< RTCCLK divider to the wakeup timer
#endif

#define CFG_RTCCLK_DIV              CFG_RTCCLK_DIVIDER_CONF                                   //!< RTC clock divider
#define CFG_RTC_ASYNCH_PRESCALER    (CFG_RTCCLK_DIV - 1)                                      //!< Asynchronous prescaler of the RTC
#define CFG_RTC_SYNCH_PRESCALER     (DIVR( LSE_VALUE, (CFG_RTC_ASYNCH_PRESCALER+1) ) - 1 )    //!< Synchronous prescaler of the RTC

#endif

/** tick timer values */
#define CFG_TS_TICK_VAL           DIVR( (CFG_RTCCLK_DIV * 1000000), LSE_VALUE )                     //!< Tick timer values
#define CFG_TS_TICK_VAL_PS        DIVR( ((uint64_t)CFG_RTCCLK_DIV * 1e12), (uint64_t)LSE_VALUE )    //!< Tick timer values

/*!
 * \enum CFG_TimProcID_t
 *
 * \brief Timer Proc ID type.
 *
 */
typedef enum
{
  CFG_TIM_PROC_ID_ISR,       //!< ISR proc ID
  /* USER CODE BEGIN CFG_TimProcID_t */

  /* USER CODE END CFG_TimProcID_t */
} CFG_TimProcID_t;

/******************************************************************************
 * Debug
 ******************************************************************************/
/**
 * When set, this resets some hw resources to set the device in the same state than the power up
 * The FW resets only register that may prevent the FW to run properly
 *
 * This shall be set to 0 in a final product
 *
 */
#define CFG_HW_RESET_BY_FW         0    //!< Resets some hw resources to set the device in the same state than the power up

/**
 * keep debugger enabled while in any low power mode when set to 1
 * should be set to 0 in production
 */
#define CFG_DEBUGGER_SUPPORTED     0    //!< Keep debugger enabled while in any low power mode


#define CFG_DEBUG_BLE_TRACE        0    //!< Trace enable or disable in the BLE services
#define CFG_DEBUG_APP_TRACE        0    //!< Enable or Disable traces in BLE application
#define BLE_DBG_DIS_EN             0    //!< Display trace for DIS service
#define BLE_DBG_BAS_EN             0    //!< Display trace for BAS service
#define BLE_DBG_ESS_EN             0    //!< Display trace for ESS service
#define BLE_DBG_IAS_EN             0    //!< Display trace for IAS service
#define BLE_DBG_LLS_EN             0    //!< Display trace for LLS service
#define BLE_DBG_TPS_EN             0    //!< Display trace for TPS service

#define PRINT_NO_MESG(...)				//!< No debug message

#if (CFG_DEBUG_BLE_TRACE != 0)
#undef PRINT_MESG_DBG
#undef PRINT_LOG_BUFF_DBG

#define PRINT_LOG_BUFF_DBG(...)      do { aos_log_msg(aos_log_module_ble, aos_log_level_debug, false, s); } while (0);
#define PRINT_MESG_DBG(s...)         do { aos_log_msg(aos_log_module_ble, aos_log_level_debug, false, s); } while (0);
#define PRINT_MESG_WARNING(s...)     do { aos_log_msg(aos_log_module_ble, aos_log_level_warning, false, s); } while (0);
#else
#define PRINT_LOG_BUFF_DBG(...)		//!< No Log messages
#define PRINT_MESG_DBG(...)			//!< No debug message
#define PRINT_MESG_WARNING(s...)	//!< No warning message
#endif

/**
 * Macro definition
 */
#if ( BLE_DBG_DIS_EN != 0 )
#define BLE_DBG_DIS_MSG             PRINT_MESG_DBG    //!< Print debug messages for DIS
#else
#define BLE_DBG_DIS_MSG             PRINT_NO_MESG     //!< No print debug messages for DIS
#endif

#if (BLE_DBG_BAS_EN != 0)
#define BLE_DBG_BAS_MSG             PRINT_MESG_DBG    //!< Print debug messages for BAS
#else
#define BLE_DBG_BAS_MSG             PRINT_NO_MESG     //!< No print debug messages for BAS
#endif

#if (BLE_DBG_ESS_EN != 0)
#define BLE_DBG_ESS_MSG             PRINT_MESG_DBG    //!< Print debug messages for ESS
#else
#define BLE_DBG_ESS_MSG             PRINT_NO_MESG     //!< No print debug messages for ESS
#endif

#if (BLE_DBG_IAS_EN != 0)
#define BLE_DBG_IAS_MSG             PRINT_MESG_DBG    //!< Print debug messages for IAS
#else
#define BLE_DBG_IAS_MSG             PRINT_NO_MESG     //!< No print debug messages for IAS
#endif

#if (BLE_DBG_LLS_EN != 0)
#define BLE_DBG_LLS_MSG             PRINT_MESG_DBG    //!< Print debug messages for LLS
#else
#define BLE_DBG_LLS_MSG             PRINT_NO_MESG     //!< No print debug messages for LLS
#endif

#if (BLE_DBG_TPS_EN != 0)
#define BLE_DBG_TPS_MSG             PRINT_MESG_DBG    //!< Print debug messages for TPS
#else
#define BLE_DBG_TPS_MSG             PRINT_NO_MESG     //!< No print debug messages for TPS
#endif

#if (CFG_DEBUG_APP_TRACE != 0)
#define APP_DBG_MSG                 PRINT_MESG_DBG    //!< Print debug messages for BLE APP
#define APP_WRNG_MSG                PRINT_MESG_WARNING//!< Print warning messages for BLE APP
#else
#define APP_DBG_MSG                 PRINT_NO_MESG     //!< No print debug messages for BLE APP
#define APP_WRNG_MSG                PRINT_NO_MESG     //!< No print warning messages for BLE APP
#endif

#if ( (CFG_DEBUG_BLE_TRACE != 0) || (CFG_DEBUG_APP_TRACE != 0) )
#define CFG_DEBUG_TRACE             0                 //!< Enable debug tracing
#define CFG_DEBUG_BLE_TRACE         1                 //!< Enable debug tracing

#endif


/******************************************************************************
 * OTP manager
 ******************************************************************************/
#define CFG_OTP_BASE_ADDRESS    OTP_AREA_BASE         //!< OTP BASE address

#define CFG_OTP_END_ADRESS      OTP_AREA_END_ADDR     //!< OTP END address


/*! @}*/
#ifdef __cplusplus
}
#endif
#endif /*APP_CONF_H */

