/*/*
 * @file aos_otp.h - One-Time Programmable memory interface.
 *
 * This file implements the API for reading and writing the OTP memory.
 *
 * @author Marc Bongartz (marc.bongartz@abeeway.com)
 *
 * Copyright (C) 2023, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */
#pragma once

typedef enum {
    aos_otp_type_raw_data,      // 6 bytes, user defined data
    aos_otp_type_hse_trim,      // HSE trim value (6 bits)
    aos_otp_type_mac_address_1,	// A MAC address (ie, for storing the BLE address)
	aos_otp_type_mac_address_2,	// Another MAC address
}  aos_otp_type_t;

typedef union {
    uint8_t raw_data[6];			// User defined data (6 bytes)
    uint8_t hse_trim:6;				// HSE trim value (6 bits)
    uint8_t mac_address[6];			// MAC address (6 bytes)
} __attribute__((packed)) aos_otp_data_t;

/*!
 * \fn int aos_otp_read(const aos_otp_type_t type, aos_otp_data_t *data);
 *
 * \brief Read an aos_otp_data_t value from OTP memory.
 *
 * \param type - enumerated value indicating the value type
 * \param data - pointer to an aos_otp_data_t storage location.
 *
 * \return 0 on success, negative value on error (ie, no such value stored).
 *
 */
int aos_otp_read(const aos_otp_type_t type, aos_otp_data_t *data);

/*!
 * \fn int aos_otp_write(const aos_otp_type_t type, const aos_otp_data_t *data);
 *
 * \brief Write an aos_otp_data_t value to OTP memory.
 *
 * \param type - enumerated value indicating the value type
 * \param data - pointer to an aos_otp_data_t storage location.
 *
 * \return 0 on success, negative value on error (ie, OTP storage exhausted).
 *
 */
int aos_otp_write(const aos_otp_type_t type, const aos_otp_data_t *data);

/*!
 * \fn int int aos_otp_free_entries(void);
 *
 * \brief Check the number of free OTP entries.
 *
 * \return the number of available OTP entries.
 *
 */
int aos_otp_free_entries(void);

