/*!
 * \file aos_uart.h
 *
 * \brief UART driver.
 *
 * \details This module drives UARTs (LEUART/USART)
 *
 * \verbatim
 * The LEUART accepts the STOP2 mode.
 * The USART accepts the STOP1 mode.
 *
 * The LEUART is clocked by the LSE for baud-rates below or equals to 9600 and by the
 * HSI for baud-rates above.
 *
 * The USART is always clocked by the HSI.

 * For both UARTS and baud-rates above 57600, the HSI clock is maintained during the MCU stop state.
 * This avoids loosing data due to the UART wakeup time at the counterpart of an extra 100 uA.
 *
 * \endverbatim
 *
 *
 * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "aos_system.h"


#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \defgroup aos_uart UART driver
 *
 * \addtogroup aos_uart
 * @{
 */

/*!
 * \enum aos_uart_speed_t
 *
 * \brief Acceptable UART baudrates
 *
 * \warning Do not change the order
 */
typedef enum {
	aos_uart_speed_1200 = 0,		//!< 1200 bauds
	aos_uart_speed_2400,			//!< 2400 bauds
	aos_uart_speed_4800,			//!< 4800 bauds
	aos_uart_speed_9600,			//!< 9600 bauds
	aos_uart_speed_19200,			//!< 19200 bauds
	aos_uart_speed_38400,			//!< 38400 bauds
	aos_uart_speed_57600,			//!< 57600 bauds
	aos_uart_speed_115200,			//!< 115200 bauds
	aos_uart_speed_230400,			//!< 230400 bauds
	aos_uart_speed_460800,			//!< 460800 bauds
	aos_uart_speed_last				//!< Last entry in the enumerated
} aos_uart_speed_t;

/*!
 * \enum aos_uart_stop_bit_t
 *
 * \brief Number of stop bits
 *
 * \warning Do not change the order
 */
typedef enum {
	aos_uart_stop_bit_0_5 = 0,	//!< Half stop bit
	aos_uart_stop_bit_1,		//!< One stop bit
	aos_uart_stop_bit_1_5,		//!< One and a half stop bit
	aos_uart_stop_bit_2,		//!< Two stop bits
	aos_uart_stop_bit_last		//!< Last entry in the enumerated
} aos_uart_stop_bit_t;

/*!
 * \enum aos_uart_parity_bit_t
 *
 * \brief Parity selection
 *
 * \warning Do not change the order
 */
typedef enum {
	aos_uart_parity_none = 0,	//!< No parity
	aos_uart_parity_odd,		//!< Odd parity
	aos_uart_parity_even,		//!< Even parity
	aos_uart_parity_last		//!< Last entry in the enumerated
} aos_uart_parity_bit_t;

/*!
 * \enum aos_uart_data_size_t
 *
 * \brief Data format selection
 *
 * \warning Do not change the order
 */
typedef enum {
	aos_uart_data_size_7_bits,	//!< Data coded on 7 bits
	aos_uart_data_size_8_bits,	//!< Data coded on 8 bits
	aos_uart_data_size_9_bits,	//!< Data coded on 9 bits
	aos_uart_data_size_last		//!< Last entry in the enumerated
} aos_uart_data_size_t;

/*!
 * \enum aos_uart_hw_flow_control_t
 *
 * \brief Hardware flow control (RTS/CTS) selection
 *
 * \warning Do not change the order
 */
typedef enum {
	aos_uart_hw_flow_control_off,	//!< No hardware flow control
	aos_uart_hw_flow_control_on,	//!< Hardware flow control enabled
	aos_uart_hw_flow_control_last	//!< Last entry in the enumerated
} aos_uart_hw_flow_control_t;


/*!
 * \enum aos_uart_type_t
 *
 * \brief UART type
 *
 * \note The USB CDC is considered as an UART
 *
 * \warning Do not change the order
 */
typedef enum {
	aos_uart_type_lpuart1 = 0,	//!< Low power UART
	aos_uart_type_usart1,		//!< USART 1
	aos_uart_type_usb,			//!< USB CDC
	aos_uart_type_custom,		//!< Custom driver
	aos_uart_type_last			//!< Last entry in the enumerated
} aos_uart_type_t;

/*!
 * \brief User callback calls to retrieve RX data.
 *
 * \param user_arg User argument. Opaque for the driver
 *
 * \note: Callback called under the system thread.
 */
typedef void (*aos_uart_rx_callback_t)(void* user_arg);

/*!
 * \struct aos_uart_config_t
 *
 * \brief Structure containing the UART configuration parameters
 */
typedef struct {
	aos_uart_speed_t speed;							//!< UART baudrate
	aos_uart_stop_bit_t stop;						//!< Number of stop bits selection
	aos_uart_parity_bit_t parity;					//!< Parity selection
	aos_uart_data_size_t data_format;				//!< Data format selection
	aos_uart_hw_flow_control_t hard_flow_control;	//!< Hardware flow control selection
	uint16_t tx_buffer_size;						//!< Size of the transmit buffer
	uint16_t rx_buffer_size;						//!< Size of the receive buffer
	uint8_t* tx_buffer;								//!< Transmit buffer. Provided by the user
	uint8_t* rx_buffer;								//!< Receive buffer. Provided by the user
	aos_uart_rx_callback_t user_rx_cb;				//!< User callback called upon characters reception
	void* user_arg;									//!< User argument passed along the callback. Opaque for the driver
	struct {
		struct aos_uart_driver_t* drv;				//!< Custom driver. Must be populated if UART type = custom
		const void* param;							//!< Custom specific config if any.
	} custom_drv;									//!< Custom driver parameters
} aos_uart_config_t;

