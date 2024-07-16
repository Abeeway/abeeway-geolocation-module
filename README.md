
# Abeeway Geolocation Module example project

**VERSION: v0.5-0**

## Project components

This project is built from the following components:

- Abeeway Geolocation Module Evaluation Board
  - *Please find its user manual [here](https://docs.thingpark.com/thingpark-location/B-Feature-Topics/GeolocModuleEVKIntro_C/#documentation)*
- STM32CubeIDE development environment.
  - *Please download it from [here](https://www.st.com/en/development-tools/stm32cubeide.html)*
- Abeeway Operating System SDK (aos-sdk)
  - *Please check its source code [here](aos-sdk) and its documentation [here](https://htmlpreview.github.io/?https://github.com/Abeeway/abeeway-geolocation-module/blob/master/aos-sdk/documentation/html/index.html).*

## The demo application

The result of the project is a simple application that demonstrates the features of the Abeeway Geolocalization Module and its SDK. The application has 2 main components:

1. A command line interface that allows users configuring the Abeeway Geolocation Module through an USB port.
2. A simple demo application running as an RTOS task with the following features:
   - Blinking LED_4 of the board
   - Sending a LoRaWAN Uplink Message upon pressing Switch_4 of the board
   - Turning On/Off LED_3 of the board upon sending a LoRaWAN Downlink message:
     - To turn on, send a DL messaage with payload 01 to FPort: 2
     - To turn off, send a DL messaage with payload 02 to FPort: 2

## Installing the Development Environment

### Install the project files and set up the STM32CubeIDE project on your computer

1. Install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) on your computer
2. Clone this repository:  
   `git clone https://github.com/Abeeway/abeeway-geolocation-module`
3. Import the project into STM32CubeIDE
   - Select the **_File > Import > Existing Project Into Workspace_** menu item
   - Select the following folder of the cloned repo: `abeeway-geolocation-module/apps/community/app-community-demo` repo

### Compile the demo app and install it on the Module

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

   - Display your configuration and save it in the flash memory

      ```Shell
      settings display
      settings save
      ```

3. Test the LoRaWAN connectivity
   - Provision your Module on your LoRaWAN Network Server, (Please note that its `DevEUI` was shown by the `settings display` command)  
   - Open a new LoRaWAN connection on the module, join the network and check if the join was sucsessful. Execute the following CLI commands:

      ```Shell
      lora open
      lora join 
      lora status
      ```

   - Send a LoRaWAN test message (and chek in the logs of the network server if it has arrived there)

      ```Shell
      lora send aabbcc
      ```

## Working with the Demo Application

### Test the Demo application

1. Check if LED_3 is blinking with 1000ms prdiod
2. Press the button of Switch #4 of the Evaluation Board and check on your Network Server that every push results in a new LoRaWAN uplink message
3. Send a LoRaWAN Downlink message with Payload: "01" to FPort 2 of the Module and see if LED_4 turns on once the downlink arrives.
   - Please note that the downlink message can only be delivered as a response to an uplink message. The uplink message can be triggered by pressong the button of Switch #4.
4. Send a LoRaWAN Downlink message with Payload: "00" to FPort 2 of the Module and see if LED_4 turns off once the downlink arrives. (Press the button of Switch #4 to get the downlink)

### Modify the demo application

Please check the content of the `src/main.c` file of the source code and see how the simple demo application works. You can also check how the CLI commands are implemented in the `src/cli-cmd-*.c` files.
