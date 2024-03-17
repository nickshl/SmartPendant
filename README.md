# SmartPendant

Project of SmartPendant for grblHAL

![Image](https://github.com/Devtronic-US/SmartPendant/raw/main/Media/Devtronic_SmartPendant_Case.png "Devtronic SmartPendant Case")

## Software

To build SmartPendant [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) is used
To load firmware via USB [STM32CubeProg](https://www.st.com/en/development-tools/stm32cubeprog.html) is used

## Hardware

Full assembled custom board will be available at some point: https://github.com/Devtronic-US/SmartPendant

To make this project yourself, you will need three essential parts:

* [WeAct BlackPill F411 25M HSE:](https://s.click.aliexpress.com/e/_DC6TlGd)

* [3.5" Display with touchscreen based on ILI9488 LCD controller and FT6236 touch controller](https://www.aliexpress.us/item/3256804935586911.html)

* [60 mm 6 pin 100 PPR handwheel](https://s.click.aliexpress.com/e/_DCFuJHr)

This handwheel also works from 3.3V. STM321F411 pins handwheel connected to is 5V tolerant, so it can be powered from 5V.

## Schematic

![Image](https://github.com/Devtronic-US/SmartPendant/raw/main/Media/Schematic_Smart_Pendant_v1_1.png "Devtronic SmartPendant Schematic")

# Thanks

Many thanks to Terje who made this project possible by adding another datastream to grblHAL.