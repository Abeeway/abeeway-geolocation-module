# demo3

This example demonstrates the use of the geolocation

## Demonstration purpose
This example demonstrates the following:
- Use of the CLI service
- Use of the configuration service
- Use of the WIFI, BLE scan and GNSS services
- Use of the basic geolocation engine service
- Use of the LoRa service
- Use of the LED driver
- Use of the buzzer driver
- Use of the LIS2DW12 accelerometer
- Main application task using a freeRTOS xQueue to handle application events.
- System log usage over the CLI

## Demonstration overview
The accelerometer is opened at the start time. Once its start sequence is complete, shake the EVK.
This will wakeup the accelerometer and send the motion_start application event.
On the motion start event, the geolocation basic engine is started. It will schedule the 3 technologies GNSS, WIFI and BLE
in this order.
On the geolocation completion:
- A lora message is sent to the network and a led will blink.
- The LED blinks on success


## Geolocation technology scheduling
- The first geolocation technology (GNSS) is always done
- The second geolocation technology (WIFI) is skipped if_the first one is successful.
- The third geolocation technology (BLE) is always done

The list of the geolocation technologies to be used are configurable by the parameter 0:
This is a byte array of 5 entries. Each entry (on 8 bits) contains:
- bit 7..6: geolocation action
- bit 5..0: geolocation type
The last entry should be null (to find the number of configured techno).

The number of BLE beacons is also configurable via the parameter 1.

Notes
- The GNSS may take a long time to do the first fix, since it we'll be in cold start.
- The BLE scan used is the BLE-SCAN-1
- Once the geolocation is done, the application switches the technology action of the WIFI (from skip on success to always-done)


## CLI commands
To login use the password **123**.

The main commands are:
config    <cmd> Configuration commands
geoloc    <cmd> Geolocation commands
help      <cmd> Display help information
logout    Disconnect the CLI
lora      <cmd> LoRaWAN commands
?         Display all helps
system    <cmd> System commands

To display the help of a main command, enter **help command**. Example
user> help config
config    <cmd> Configuration commands
    info      Display information in flash
    show      Show parameters
    read      <id> Read a parameter
    write     <id> <value> Write a parameter
    erase     Flash the factory default
    save      Save the configuration in flash

Once the geolocation ended, you may want to display the result. This can be done using the command **geoloc show**
