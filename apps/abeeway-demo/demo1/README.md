# demo1

This very simple example demonstrates the use of the AOS facilities on the EVK.

## Demonstration purpose
This example demonstrates the following:
- Basic SDK initialization
- Main application task using a freeRTOS xQueue to handle application events.
- Basic digital output GPIO (LED2)
- FreeRTOS timer creation and use by the LED2
- Digital input GPIO (SW3) using interruption serviced under the system thread
- CLI command creation
- System log usage over the CLI


## Demonstration overview
Once started, the LED 2 blinks at a frequency of 1Hz.
The CLI is available over the LPUART (which is rerouted to the debug USB port).
To connect to the CLI, you need a tool, which is able to provide a terminal:
- On window you can use TeraTerm. Open the connection window and select the appropriate USB port.
- On Linux you can use: minicom -D /dev/ttyACM0
The connection should be established at 56700 bauds, no parity, 1 stop and 8 bits data.
Once connected, you should be prompted to login. The default password is 123.

A press on the button SW3 triggers the log message "SW3 pressed"

## Demonstration details
Each time the timer expires, a message is sent to the application thread. On reception of the message,
the application thread toggles the LED.
While the LED toggling could occur under the timer callback (FreeRTOS timer thread), the example shows
how to defer a processing from a given thread to the application one.

If you want the CLI over the actual USB port of the module, you just need to replace **aos_uart_type_lpuart1**
by **aos_uart_type_usb** in the **srv_cli_open** function.
In this case, the second USB port should be connected. Just note that when the actual USB port is connected
the module does not run in low power mode. It will move in low power when the cable will be disconnected.

Explore the system CLI commands:
- To display the help, enter **help system" or **system**
- To display the help of all commands and sub commands enter, **?**
- To display the system information, enter **system info**
- To display the threads,  **system threads**
Note that abbreviated command names also work. Example **sys i** will display the system information.

The Error trigger command is accessible only in super user mode. To log with this access right you must first
logout the CLI using the **logout** command then enter the passowrd 456.


