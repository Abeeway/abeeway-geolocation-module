
# Abeeway Geolocation Module example project

A sample application based on the Abeeway module SDK (AOS).

This folder contains a sample project for the Abeeway module that can be imported in the STM32CubeIDE development environment. It is intended to be used as an example on using the AOS SDK and a number of AOS services.

## Installing the project files and setting up the STM32CubeIDE project.

Prerequisites are a working installation of the STM32CubeIDE development environment. This Eclipse based development environment can be obtained free of charge from [ST's web site](https://www.st.com).

To install the demo project, follow the four simple steps below:

1. Clone this repository:  
   `git clone https://github.com/Abeeway/abeeway-geolocation-module`

2. Import the project into STM32CubeIDE
3. Select the **_File > Import > Existing Project Into Workspace_** menu item
4. Select the folder of the cloned `abeeway-geolocation-module` repo

From here you can modify, build, flash and debug the project, all from the STM32CubeIDE.

## Using the demo

The demo application is a low power FreeRTOS application, based on the AOS SDK.

It is controlled through a command line interface, accessible through the USB-serial controller available on the integrated ST-Link on the Abeeway Evaluation Kit.

The serial port must be set to 57600 bauds, 8 data bits, no parity, one stop bit (8N1).

Log in with the pin code '123' or '456'. The former provides normal user access, the latter provides super-user access, making different sets of CLI commands available.

The `help` command is available throughout, but typing any invalid command or omitting a command option will usually show the available commands or options.

```
super> help
ble       <cmd> BLE commands
gnss      <cmd> GNSS commands (MT3333)
gpio      <cmd> GPIO related commands
help      <cmd> Display help information
logout    Disconnect the CLI
lora      <cmd> LoRa related commands
pwm       <cmd> PWM related commands
?         Display all helps
settings  <cmd> LoRa settings related commands
system    <cmd> System commands
uart      <cmd> UART commands
wifi      <cmd> WIFI related commands
super>
```

Commands may be abbreviated as long as they remain unambiguous.

```
super> wi
Missing options for wifi
Options are:
	scan      : Perform a WIFI scan
	info      : Display WIFI information
	results   : Show WIFI scan results
	parameters: parameter related commands
ERROR
super> wi sc
Scan start
Number of results: 1
 MAC Address: 0x00 0x0c 0x43 0x26 0x60 0x60, RSSI: -56
OK
super>
```

Feel free to experiment with the different commands, and refer to the source code to understand how to perform specific operations.
