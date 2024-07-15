/*!
 *
 * \file aos_it_priority.h
 *
 * \brief Interrupt priorities.
 *
 * \verbatim
 * 1. The lowest values have highest priorities.
 * 2. The priority groups are not used.
 * 3. The cortex HAL NVIC functions NVIC_xxx should be preferred to HAL_NVIC_xxx
 * \endverbatim
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */
#pragma once

#define AOS_INT_PRIO_HIGHEST	0			//!< Highest priority
#define AOS_INT_PRIO_LOWEST		15			//!< Lowest priority

#define AOS_INT_PRIO_FREERTOS_SYSCALL 5		//!< Highest priority for an interrupt using freeRTOS API

#define AOS_INT_PRIO_USB		AOS_INT_PRIO_FREERTOS_SYSCALL			//!< USB CDC
#define AOS_INT_PRIO_RTC_ALM	AOS_INT_PRIO_FREERTOS_SYSCALL			//!< RTC alarm
#define AOS_INT_PRIO_LPUART1	AOS_INT_PRIO_FREERTOS_SYSCALL			//!< Low power UART
#define AOS_INT_PRIO_USART1		AOS_INT_PRIO_FREERTOS_SYSCALL			//!< USART
#define AOS_INT_PRIO_I2C1_EVT	AOS_INT_PRIO_FREERTOS_SYSCALL			//!< I2C1 event
#define AOS_INT_PRIO_I2C1_ERR	AOS_INT_PRIO_FREERTOS_SYSCALL			//!< I2C1 error
#define AOS_INT_PRIO_I2C3_EVT	AOS_INT_PRIO_FREERTOS_SYSCALL			//!< I2C3 event
#define AOS_INT_PRIO_I2C3_ERR	AOS_INT_PRIO_FREERTOS_SYSCALL			//!< I2C3 error
#define AOS_INT_PRIO_IPCC_RX	AOS_INT_PRIO_FREERTOS_SYSCALL			//!< IPCC (Inter-processor communication channel) RX
#define AOS_INT_PRIO_IPCC_TX	AOS_INT_PRIO_FREERTOS_SYSCALL			//!< IPCC (Inter-processor communication channel) TX
#define AOS_INT_PRIO_HSEM		AOS_INT_PRIO_FREERTOS_SYSCALL			//!< Hardware semaphore. To sync shared peripheral M0/M4
#define AOS_INT_PRIO_GPIO_BASE	AOS_INT_PRIO_FREERTOS_SYSCALL			//!< Highest interruption level for GPIOs
#define AOS_INT_PRIO_FLASH		AOS_INT_PRIO_FREERTOS_SYSCALL			//!< Flash interrupts
#define AOS_INT_PRIO_RTC_WU		AOS_INT_PRIO_FREERTOS_SYSCALL			//!< RTC wakeup

#define AOS_INT_PRIO_SVCALL		AOS_INT_PRIO_HIGHEST	//!< Must be the highest
#define AOS_INT_PRIO_PENDSV		AOS_INT_PRIO_LOWEST		//!< Must be the Lowest
#define AOS_INT_PRIO_SYSTICK	AOS_INT_PRIO_HIGHEST	//!< Must be the highest.

#define AOS_INT_PRIO_HSI_CALIB	AOS_INT_PRIO_HIGHEST 	//!< HSI calibration using the timer