/*!
 * \struct aos_uart_stats_t
 *
 * \brief Structure containing the UART statistics
 */
typedef struct {
	uint32_t rx_bytes;					//!< Number of bytes received
	uint32_t tx_bytes;					//!< Number of transmitted bytes
	uint32_t wakeup;					//!< Number of times the UART has woken up
	uint32_t sleep_accepted;			//!< Number of times the sleeping mode has been accepted
	uint32_t sleep_refused;				//!< Number of times the sleeping mode has been refused
	uint32_t parity_errors;				//!< Number of RX bytes with a parity error
	uint32_t noise_errors;				//!< Number of RX noise detection error
	uint32_t framing_errors;			//!< Number of RX framing errors
	uint32_t ovr_errors;				//!< Number of RX overrun error
	uint32_t rx_fifo_full;				//!< Number of bytes rejected due to RX FIFO full
} aos_uart_stats_t;



/*!
 * \enum aos_uart_ioctl_type_t
 *
 * \brief Driver IO control requests
 */
typedef enum {
	aos_uart_ioctl_type_flush_tx,					//!< Flush the transmit FIFO. No parameter
	aos_uart_ioctl_type_flush_rx,					//!< Flush the receive FIFO. No parameter
	aos_uart_ioctl_type_get_stats,					//!< Read the statistics. Field data contains the stats
	aos_uart_ioctl_type_clear_stats,				//!< Clear the statistics. No parameter
	aos_uart_ioctl_type_last						//!< Last item in the list
} aos_uart_ioctl_type_t;

/*!
 * \struct aos_uart_ioctl_info_t
 *
 *	\brief Information block belonging to an IO request
 */
typedef struct {
	aos_uart_ioctl_type_t type;					//!< IO request type
	union {
		uint32_t param;							//!< Integer parameter. Reserved for future use.
		aos_uart_stats_t* stats;				//!< Statistics
	};
} aos_uart_ioctl_info_t;


/*
 * ***********************************************************
 * Driver access functions
 * ***********************************************************
 */
/*!
 * \brief Open prototype
 *
 * \param type UART type
 * \param config UART configuration
 *
 * \return The result of the operation
 */
typedef aos_result_t (*aos_uart_open_t)(aos_uart_type_t type, aos_uart_config_t* config);


/*!
 * \brief Close prototype
 *
 * \param type UART type
 *
 * \return The result of the operation
 */
typedef aos_result_t (*aos_uart_close_t)(aos_uart_type_t type);

/*!
 * \brief Read bytes from the UART
 *
 * \param type UART type
 * \param data Buffer where to store the bytes
 * \param length Input:Maximum number of bytes to read. Output: Number of bytes read.
 *
 * \return The result of the operation
 */
typedef aos_result_t (*aos_uart_read_t)(aos_uart_type_t type, uint8_t* data, uint16_t* length);

/*!
 * \brief Write bytes over the UART
 *
 * \param type UART type
 * \param data Buffer where to the bytes to be sent reside.
 * \param length Input:Number of bytes to transmit. Output: Number of bytes actually sent.
 *
 * \return The result of the operation
 */
typedef aos_result_t (*aos_uart_write_t)(aos_uart_type_t type, const uint8_t* data, uint16_t* length);

/*!
 * \brief Control the UART driver
 *
 * \param type UART type
 * \param info IO request information.
 *
 * \return The result of the operation
 */
typedef aos_result_t (*aos_uart_ioctl_t)(aos_uart_type_t type, aos_uart_ioctl_info_t* info);


/*!
 * \struct aos_uart_driver_t
 *
 * \brief Driver access
 */
typedef struct {
	aos_uart_open_t open;			//!< Open the driver
	aos_uart_close_t close;			//!< Close the driver
	aos_uart_read_t read;			//!< Read bytes received over the UART
	aos_uart_write_t write;			//!< Send bytes over the UART
	aos_uart_ioctl_t ioctl;			//!< Control the driver
} aos_uart_driver_t;

/*!
 * \var aos_uart_driver
 *
 * \brief UART driver access
 */
extern aos_uart_driver_t aos_uart_driver;


/*
 * ***********************************************************
 * Facilities
 * ***********************************************************
 */
/*!
 * \fn uint32_t aos_uart_get_speed(aos_uart_speed_t speed);
 *
 * \brief Request the actual UART speed based on the passed enumerated.
 *
 * \param speed UART speed
 *
 * \return The actual speed in integer format. Example the function returns 1200 when the aos_uart_speed_1200 is given.
 */
uint32_t aos_uart_get_speed(aos_uart_speed_t speed);


/*! @}*/
#ifdef __cplusplus
}
#endif
