/*!
 * @file board_drv_led.h
 *
 * @brief Top level API for the led driver.
 * The led driver is based on a sequencer that is able to play LED patterns
 * The driver supports up to 4 LEDs
 * There is one sequencer per LED. So different patterns can be played at the same time on several LEDs.
 * However a same sequencer cannot drive multiple LEDs.
 *
 * @note THE LED SHOULD LIGHT WHEN A LOGICAL 0 IS SET ON THE GPIO
 */
#pragma once

typedef enum {
	board_drv_led_state_off = 0, // Turn the LED off
	board_drv_led_state_on       // Turn the LED on
} board_drv_led_state_t;

typedef struct {
	enum {
		board_drv_seq_done,      // Sequence done, do not reschedule the sequencer task
		board_drv_seq_led_off,   // Turn the LED off for <ms> milliseconds
		board_drv_seq_led_on,    // Turn the LED on for <ms> milliseconds
		board_drv_seq_loop,      // Loop back to the start of the current sequence <ms>-1 times (0=forever)
		board_drv_seq_wait       // Reschedule sequencer task after <ms> of milliseconds
	} action;                    // The sequencer action to execute.
	uint16_t ms;                 // Argument to the sequencer action, mostly a duration in ms.
} board_drv_led_sequencer_entry_t;

/*!
 * LED event callback definitions. There are a number of events that may be
 * notified to the user, these are defined here.
 */
typedef enum {
	board_drv_led_event_seq_starting,		// LED sequencer is starting
	board_drv_led_event_seq_stopped,		// LED sequencer has stopped
} board_drv_led_event_t;

typedef struct {
	board_drv_led_event_t event;
	aos_gpio_id_t         led_id;
} board_drv_led_event_info_t;

typedef void (*board_drv_led_callback_t)(void *, board_drv_led_event_info_t *info);


/*!
 * @brief aos_board_led_config_t - Button configuration information.
 */
typedef struct {
	aos_gpio_id_t            gpio;                  //!< The GPIO to be used.
	board_drv_led_callback_t cbfn;                  //!< User callback function
	void                     *cbarg;                //!< User callback argument
	uint32_t				consumption_ua;			//!< LED consumption in micro-amps
} aos_board_led_config_t;

/*!
 * @brief board_drv_led_set() - turn a board led on or off.
 *
 * This function turns on or off one of the available LEDs.
 *
 * @param led aos_gpio_id_t GPIO of the LED to turn on or off
 *
 * @param state board_drv_led_state_t state to set the LED to.
 *
 * @return aos_result_t indicating success or failure (ie, no such led)
 */
aos_result_t board_drv_led_set(aos_gpio_id_t id, board_drv_led_state_t on);

/*!
 * @brief board_drv_led_sequencer_stop() - stop a running LED sequencer task
 *
 * This function stops a running sequencer task, leaving the LED as is.
 *
 * @param led aos_gpio_id_t GPIO of the LED to turn it's sequence off
 *
 * @return aos_result_t indicating success or failure (ie, no such led)
 *
 */
aos_result_t board_drv_led_sequencer_stop(aos_gpio_id_t id);

/*!
 * @brief board_drv_led_sequencer_start() - start a sequencer task for a LED.
 *
 * This function initiates a sequencer task for a given LED. An eventual
 * active sequencer task is stopped.
 *
 * @param id aos_gpio_id_t GPIO of the LED to play the sequence
 * @param seq the address of a board_drv_led_sequencer_entry_t array with the
 * sequencer instructions to execute.
 *
 * For example, for a short double flash every two seconds, repeated
 * for a total of 10 times:
 *
 * 	static const board_drv_led_sequencer_entry_t blink_seq[] = {
 * 		{ board_drv_seq_led_on, 20 }, { board_drv_seq_led_off, 130 },
 * 		{ board_drv_seq_led_on, 20 }, { board_drv_seq_led_off, 130 },
 * 		{ board_drv_seq_wait, 1700 }, { board_drv_seq_loop, 10 },
 * 	};
 *
 * 	board_drv_led_sequencer_start(aos_gpio_id_2, blink_seq);
 *
 * @return aos_result_t indicating success or failure (ie, no such led)
 *
 */
aos_result_t board_drv_led_sequencer_start(aos_gpio_id_t id, const board_drv_led_sequencer_entry_t *seq);

/*!
 * @brief board_led_create() - Enable and register a LED event callback handler.
 *
 * This function Enable and registers a callback handler for led events.
 * You should remove the led (if it's already created) before changing it's callback function or argument,
 *
 * @param  cfg config for a specific led, a NULL  function pointer can be passed to cbfn and cbarg.
 *
 * @return board_drv_status_success if the registration was successful or the led already created.
 * @return aos_result_not_supported if the selected led could not be registered.
 *
 */
aos_result_t board_led_create(aos_board_led_config_t* cfg);

/*!
 * @brief board_led_remove() - Remove a led
 *
 * This function remove and uninitialize an already created led
 *
 * @param id led gpio id to be removed
 */
void board_led_remove(aos_gpio_id_t id);

/*!
 * @brief board_drv_led_get_consumption_uah() - Retrieve the total consumption of all LEDs.
 *
 *
 * @return The total consumption of all LEDs.
 * @note The configuration (during the creation) should contain the LED consumption when on.
 */
uint64_t board_drv_led_get_consumption_uah(void);

/*!
 * @brief board_drv_led_clear_consumption() - Clear the consumption for all LEDs
 *
 *
 * @return The total consumption of all LEDs.
 * @note The configuration (during the creation) should contain the current of the LED when on.
 */
void board_drv_led_clear_consumption(void);

