/*
 * aos_board.h
 *
 *  Created on: Jan 19, 2022
 *      Author: marc
 */

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif
//
// The AOS_BOARD_TYPE conditional defines for which board we are
// building the code. It should be defined in the project settings
// or makefile. By default, we build for the EVK_ES2 dev board.
//
// To add support for a new board called, say, MY_BOARD:
//  1. Create the bsp header file, name it "aos_boards/aos_board_MY_BOARD.h",
//     taking example on an existing board header file. Make sure to check on
//     the __AOS_BOARD_WAS_INCLUDED__ conditional to avoid users including it
//     directly from the application code.
//  2. Create a bsp directory, name it MY_BOARD
//  3. Place any board-specific source files in the MY_BOARD/ directory
//  4. Compile your application with -DABEEWAY_BOARD_TYPE=MY_BOARD
//  5. Create a library called libbsp-MY_BOARD.a with the files in MY_BOARD/
//  6. Link your application against the object library (-lbsp-MY_BOARD)
//
#if !defined(AOS_BOARD_TYPE)
#define AOS_BOARD_TYPE EVK_ES2
#endif

#define __AOS_BOARD_WAS_INCLUDED__ (1)

#define __gcc_header(x) #x
#define _gcc_header(x) __gcc_header(aos_board_##x.h)
#define gcc_header(x) _gcc_header(x)
#include gcc_header(AOS_BOARD_TYPE)

#undef __AOS_BOARD_WAS_INCLUDED__

/*!
 * \fn int aos_board_init(void)
 *
 * \brief Prepare a board for use.
 *
 * This function does the board-specific initializaton. This is a prototype
 * only, the actual function is implemented as part of board-specific code.
 *
 * This function is called from AOS once the scheduler has been started.
 *
 */
int aos_board_init(void);

/*!
 * \fn void aos_board_led_set(aos_board_led_idx_t led_idx, bool led_on)
 *
 * \brief Set the state of one of the board LEDs.
 *
 * \param [in] led_idx LED index, from 0 to aos_board_led_count()
 *
 * \param [in] led_on  LED state (false=LED off, true=LED on)
 *
 */
void aos_board_led_set(aos_board_led_idx_t led_idx, int led_on);

/*!
 * \fn void aos_board_led_toggle(aos_board_led_t led_idx)
 *
 * \brief Toggles one of the board LEDs.
 *
 * \param [in] led_idx LED index, from 0 to aos_board_led_count()
 *
 */
void aos_board_led_toggle(aos_board_led_idx_t led_idx);

/*!
 * \fn aos_board_led_count(void)
 *
 * \brief Returns the number of LEDs on the board.
 *
 */
unsigned aos_board_led_count(void);

#if defined(__cplusplus)
}
#endif
