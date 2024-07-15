/*!
 * @file board_drv_buzzer.h
 *
 * @brief Top level API for the buzzer driver.
 *
 * @author Hamza Saadi, March 20 2023
 *
 * Copyright (C) 2023, Abeeway (http://www.abeeway.com/)
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

#if defined(__cplusplus)
extern "C" {
#endif

#include "aos_common.h"

/*
 * Buzzer API definition. Separated from mt_lib.h as that header file was growing
 * too large, and to be able in the future to conditionally compile buzzer support
 * without sprinkling conditionals all across the mt_lib header file.
 */

typedef struct {
	enum {
		board_drv_buzz_done,			// Sequence done, do not reschedule the sequencer task
		board_drv_buzz_freq,			// Sound the buzzer at frequency <ms>
		board_drv_buzz_loop,			// Loop back to the start of the current sequence <ms>-1 times (0=forever)
		board_drv_buzz_wait,			// Reschedule sequencer task after <ms> of milliseconds
		board_drv_buzz_volume			// Set buzzer volume percentage (0% .. 100%)
	} action;					// The sequencer action to execute.
	uint16_t ms;				// Argument to the sequencer action, mostly a duration in ms.
} board_drv_buzz_sequencer_entry_t;

/*!
 * @brief board_drv_buzzer_set_frequency() - sound the buzzer at a specified frequency.
 *
 * This function outputs a tone via the buzzer, at the requested frequency.
 *
 * @param freq frequency value in Hertz
 *
 */
void board_drv_buzzer_set_frequency(uint32_t freq);

/*!
 * @brief board_drv_buzzer_set_volume() - set the buzzer volume (as a percentage).
 *
 * This function sets the buzzer volume (on hardware that permits it).
 *
 * It is not supported on differential driven buzzer hardware, nor on
 * active buzzers. Only the single drive buzzer allows modification of
 * the buzzer volume by varying the PWM duty cycle.
 *
 * @param percentage The desired volume as a percentage, (0=off to 100=max).
 *
 */
void board_drv_buzzer_set_volume(uint8_t pct);

/*!
 * @brief board_drv_buzzer_set_volume_limit() - set the buzzer volume limit (as a percentage).
 *
 * This function sets the maximum volume the buzzer may use, to avoid blowing the fuse
 * added by GHE-943 or simply to reduce the default buzzer volume if it is not controlled
 * by the application firmware..
 *
 * @param percentage The desired volume limit as a percentage, (0=no limit).
 *
 * @remark Note that setting the max volume will affect the interpretation of the
 * buzzer volume percentage, which becomes a percentage of the volume limit.
 *
 * Volume limit changes are applied immediately. This function is normally called at
 * board initialization time only.
 *
 */
void board_drv_buzzer_set_volume_limit(uint8_t pct);

/*!
 * @brief board_drv_buzzer_sequencer_stop() - stop a running buzzer sequencer task
 *
 * This function stops a running sequencer task, turning off the buzzer.
 *
 * @return aos_result_t indicating success or failure.
 *
 */
aos_result_t board_drv_buzzer_sequencer_stop(void);


/*!
 * @brief board_drv_buzzer_sequencer_start() - start a buzzer sequencer task
 *
 * This function initiates a sequencer task for the buzzer. An eventual
 * active sequencer task is stopped.
 *
 * @param seq the address of a board_drv_buzz_sequencer_entry_t array with the
 * sequencer instructions to execute.
 *
 * For example, to play a short tune repeated for a total of 3 times:
 *
 *	static const board_drv_buzz_sequencer_entry_t noise[] = {
 *		{ board_drv_buzz_freq, 7700 }, { board_drv_buzz_wait,  70 },
 *		{ board_drv_buzz_freq,    0 }, { board_drv_buzz_wait,  30 },
 *		{ board_drv_buzz_freq, 7800 }, { board_drv_buzz_wait, 100 },
 *		{ board_drv_buzz_freq,    0 }, { board_drv_buzz_wait,  30 },
 *		{ board_drv_buzz_freq, 7900 }, { board_drv_buzz_wait, 100 },
 *		{ board_drv_buzz_freq,    0 }, { board_drv_buzz_wait, 300 },
 *		{ board_drv_buzz_loop, 3 },
 *	};
 *
 * 	board_drv_buzzer_sequencer_start(noise);
 *
 * @return aos_result_t indicating success or failure.
 *
 */
aos_result_t board_drv_buzzer_sequencer_start(const board_drv_buzz_sequencer_entry_t *seq);

/*!
 * @brief board_drv_buzzer_sequencer_is_running() - test whether the buzzer is running.
 *
 * This is necessary to ensure we do not go below EM1 when the buzzer is running,
 * or to ensure a tone sequence runs to completion before starting another one.
 *
 * @return true if the buzzer is running (ie, the associated timer is active).
 *
 */
bool board_drv_buzzer_sequencer_is_running(void);

/*!
 * Buzzer event callback definitions. There are a number of events that may be
 * notified to the user, these are defined here.
 */
typedef enum {
	board_drv_buzzer_event_seq_starting,		// Buzzer sequencer is starting
	board_drv_buzzer_event_seq_stopped,		// Buzzer sequencer has stopped
} board_drv_buzzer_event_t;

typedef struct {
	board_drv_buzzer_event_t		event;
} board_drv_buzzer_event_info_t;

typedef void (*board_drv_buzzer_callback_t)(void *, board_drv_buzzer_event_info_t *info);

/*!
 * @brief board_drv_buzzer_init() - Initialize the buzzer driver
 *
 * This function registers a callback handler for buzzer events. This allows an
 * application to take preventive measures to, for example, ignore buzzer induced
 * motion events.
 *
 * @param  consumption Buzzer consumption in uA given for the max volume
 *
 * @param user_cb user callback event handling function
 *
 * @param user_arg user argument to pass to the user event handling function
 *
 * @return aos_result_success if the registration was successful.
 * @return aos_result_not_found if the selected buzzer ID  does not exist
 *
 * @remark To clear a user event callback, a NULL  function pointer can be passed.
 */
aos_result_t board_drv_buzzer_init(uint32_t consumption_ua, board_drv_buzzer_callback_t user_cb, void *user_arg);


/*!
 * @brief board_drv_buzzer_get_consumption_uah() - Return the buzzer consumption
 *
 * @return The buzzer consumption in uAh
 */
uint64_t board_drv_buzzer_get_consumption_uah(void);

/*!
 * @brief board_drv_buzzer_clear_consumption() - Clear the buzzer consumption
 *
 */
void board_drv_buzzer_clear_consumption(void);


#if defined(__cplusplus)
}
#endif

