
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

Before starting any development on the Abeeway EVB, please verify if all components have the latest firmware and update if necessary. Please follow the instructions of the [Abeeway EVB initial FW flashing](https://github.com/Abeeway/abeeway-geolocation-module/tree/master/docs/Type1WL-EVB_first_flash.md) user guide.

## Software Development Kit (SDK)

This Git repository collects all releases of the SDK provided for the `Type1WL Evaluation Board`.
Each release is placed in a diffferent branch as listed below:

- [aos-sdk_v1.1-0](https://github.com/Abeeway/abeeway-geolocation-module/tree/aos-sdk_v1.1-0) (Latest)
- [aos-sdk-v0.5-0](https://github.com/Abeeway/abeeway-geolocation-module/tree/aos-sdk-release-V0.5-0-2023.02.24) (Deprecated)
