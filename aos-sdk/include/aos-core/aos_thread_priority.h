/*
 * \file aos_thread_priority.h
 *
 * \brief freeRTOS thread priorities.
 *
 * \verbatim
 *  The highest values have highest priorities.
 * \endverbatim
 */
#pragma once

#define AOS_THREAD_PRIO_HIGHEST		10			//!< Highest priority
#define AOS_THREAD_PRIO_LOWEST		0			//!< Lowest priority

#define AOS_THREAD_PRIO_SYSTEM 		8			//!< Interrupt trampoline. Highest priority for AOS.
#define AOS_THREAD_PRIO_LORA		7			//!< LoRa processing thread. Require high level for timing constraint
#define AOS_THREAD_PRIO_SHCI_USER	5			//!< BLE SHCI user event
#define AOS_THREAD_PRIO_HCI_USER	4			//!< BLE HCI user event
#define AOS_THREAD_PRIO_GNSS		4			//!< GNSS processing thread
#define AOS_THREAD_PRIO_GEOLOC		3			//!< Geolocation processing thread
#define AOS_THREAD_PRIO_RTOS_TIMERS	3			//!< Free RTOS timer processing (Application timers).

#define AOS_THREAD_PRIO_APPLICATION	1			//!< Application thread (main)
#define AOS_THREAD_PRIO_XMODEM_SRV	4			//!< LR1110 firmware update thread
#define AOS_THREAD_PRIO_CLI_SRV		0			//!< CLI thread processing

#define AOS_THREAD_PRIO_IDLE		0			//!< Idle thread (FreeRTOS)
