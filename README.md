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

Schematic vesion 1.3:
![Image](https://github.com/Devtronic-US/SmartPendant/raw/main/Media/Schematic_Smart_Pendant_v1_3.png "Devtronic SmartPendant Schematic")

# Precompiled Firmware

Latest firmware HEX file can be found in Release folder: [SmartPendant.hex](https://github.com/nickshl/SmartPendant/blob/main/Release/SmartPendant.hex)

To load new firmware [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) is used.
Connect SmarPendant to PC using USB-C cable. Press and hold BOOT0 button, then short press NRST button, couple seconds later BOOT0 button can be released.
Open STM32CubeProgrammer. In top right corner choose "USB" from drop down list.
If field "Port" in "USB Configuration" show "No DFU detected" click update button near it.
Click "Connect" button - STM32CubeProgrammer should establish connection and show current device memory content.
Click "Open File" in left to corner, select firmware HEX file, then click "Download" button in top left corner.
When flashing is done, close STM32CubeProgrammer and short press NRST button on the Controller to restart it. 

**Note:** if flashing is successful, but firmware isn't work, check STM32CubeProgrammer log. If it says "sector 0000 does not exist" erasing operation wasn't successful and new firmware "merged"(write operation can only flip bits from 1 to 0) with old one. To fix this issue, press "eraser" button in bottom right corner and click "Ok" button in pop up window. After erasing is finished "Device memory" should show only FFFFFFFF. Flash new firmware after that. 

# Thanks

Many thanks to Terje who made this project possible by adding another datastream to grblHAL.