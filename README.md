
# Abeeway Type1WL Geolocation Module Evaluation Kit (EVK)

**VERSION: v1.2-1** (Latest) -> [back to master branch](https://github.com/Abeeway/abeeway-geolocation-module)

## Introduction

The Type1WL Evaluation Kit includes the following components:

- Type1WL Evaluation Board - version V2.4
- Abeeway Operating System Software Development Kit (AOS-SDK)
- The source codes of Example Applications

The following chapters explain how to start software development on the Type1WL module with the help of the EVK.

## Set up the Development Environment

1. Install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) on your computer
2. Clone this repository and checkout the `aos-sdk_v1.2-1` branch:  

   ```bash
   git clone https://github.com/Abeeway/abeeway-geolocation-module.git`
   git checkout aos-sdk_v1.2-1
   ```

3. Start the STM32CubeIDE and select the cloned repo as the workspace directory
4. Import the project into STM32CubeIDE
   - Click on the **_File > Import > General > Existing Project Into Workspace_** menu item
   - Select the following folder of the cloned repo: `abeeway-geolocation-module/apps/abeeway-demo`

## Compile the demo apps and install one of them on the Module

1. Open the `abeeway-demo` folder in the Project Explorer and select the `demo1` project
2. Compile the code by clicking on the **_Project > Build Project_** menu item.
3. Connect the Evaluation Board to your computer with an USB cable through USB_3 port.
4. Write the compiled firmware to the Abeeway Geolocation Module through the module's built-in ST-Link debugger by clicking on the **_Run > Run_** menu item. The compiled binary files are in `\abeeway-geolocation-module\apps\abeeway-demo\demo1\demo1-evk-debug` folder. You can also flash the binary file using `STM32CubeProgrammer` or the Abeeway Bootloader. Please follow the instructions in the [Abeeway Firmware Flashing Guide](https://github.com/Abeeway/abeeway-geolocation-module/tree/master/docs/Type1WL-EVB_first_flash.md).
5. After the download has been finished and the device restarted check if LED_3 is blinking.
6. Use a Serial Terminal tool (e.g.: minicom (on Linux), TeraTerm or Putty (on Windows), etc.) to connect your computer to the board and see the command line interface.
   - The serial port must be set to 57600 bauds, 8 data bits, no parity, one stop bit (8N1).
   - The login pin is either `123` or `456`, The former provides normal user access, the latter provides super-user access, making different sets of CLI commands available.
   - Use the `help` command to see the available command/parameter options. Commands may be abbreviated as long as they remain unambiguous.
7. Optionally, you may repeat the above steps with the `demo2` and `demo3` projects too. (Please note that LED_3 blinks only in case of the `demo1` app.)

## Read the detailed user guide and study the source code

1. Read the [AOS-SDK User Guide](https://github.com/Abeeway/abeeway-geolocation-module/tree/aos-sdk_v1.2-1/docs/AOS_user_guide.pdf)
2. Study the source code of the apps by having a look at the `README.md` and `src/main.c` files of the appropriate project.
   - [demo1/README.md](https://github.com/Abeeway/abeeway-geolocation-module/tree/aos-sdk_v1.2-1/apps/abeeway-demo/demo1/README.md),
   - [demo2/README.md](https://github.com/Abeeway/abeeway-geolocation-module/tree/aos-sdk_v1.2-1/apps/abeeway-demo/demo2/README.md),
   - [demo3/README.md](https://github.com/Abeeway/abeeway-geolocation-module/tree/aos-sdk_v1.2-1/apps/abeeway-demo/demo3/README.md)
3. Study the auto-generated API documentation of the aos-sdk in the [aos-sdk/docs/html](https://github.com/Abeeway/abeeway-geolocation-module/tree/aos-sdk_v1.2-1/aos-sdk/docs/html) folder. After you cloned this repository, you can open the [`index.html`](https://htmlpreview.github.io/?https://github.com/Abeeway/abeeway-geolocation-module/blob/aos-sdk_v1.2-1/aos-sdk/docs/html/index.html) file with your favorite web browser to start reading.
