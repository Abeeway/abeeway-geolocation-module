/*!
 *
 * \file    aos_flash.h
 *
 * \brief   Flash memory interface API.
 *
 * \details This module provides an interface to flash memory, essentially
 *          intended for DFU / FUOTA purposes.
 *
 * \note    Write operations should be limited to the strict minimum to avoid
 *          deteriorating the FLASH memory, which is only guaranteed for 10000
 *          erase/write cycles.
 *
 * \note    For application data storage, prefer the aos_nvm API.
 *
 *  Created on: Jan 2, 2023
 *      Author: marc
 *
 * Copyright (C) 2023, Abeeway (www.abeeway.com). All Rights Reserved.
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimers. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimers in the documentation and/or
 * other materials provided with the distribution.
 *
 * In no event shall the authors or copyright holders be liable for any special,
 * incidental, indirect or consequential damages of any kind, or any damages
 * whatsoever resulting from loss of use, data or profits, whether or not
 * advised of the possibility of damage, and on any theory of liability,
 * arising out of or in connection with the use or performance of this software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS WITH THE SOFTWARE
 *
 */
#pragma once

#include "aos_common.h"	// aos_result_t definitions

///
/// @brief Determine the number of valid flash pages.
/// @return The number of flash pages that exist on this MCU.
///
unsigned aos_flash_page_count();

///
/// @brief Determine the flash page size.
/// @return The size of a flash page in bytes.
///
unsigned aos_flash_page_size();

///
/// @brief Determine the flash memory address of a flash page.
/// @param page A valid flash page number for which to determine
///             the memory address.
/// @return A valid flash memory address, or 0 if an invalid page
///         number was passed.
///
uint32_t aos_flash_page_address(uint32_t page);

///
/// @brief Determine the flash page number for a flash memory address.
/// @param address A valid flash memory address for which to determine
///                the page number.
/// @return The flash page number containing the address,
///         or the value 0xffffffff if an invalid address was passed. 
///
uint32_t aos_flash_page_number(uint32_t address);


///
/// @brief Determine the offset in a flash page of a given flash memory address.
/// @param address A valid flash memory address for which to determine
///                the page offset.
/// @return The offset within the flash page, 
///         or the value 0xffffffff if an invalid address was passed. 
///
uint32_t aos_flash_page_offset(uint32_t address);

///
/// @brief Erase one or more flash pages.
/// @param page The starting page number to erase.
/// @param npages The number of pages to erase.
/// @return aos_result_t indicating success or failure.
///
aos_result_t aos_flash_erase_page(uint32_t page, uint16_t npages);

///
/// @brief Write a double word (64 bits) to a flash memory address.
/// @param address A 64-bit aligned flash memory address
/// @param data The 64-bit value to write
/// @return aos_result_t indicating success or failure.
///
aos_result_t aos_flash_write_dword(uint32_t address, uint64_t data);

///
/// @brief Fill a flash page with data from a memory area. This function
/// is built upon aos_flash_write_dword(), and can be used to copy a RAM
/// or FLASH memory area to a FLASH page.
/// @param dest_page destination flash page number
/// @param src_addr The source address from where to copy the data.
/// @return aos_result_t indicating success or failure.
/// @note The source address must be 32-bit aligned, as the data is read
///       in 64-bit chunks. The destination page must have been previously
///       erased.
///
aos_result_t aos_flash_write_page(unsigned dest_page, uint32_t src_addr);
