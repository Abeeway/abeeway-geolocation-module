/*!
 * \file main.c
 *
 * \brief startup of application
 *
 *  * \copyright (C) 2022, Abeeway (www.abeeway.com). All Rights Reserved.
 */

#include "FreeRTOS.h"
#include "task.h"

#include "aos_board.h"
#include "aos_system.h"

// General definitions
#define APP_MAIN_LED_PERIOD			1000	//!< Main LED blink period in ms

// Application Thread
#define APP_TASK_STACK_SIZE 		(2048/sizeof(StackType_t))
#define APP_TASK_PRIORITY 			AOS_THREAD_PRIO_APPLICATION
static TaskHandle_t _app_task;
static StaticTask_t _app_task_info;
static StackType_t _app_task_stack[APP_TASK_STACK_SIZE];

/*!
  * \brief  Application main thread
  * \param argument User argument
  */
static void _application_task(void *argument)
{
	for ( ; ; ) {
		// Toggle the LED state
		aos_board_led_toggle(aos_board_led_idx_led4);
		vTaskDelay(pdMS_TO_TICKS(APP_MAIN_LED_PERIOD));
	}
	vTaskDelete(NULL);
}

/*!
  * \brief  Application entry point.
  * \return unused returned value
  */
int main(void)
{
	// Initialize the system
	aos_system_init();

	// Create the system task. Since it is statically allocated. it should not fail
	_app_task = xTaskCreateStatic(_application_task, "Application", APP_TASK_STACK_SIZE, NULL,
			APP_TASK_PRIORITY, _app_task_stack, &_app_task_info);

	// Start scheduler
    vTaskStartScheduler();
}
