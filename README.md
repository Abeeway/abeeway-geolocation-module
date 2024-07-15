
# Abeeway Geolocation Module

## Project components

This project is utilizing the following components:

- Abeeway Geolocation Module Evaluation Board
  - *Please find its user manual in the `docs` folder*
- STM32CubeIDE development environment
  - *Please download it from [here](https://www.st.com/en/development-tools/stm32cubeide.html)*
- Abeeway Operating System SDK (aos-sdk)
  - Please find its compiled binary files in the `aos-sdk` folder
- Example Applications
  - Please find example source codes in the `apps` folder.

## Installing the Development Environment

### Collect the project files and set up the STM32CubeIDE project on your computer

1. Install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) on your computer
2. Clone this repository:  
   `git clone https://github.com/Abeeway/abeeway-geolocation-module`
3. Start STM32CubeIDE and select the cloned repo as the workspace directory
4. Import the project into STM32CubeIDE
   - Click on the **_File > Import > Existing Project Into Workspace_** menu item
   - Select the following folder of the cloned repo: `abeeway-geolocation-module/apps/community/abeeway-demo`

### Compile the `demo1`/`demo2`/`demo3` apps and install one of them the Module

1. Open the `abeeway-demo` folder in the Project Explorer and select the `demo1` project
2. Compile the code by clicking on the **_Project > Build Project_** menu item.
3. Connect the Evaluation Board to your computer with an USB cable through USB_3 port.
4. Write the compiled firmware to the Abeeway Geolocation Module through the module's built-in ST-Link debugger by clicking on the **_Run > Run_** menu item.
5. Check that after the download has been finished, the device restarted and LED_3 is Blinking.
6. Use a Serial Terminal tool (e.g.: minicom (on Linux), Teraterm or Putty (on Windows), etc.) to connect your computer to the board and see the command line interface
   - The serial port must be set to 57600 bauds, 8 data bits, no parity, one stop bit (8N1).
   - The login pin is either `123` or `456`, The former provides normal user access, the latter provides super-user access, making different sets of CLI commands available.
   - Use the `help` command to see the available command/parameter options. Commands may be abbreviated as long as they remain unambiguous.
7. Study the source code of the app by having a look at the `README.md` and `src/main.c` files of the `demo1` project.
8. Optionally, you may repeat the above steps with the `demo2` and `demo3` projects too. (Please note that LED_3 blinks only in case of the `demo1` app.)