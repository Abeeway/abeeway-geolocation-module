/*!
 * \file aos_error.h
 *
 * \brief Handle Hardware and firmware unrecoverable errors
 *
 * \verbatim
 * Once an error occurs, the system will reset if the release version is used.
 * When the macro DEBUG is defined, the system will loop indefinitely allowing
 * interactions with a debugger.
 *
 * The error is stored in non init area. So it is available across resets.
 * It is strongly recommended to clear the error once it has been taken into account or
 * when a normal reset occurs. This will avoid reading an incoherent reset cause under
 * normal reset (requested by the user).
 *
 * The stored information type depends on the error code. Refer to the error code definitions
 * to know which king of information is stored.
 * \endverbatim
 *
 */
#pragma once

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup aos_error AOS error management
 *
 * \addtogroup aos_error
 * @{
 */
#define AOS_ERROR_MAX_FILE_NAME 80			//!< Max size of a file name. File name Sizes above this value are truncated.


/*!
 * \enum aos_error_code_t
 *
 * \brief Error codes
 */
typedef enum {
	AOS_ERROR_NONE = 0,
	AOS_ERROR_HW_NMI,								//!< Hardware non-maskable interrupt fault. Registers available.
	AOS_ERROR_HW_FAULT,								//!< Hardware fault. Registers available.
	AOS_ERROR_HW_MPU,								//!< Hardware MPU fault. Registers available.
	AOS_ERROR_HW_BUS,								//!< Hardware bus fault. Registers available.
	AOS_ERROR_HW_USAGE,								//!< Hardware usage fault. Registers available.
	AOS_ERROR_HW_IRQ,								//!< Unexpected interruption. Register available
	AOS_ERROR_HW_WDOG,								//!< Hardware watchdog
	AOS_ERROR_HW_BOR,								//!< Brown-out occur
	AOS_ERROR_SW_BASE,								//!< First Software error
	AOS_ERROR_SW_ST_HAL_ERROR = AOS_ERROR_SW_BASE,	//!< ST HAL error. No error data
	AOS_ERROR_SW_FREERTOS_ASSERT,					//!< FreeRTOS assertion. File name and line available
	AOS_ERROR_SW_FREERTOS_TASK_OVF,					//!< FreeRTOS Task stack overflow. File name = task name
	AOS_ERROR_SW_BLE_ASSERT,						//!< Bluethooth assertion
	AOS_ERROR_SW_RTC_FAIL,							//!< Real Time Clock peripheral fails to start. File name an line available
	AOS_ERROR_SW_LORA_FAIL,							//!< LoRa unrecoverable failure
	AOS_ERROR_SW_DEBUG,								//!< Used to debug

	// Must be the last
	AOS_ERROR_SW_APP_START							//!< First available error code for the application
} aos_error_code_t;


/*!
 * \struct aos_error_extra_info_t
 *
 * \brief Extra MCU registers usable for debug purpose
 * \note These registers are relevant only in the cases:
 *	1. if (CSFR & 0x0080) is not null then MMFAR is relevant
 *	2. if (CSFR & 0x8000) is not null then BFAR is relevant
 */
typedef struct {
	uint32_t mmfar;				//!< MemManaget Fault Address Register
	uint32_t bfar;				//!< Bus Fault Address Register
} aos_error_extra_info_t;

/*!
 * \struct aos_error_reg_info_t
 *
 * \brief MCU registers values obtained during a hardware fault
 */
typedef struct {
	uint32_t r0;				//!< General purpose register R0
	uint32_t r1;				//!< General purpose register R1
	uint32_t r2;				//!< General purpose register R2
	uint32_t r3;				//!< General purpose register R3
	uint32_t r12;				//!< General purpose register R12
	uint32_t lr;				//!< Link register
	uint32_t pc;				//!< Program counter
	uint32_t xpsr;				//!< Program status register
	int32_t irq;				//!< IRQ number. Relevant only in case of AOS_ERROR_HW_IRQ error.
	uint32_t csfr;				//!< Configurable Fault Status register
	uint32_t dsfr;				//!< Debug Fault status Register
	uint32_t asfr;				//!< Auxiliary Fault Status Register
	uint32_t hsfr;				//!< Hardware Fault Status Register
	aos_error_extra_info_t extra; //!< Additional register information
} aos_error_reg_info_t;

/*!
 * \struct aos_error_info_t
 *
 * \brief Generic structure handling the error
 *
 * \note The content of the structure depends on the error code:
 * - Error code less than AOS_ERROR_SW_BASE: reg_info filled.
 * - Error code above or equals to AOS_ERROR_SW_BASE: file and line filled.
 *   The file field contains the FreeRTOS task name for the error AOS_ERROR_SW_FREERTOS_TASK_OVF.
 */
typedef struct {
	aos_error_code_t code;							//!< Error code
	union {
		struct {
			char file[AOS_ERROR_MAX_FILE_NAME];		//!< File name in which the fault occurred
			unsigned line;							//!< Line in the file at which the fault occured.
		};
		aos_error_reg_info_t reg_info;				//!< Register information
	};
} aos_error_info_t;


/*!
 * \fn void aos_error_init(void)
 *
 * \brief Initialization function
 *
 * \note The function is called by AOS itself. Applications should not not use it.
 */
void aos_error_init(void);


/*!
 * \fn aos_error_info_t* aos_error_get(void)
 *
 * \brief Retrieve the error information block
 *
 * \return The error information block
 */
aos_error_info_t* aos_error_get(void);

/*!
 * \fn void aos_error_clear(void)
 *
 * \brief Clear the error
 *
 */
void aos_error_clear(void);


/*!
 * \fn void aos_error_trigger(aos_error_code_t code, const char *file, unsigned line)
 *
 * \brief trigger an error
 *
 * \param code Error code
 * \param file Locate the file name from which this function is called. Usually, the macro __FILE__ is used.
 * \param line Code line number inside the file from which this function is called. Usually, usually the __LINE__ macro is used.
 */
void aos_error_trigger(aos_error_code_t code, const char *file, unsigned line);


/*!
 * \fn void aos_error_trigger_nmi(uint32_t *stack_pointer)
 *
 * \brief Called by the NMI exception handler
 *
 * \param stack_pointer Stack pointer containing MCU registers
 * \note The function is called by AOS only. Applications should not not use it.
 */
void aos_error_trigger_nmi(uint32_t *stack_pointer);

/*!
 * \fn void aos_error_watchdog(void)
 *
 * \brief Called after reset if the watchdog has fired.
 *
 * \note The function is called by AOS only. Applications should not not use it.
 */
void aos_error_watchdog(void);


/*!
 * \fn void aos_error_trigger_debug(const char *fmt, ...)
 *
 * \brief Debug by halting the core. At the next reset, the error stored in noinit will be provided.
 *
 * \param fmt: Formatting string
 * \note Max size of the full string is 32 bytes. Error is inside the file.
 */
void aos_error_trigger_debug(const char *fmt, ...);

/*! @}*/
#ifdef __cplusplus
}
#endif
