# Abeeway BLE Geolocation 

## Project components

This project is built from the following components:

- Abeeway Geolocation Module Evaluation Board
  - *Please find its user manual [here](https://docs.thingpark.com/thingpark-location/B-Feature-Topics/GeolocModuleEVKIntro_C/#documentation)*
- STM32CubeIDE development environment.
  - *Please download it from [here](https://www.st.com/en/development-tools/stm32cubeide.html)*
- Abeeway Operating System SDK (aos-sdk)
  - *Please check its source code [here](aos-sdk) and its documentation [here](https://htmlpreview.github.io/?https://github.com/Abeeway/abeeway-geolocation-module/blob/master/aos-sdk/documentation/html/index.html).*

## The demo application

The result of the project is a simple application that demonstrates the features of the Abeeway Geolocalization Module and its SDK. The application has as main components : 

1. A commande line interface that allows users configuring the Abeeway Geolocation Module trough an USB port.
2. A simple demo application running as an RTOS task with the following features :
	- Start scan eddystone BLE beacons with a defined signature set by MAIN 1 and MAIN2 parameters at a periodic and programmable time.
	- Sending a LoRaWAN Uplink Message containing the information collected (MAC address + RSSI)
	
	
## Installing the Development Environment

### Install the project files and set up the STM32CubeIDE project on your computer

1. Compile the code by clicking on the 'Hammer' icon on the top of the STM32CubeIDE screen
2. Connect the Evaluation Board to your computer with an USB cable through USB_3 port. 
3. Download the compiled firmware to the Abeeway Geolocation Module through the built-in ST-Link debugger
   - Select the **_Run > Run_** menu item in STM32CubeIDE
4. Check that after the download was done, the device restarted and LED_3 is Blinking

### Configure the Module for LoRaWAN connectivity

1. Use a Serial Terminal tool (e.g.: Teraterm, Putty, etc.) to connect your computer to the board and see the command line interface
   - The serial port must be set to 57600 bauds, 8 data bits, no parity, one stop bit (8N1).
   - The login pin is either `123` or `456`, The former provides normal user access, the latter provides super-user access, making different sets of CLI commands available.
   - Use the `help` command to see the available command/parameter options. Commands may be abbreviated as long as they remain unambiguous.
2. Set the LoRaWAN connectivity parameters of the Module executing the following CLI commands: 
   - Erase the current config and set factory default configuration

      ```Shell
      settings erase
      settings factory
      ```
      
   - Set your LoRaWAN region. (You can see the possible region options by the the `settings set region help` command)

      ```Shell
      settings set region EU868
      ```

   - Set the your selected activation type (`otaa` or `abp`)

      ```Shell
      settings set activation otaa
      ```

   - Set the your `JoinEUI` and `AppKey`
     Feel free to change appkey/nwkkey. The AppKey and NwkKey must be the same!

      ```Shell
      settings set joineui 20635f000a000001
      settings set appkey 00000000000000000000000000000000
      settings set nwkkey 00000000000000000000000000000000
      ```
   - Set the your `Deveui`
     Feel free to change deveui.
     
     ```Shell
     settings set deveui 0000000000000000
     ```
   - Display your configuration and save it in the flash memory

      ```Shell
      settings display
      settings save
      ```
## Working with the Demo Application

### Test the Demo application

Check if the LED4 is bliking with 1000ms period

#### How to use CLI tool to set BLE Filter
1. Use a Serial Terminal tool (e.g.: Teraterm, Putty, etc.) to connect your computer to the board and see the command line interface
   - The serial port must be set to 57600 bauds, 8 data bits, no parity, one stop bit (8N1).
   - The login pin is either `123` or `456`, The former provides normal user access, the latter provides super-user access, making different sets of CLI commands available.
   - Use the `help` command to see the available command/parameter options. Commands may be abbreviated as long as they remain unambiguous.

2. Set the BLE parameters of the Module executing the following CLI commands:

    - setup the scan repeat delay at 30s

        ```Shell
        geolocBle  -rep 30
        ```
    - setup filter *ABEEWAY1* 

        ```Shell
        geolocBle -filter 4142454557455931
        ```

#### How to use the Downlink message to set BLE Filter

It is also possible to set the BLE filter by a dowlink message.
1. Send a LoRaWAN dowlink message with payload : `"0B026900000028"` or `"0B02690000003C"`  to change the `repeat-delay` of the BLE scan `(00000028 = 40s)`, `(0000003C = 60s)`. The default delay is 30s.
2. Send a LoRaWAN dowlink message with payload : `"0B024E41424545"` and `"0B024F57415931"` to change the `Filter`

    |      ID     |    Description   |
    | ----------- | ---------------- |
    |  0x0B       | Update parameter |
    |  0x02       | Ack Token        |
    |  0x4E       | Filter Main 1 ID |
    |  0x4F       | Filter Main 2 ID |
    |  0x41424545 | A B E E          |
    |  0x57415931 | W A Y 1          |

  - Please note that the downlink message can only be delivered as a response to an uplink message.

#### Launch the demo application

##### In CLI

Type in de CLI : 

1. to start BLE eddystonne scan 

    ```Shell
     geolocBle  -start
     ```
2. to stop the BLE scan

    ```Shell
     geolocBle  -stop
     ```
##### With Board button

1. Press the button `Board Swith 04` to start the BLE eddystonne scan. After the scan done, a payload that contains informations (MAC adress & RSSI) will be send via LoRa. Then check 
on your Network Server that every push results in a new LoRaWAN uplink message
2. Press the button `Board switch 05` to stop BLE scan.

### Modify the demo application

Please check the content of the `abeeway-geolocation-module/apps/community/app-cmty_ble-geoloc/src/` 
   - `*/main.c*` : 
      startup application, initialization of the system, the CLI services initialization
        
   - `*/app_scan_report.c*` :
      This file contains
  	    - the function that is execute when downlink message is received;
 		- the function on press on button 4, that start BLE scan and print the result of scan;
  		- and the function application task that start the application.

   - `*/ble_scan_handler.c*` :
        - scan and print the result of BLE scan function
 	    - and the callback function when BLE scan is done

   - `*/btn_handling.c*` :
        - the button configuration function,
        - the functions to open BLE and close it

   - `*/encode_handling.c*` :
        - somes encoding and swaping functions

   - `*/lora_handler.c*` :
        - setup a loramac datarate function;
        - and a sending of a payload containing the information collected (MAC address + RSSI), function.

 files of the source code and see how the demo application works. You can also check how the CLI commands are implemented in the `src/cli-cmd-*.c` files.