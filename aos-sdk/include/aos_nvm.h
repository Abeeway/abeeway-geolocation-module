/*!
 *
 * \file    aos_nvm.h
 *
 * \brief   Non-volatile memory interface API.
 *
 * \details This module provides an interface to the non-volatile memory provided
 *          by AOS for application use.
 *
 * \note    Write operations should be limited to the strict minimum to avoid
 *          deteriorating the underlying storage medium, notably FLASH memory
 *          which is only guaranteed for 10000 erase/write cycles.
 *
 *  Created on: May 30, 2022
 *      Author: marc
 *
 * Copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
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

/*!
 *
 * \fn      aos_nvm_size(void);
 *
 * \brief   Returns size of the user non-volatile memory, in words.
 *
 * \retval  The returned value is the number of words that can be stored
 *          in the user non-volatile memory.
 *
 */
unsigned aos_nvm_size(void);

/*!
 *
 * \fn      aos_result_t aos_nvm_read(unsigned word_address, uint32_t *ret_word);
 *
 * \brief   Reads a word (32-bit) value from the user non-volatile memory.
 *
 * \param   [IN] address   - zero-based address of the word in user nvm.
 *
 * \param   [OUT] ret_word - pointer to an uint32_t variable to receive the value.
 *
 * \retval  aos_result_t Status of the operation. If the parameters are not
 *          valid, the function returns aos_result_param_error. Upon success,
 *          it returns aos_result_success.
 */
aos_result_t aos_nvm_read(unsigned address, uint32_t *ret_word);

/*!
 *
 * \fn      aos_result_t aos_nvm_write(unsigned word_address, uint32_t word);
 *
 * \brief   Writes a word (32-bit) value to the user non-volatile memory.
 *
 * \param   [IN] address   - zero-based address of the word in user nvm;
 *
 * \param   [IN] word      - an uint32_t value to be written to nvm.
 *
 * \retval  aos_result_t Status of the operation. If the parameters are not
 *          valid, the function returns aos_result_param_error. Upon success,
 *          it returns aos_result_success.
 */
aos_result_t aos_nvm_write(unsigned word_address, uint32_t word);

