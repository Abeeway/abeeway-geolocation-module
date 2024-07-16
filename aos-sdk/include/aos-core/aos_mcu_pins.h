/*
 * aos_mcu_pins.h - constants (names) for the MCU pins, as defined
 * on the module schematics. External pins are prefixed with PIN_EXT,
 * module internally connected pins are prefixed with PIN_MCU (so we
 * can spot assignment errors in the code easily).
 *
 *  Created on: Feb 14, 2022
 */
#pragma once

/*!
 * STM32 GPIO pin names
 */
typedef enum {
    PA_0 = 0, PA_1, PA_2, PA_3, PA_4, PA_5, PA_6, PA_7, PA_8, PA_9, PA_10, PA_11, PA_12, PA_13, PA_14, PA_15,
    PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7, PB_8, PB_9, PB_10, PB_11, PB_12, PB_13, PB_14, PB_15,
    PC_0, PC_1, PC_2, PC_3, PC_4, PC_5, PC_6, PC_7, PC_8, PC_9, PC_10, PC_11, PC_12, PC_13, PC_14, PC_15,
    PD_0, PD_1, PD_2, PD_3, PD_4, PD_5, PD_6, PD_7, PD_8, PD_9, PD_10, PD_11, PD_12, PD_13, PD_14, PD_15,
    PE_0, PE_1, PE_2, PE_3, PE_4, PE_5, PE_6, PE_7, PE_8, PE_9, PE_10, PE_11, PE_12, PE_13, PE_14, PE_15,
    PF_0, PF_1, PF_2, PF_3, PF_4, PF_5, PF_6, PF_7, PF_8, PF_9, PF_10, PF_11, PF_12, PF_13, PF_14, PF_15,
    PH_0, PH_1, PH_2, PH_3, PH_4, PH_5, PH_6, PH_7, PH_8, PH_9, PH_10, PH_11, PH_12, PH_13, PH_14, PH_15
} aos_stm32_pin;


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
