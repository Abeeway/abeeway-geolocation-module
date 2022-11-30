/*
 * aos_mcu_pins.h - constants (names) for the MCU pins, as defined
 * on the module schematics. External pins are prefixed with PIN_EXT,
 * module internally connected pins are prefixed with PIN_MCU (so we
 * can spot assignment errors in the code easily).
 *
 *  Created on: Feb 14, 2022
 *      Author: marc
 */
#pragma once

#include "gpio.h"

#define PIN_EXT_BOOT0		PH_3
#define PIN_EXT_EN_VDD_GNSS_BU PA_15
#define PIN_EXT_EN_VDD_GNSS	PC_10
#define PIN_EXT_GNSS_PPS	PB_15
#define PIN_EXT_GPIO1	PD_14
#define PIN_EXT_GPIO2	PD_13
#define PIN_EXT_GPIO3	PD_7
#define PIN_EXT_GPIO4	PD_0
#define PIN_EXT_GPIO5	PC_9
#define PIN_EXT_GPIO6	PD_6
#define PIN_EXT_GPIO7	PD_10
#define PIN_EXT_GPIO8	PC_4
#define PIN_EXT_GPIO9	PC_5
#define PIN_EXT_GPIO10	PC_11
#define PIN_EXT_I2C_INT1	PC_2
#define PIN_EXT_I2C_INT2	PC_3
#define PIN_EXT_I2C_POWER	PB_9
#define PIN_EXT_I2C_SCL		PB_8
#define PIN_EXT_I2C_SDA	PB_7
#define PIN_EXT_LPUART_CTS	PB_13
#define PIN_EXT_LPUART_RTS	PB_12
#define PIN_EXT_LPUART_RX	PB_10
#define PIN_EXT_LPUART_TX	PB_11
#define PIN_EXT_PWM_CTRL	PB_6
#define PIN_EXT_SPI_CS	PD_2
#define PIN_EXT_SPI_MISO	PD_3
#define PIN_EXT_SPI_MOSI	PD_4
#define PIN_EXT_SPI_SCK	PD_1
#define PIN_EXT_ST_LSCO_32K	PA_2
#define PIN_EXT_ST_NRST_GNSS	PE_4
#define PIN_EXT_ST_USART_RX_GNSS	PA_9
#define PIN_EXT_ST_USART_TX_GNSS	PA_10
#define PIN_EXT_SWCLK	PA_14
#define PIN_EXT_SWDIO	PA_13
#define PIN_EXT_SWO		PB_3
#define PIN_EXT_USB_DM	PA_11
#define PIN_EXT_USB_DP	PA_12
#define PIN_EXT_USR_ADC		PA_0
#define PIN_EXT_VBAT_SENSE	PA_1
#define PIN_MCU_2G4_CTRL_BLE	PA_3
#define PIN_MCU_2G4_CTRL_WIFI	PB_2
#define PIN_MCU_GNSS_CTRL_LR	PE_3
#define PIN_MCU_GNSS_CTRL_MT	PB_1
#define PIN_MCU_GNSS_FORCE_ON PC_12
#define PIN_MCU_I2C3_SCL	PC_0
#define PIN_MCU_I2C3_SDA	PB_14
#define PIN_MCU_LR_BUSY	PC_8
#define PIN_MCU_LR_DIO9		PC_1
#define PIN_MCU_LR_MISO		PA_6
#define PIN_MCU_LR_MOSI	PA_7
#define PIN_MCU_LR_NSS	PA_4
#define PIN_MCU_LR_SCK	PA_5
#define PIN_MCU_OSC32_IN	PC_14
#define PIN_MCU_OSC32_OUT	PC_15
#define PIN_MCU_SECURITY_SUP	PE_0
#define PIN_MCU_ST_NRST_LR	PB_5
#define PIN_MCU_ST_VDD_GNSS_LNA	PB_0
