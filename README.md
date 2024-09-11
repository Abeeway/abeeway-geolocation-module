
# Abeeway Geolocation Module

## Introduction

The `Type1WL Evaluation Board - version V2.4` is the Evaluation Board (EVB) of the
[`Type1WL`](https://www.abeeway.com/wp-content/uploads/2022/04/Abeeway_Geolocation-module-data-sheet_2022-v05.pdf)
ultra-low power geolocation module co-developed by
[Murata](https://www.murata.com) and [Abeeway](https://www.abeeway.com/).

The EVB is a complete development kit allowing developers to have full access to all functionalities of the module and select

- various power management schemes,
- digital interfaces
- and features like Buzzer, LEDs, Buttons, Flash memory and Sensors.

3 micro-USB connectors are available to interface with digital lines (UART, LPUART) and the built-in
[ST-link V2](https://www.st.com/en/development-tools/st-link-v2.html).

For more details please refer to the following documents:

- [Type1WL EVB User Manual](https://github.com/Abeeway/abeeway-geolocation-module/tree/master/docs/Type1WL-EVB_user_manual.pdf)
- [Type1WL Schematics](https://github.com/Abeeway/abeeway-geolocation-module/tree/master/docs/Type1WL-EVB_v2_4_20221014-1_abw.pdf)


## Firmware Update

The Type1WL Evaluation Board is delivered with a pre-installed Manufacturing Firmware (MFG) that enables various features of the geoloc module, configures the LoRaWAN credentials, updates 3rd party middleware for the MT3333 and LR11xx and enables some radio features for testing and certification.

Before starting any development on the Abeeway EVB, please connect to the Command Line Interface of the MFG firmware through its USB2 port using a terminal emulator software like `TeraTerm` (on Windows) or `minicom` (on Linux) and verify if all components have the latest firmware.

- Power the board through the USB3 port.
- Connect your computer through the USB2 port.
- Open your terminal software and set up a connection with 57600 bouds, 8 data bits, no parity, one stop bit [8N1]
- Once you see the login prompt, type the PIN: `456` to log in as a Super User. 

  - *Note: If you typed PIN: `123` you would login as normal user.*
  - *Note: After you logged in type the `help` command to see the supported command you may execute.*

- Verify the firmware versions in your board with the following commands:

  - MFG Application version is printed with command: `system version`

    ```bash
    super> system version
    AOS: 1.0-0. Built on: Jun 18 2024, 15:17:35
    MFG: 3.0-192. Build on: Jun 18 2024, 15:18:50
    OK
    ````

  - BLE and FUS versions are printed with command: `ble version`

    ```bash
    super> ble version
    Wireless Firmware version (BLE STACK) 1.15.0
    FUS version 1.2.0
    OK
    super> 
    ```

  - LR11xx version is printed with command: `lr11xx firmware version`
  
    ```bash
    super> lr11xx firmware version
                  System Type :          1 (transceiver)
             Hardware version :       0x22
             Firmware version :     0x0308
    OK
    super> 
    ```
  
  - MT3333 version is printed with command: `gnss mt3333 version`

    ```bash
    super> gnss mt3333 version
    0d,00:02:40.340. (GNSS) MT. Started
    OK
    super> MT3333 version : AXN_5.1.7_3333_19041711
    0d,00:02:41.087. (GNSS) MT. Power down
    super>   
    ```

If any of the firmware components of your boards is not the same as printed above then follow the instructions of the [Abeeway EVB initial FW flashing](https://github.com/Abeeway/abeeway-geolocation-module/tree/master/docs/Type1WL-EVB_first_flash.md) user guide.

## Checking / Configuring LoRaWAN Parameters

If you don't know the LoRaWAN EUIs and Keys of your board, check the `provis lora` and `lora info` commands.

   ```bash
   super> provis lora    
   Missing options for lora
   Options are:
           display   : Display current provisioning settings
           erase     : Erase provisioning settings from flash
           factory   : [region]. Factory reset
           restore   : Read the provisioning from the flash
           set       : Set LoRa provisioning settings
           save      : Save LoRa provisioning settings to flash
   ERROR
   ```

   ```bash
   super> lora info
   LoRa information
    Versions
     LoRa: 1.0.4.1
     Regional: 1.0.3.2
     Modem: 4.1.0
     Chip. HW: 34. Type: 1, FW: 0x308
    EUIs
     DevEUI:20 63 5f 01 f0 01 99 4f
     JoinEUI:20 63 5f 00 0a 00 00 01
    MAC
     Region: EU868
     TX strategy: Network (ADR)
     State: Idle
     Joined: No
     DevAddr: 0x00000000
     DevNonce: 0
     Duty-cycle: Accept. Remaining 0 ms
   OK
   super>
   ```

*Note: The `provis lora display` command won't show the Keys, but you can reate new keys with the `provis lora set` command.*

   ```bash
   super> provis lora factory                                    
   OK
   super> provis lora set appkey 00000000000000000000000000000000
   OK
   super> provis lora display
   Provisioning data (unsaved):
                  MAC Region :          0 (EU868)
                  Activation :          0 (OTAA)
                  Device EUI : 20-63-5f-01-f0-01-99-4f
                    Join EUI : 20-63-5f-00-00-00-00-01
              nwkkey defined :      false
              appkey defined :       true
                 Parameter 0 : 0x00000000
                 Parameter 1 : 0x00000000
                 Parameter 2 : 0x00000000
                 Parameter 3 : 0x00000000
                 Parameter 4 : 0x00000000
                 Parameter 5 : 0x00000000
                 Parameter 6 : 0x00000000
                 Parameter 7 : 0x00000000
                 Parameter 8 : 0x00000000
                 Parameter 9 : 0x00000000
                Parameter 10 : 0x00000000
                Parameter 11 : 0x00000000
                Parameter 12 : 0x00000000
                Parameter 13 : 0x00000000
                Parameter 14 : 0x00000000
                Parameter 15 : 0x00000000
   super>
   ```

After the EVB is configured through the Command Line Interface (CLI) of the MFG firmware, you can replace the MFG with your own application or with one of the Demo Applications provided together with the Abeeway Software Development Kit.  

## Software Development Kit (SDK)

This Git repository collects all releases of the SDK provided for the `Type1WL Evaluation Board`.
Each release is placed in a diffferent branch as listed below:

- [aos-sdk_v1.2-1](https://github.com/Abeeway/abeeway-geolocation-module/tree/aos-sdk_v1.2-1) (Latest)
- [aos-sdk_v1.1-0](https://github.com/Abeeway/abeeway-geolocation-module/tree/aos-sdk_v1.1-0) (Deprecated)
- [aos-sdk-v0.5-0](https://github.com/Abeeway/abeeway-geolocation-module/tree/aos-sdk-release-V0.5-0-2023.02.24) (Deprecated)
