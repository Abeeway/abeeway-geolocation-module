/*
 * strnhex.h - String to hex conversion function
 *
 * This source module provides a function to convert a C string
 * representing hexadecimal data of arbitrary length into a user
 * provided binary buffer.
 *
 * Copyright 2021 Marc Bongartz <mbong@free.fr>
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
 */

/*!
 * @brief strnhex - asciz string to hex buffer conversion
 *
 * @descr The strnhex function converts a C string representing a hexadecimal
 * data stream, eventually containing '-' or ':' characters separating bytes,
 * to its binary equivalent in a user provided buffer.
 *
 * @param hexbuf - address of the buffer where to write the binary data
 *
 * @param buflen - length of the hexbuf binary data buffer
 *
 * @param cp - input string, nul terminated. The input string can be in plain
 *             hex binary, MAC address, or EUI format, for example:
 *
 *    0badc0ffee      (plain hex)
 *    0b:ad:c0:ff:ee  (MAC address)
 *    0b-ad-c0-ff-ee  (EUI)
 *    0bad:c0-ffee    (mix and match)
 *
 * @return This function returns the number of bytes placed in the hexbuf
 *         upon successful completion. In case of a parsing error, -1 is
 *         returned.
 *
 */
int strnhex(uint8_t *hexbuf, uint8_t buflen, const char *cp);
