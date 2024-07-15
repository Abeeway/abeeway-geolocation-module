# demo2

This example demonstrates the use of the board drivers and the bootloader.


## Environment instruction
The IDE of choice is STM32CubeIDE, available freely from https://www.st.com/.
The current supported version is  1.15.1.

The IDE provides a build system as well as an integrated debugger.

To create your working environment, follow the steps:
1. Get the official SDK version tarball and extract it. This create a directory **aos-sdk_vx.y-z**,which contains:
- A manuals directory containing the documentation
- The AOS-SDK-public directory containing the SDK and include files.
2. Create a working directory. Example: **abw-module-user**.
3. Under the working directory, copy the **AOS-SDK-public** directory.
4. Under the working directory,  Clone the **abw-app-demo** or extract the demo tarball.
5. Start the IDE and create a new workspace pointing to **abw-module-user**.
6. Finally, from the IDE import the project **abw-app-demo**
Now, the environment is ready.

Note that the **manuals/html** contains the API description.
To read it it, double click on the file index.html. This will open your favorite WEB browser with the documentation.

## Demonstration purpose
This example demonstrates the following:
- Basic SDK initialization
- Use of the button driver
- Use of the LED driver
- Use of the buzzer driver
- Use of the LIS2DW12 accelerometer
- Main application task using a freeRTOS xQueue to handle application events.
- System log usage over the CLI
- Bootloader entrance

## Demonstration overview
The accelerometer is opened at the start time. Once its start sequence is complete, shake the EVK.
This will wakeup the accelerometer and send the motion_start application event.
Then wait for the motion duration timeout (60 seconds). Once the timer elapses, the motion-stop application event is sent.
Note that the accelero.c file performs the link between the accelerometer board driver and the application.

This example maps the SW3 button to the button driver. The driver detects button presses, single clicks, double clicks
and so on. The button.c file catches the notifications from the board driver and sends appropriate application events.

The LED driver is able to plays patterns. The associated file led.c performs the link between the driver and the application.

The buzzer driver is able to plays simple beeps or a melody. The associated file buzzer.c performs the link between the driver
and the application. When the buzzer starts playing the buzzer_on application event is sent. Respectively, the buzzer_off
application is sent when the buzzer stops playing.

All application events are caught by the application event manager (events.c). Events are processed as follow:
- SW3 simple click: Buzzer plays a beep
- SW3 double clicks: Buzzer plays 2 beeps.
- SW3 press: Buzzer plays a melody and displays a LED pattern
- On buzzer_on event: The accelerometer is stopped.
- On buzzer_off event: The accelerometer is restarted.

## Bootloader
We remind that the bootloader is accessible only via the USB port (USB2). If the EVK is already connected
to the debug port (USB3), then an extra USB cable should be connected between USB2 and the PC.
To enter the bootloader, enter the CLI command **system bootloader**.



## CLI commands
To login use the password **123**.

The main commands are:
help      <cmd> Display help information
logout    Disconnect the CLI
?         Display all helps
system    <cmd> System commands

To display the help of the system command, enter **help system**. Example
user> help system
system    <cmd> System commands
    info      System information
    log       <cmd> Manage the logs
    reset     System reset
    bootloader Restart on bootloader mode
    thread [name] Thread information
    error     <cmd> Error information
    accelero  <cmd> Accelerometer information



