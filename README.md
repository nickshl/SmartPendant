# SmartPendant

Project of SmartPendant for grblHAL

![Image](https://github.com/Devtronic-US/SmartPendant/raw/main/Media/Devtronic_SmartPendant_Case.png "Devtronic SmartPendant Case")

## Software

To build SmartPendant [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) is used
To load firmware via USB [STM32CubeProg](https://www.st.com/en/development-tools/stm32cubeprog.html) is used

## Hardware

Full assembled custom board is be available there(US only): https://devtronic.square.site/

To make this project yourself, you will need three essential parts:

* [WeAct BlackPill F411 25M HSE:](https://s.click.aliexpress.com/e/_DC6TlGd)

* [3.5" Display with touchscreen based on ILI9488 LCD controller and FT6236 touch controller](https://www.aliexpress.us/item/3256804935586911.html)

* [60 mm 6 pin 100 PPR handwheel](https://s.click.aliexpress.com/e/_DCFuJHr)

This handwheel also works from 3.3V. STM321F411 pins handwheel is connected to is 5V tolerant, so it can be powered from 5V.

## Schematic

Schematic vesion 1.3(and later versions, only layout slightly changed):
![Image](https://github.com/Devtronic-US/SmartPendant/raw/main/Media/Schematic_Smart_Pendant_v1_3.png "Devtronic SmartPendant Schematic")

# Precompiled Firmware

Latest firmware HEX file can be found in Release folder: [SmartPendant.hex](https://github.com/nickshl/SmartPendant/blob/main/Release/SmartPendant.hex)

To load new firmware [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) is used:
1) If your device programmed with version **0.027.0** or later, hold top edge(MPG) button. Otherwise(not programmed, update failure, older version) hold BOOT0 button on the back instead.
2) Connect SmartPendant to PC using USB-C cable(it should be powered only from the USB!) or hit reset button on the back if it already connected and powered.
3) Couple seconds later release MPG or BOOT0 button.
4) Open STM32CubeProgrammer. In top right corner choose "USB" from drop down list.
5) If field "Port" in "USB Configuration" show "No DFU detected" click update button near it.
6) Click "Connect" button - STM32CubeProgrammer should establish connection and show current device memory content.
7) Click "Open File" in left to corner, select firmware HEX file, then click "Download" button in top left corner.
8) When flashing is done, close STM32CubeProgrammer and power cycle the device or short press NRST button on the device to restart it. 

**Note:** If during update you get and error, you may try to use different cable, anothger PC, connect device to the PC via USB hub(that is usually help by some reason) or slightly heatup STM32F411 MCU with hair dryer.
This error appears because STM32 bootloader uses internal RC oscillator to measure crystal osillator frequency. According Application Note AN2606 external crystal oscillator can be in range 4...26 MHz with 1 MHz step. However higher frequencies has lower error margin which in some cases can cause incorrect frequency detection and errors during bootload process.

Starting from version **0.027.0** if firmware detects that top edge(MPG or KEY on BlackPill) button is pressed, it calibrates internal RC oscillator and makes jump to bootloader, which should mitigate problem described above.

**Not relevant anymore:** if flashing is successful, but firmware isn't work, check STM32CubeProgrammer log. If it says "sector 0000 does not exist" erasing operation wasn't successful and new firmware "merged"(write operation can only flip bits from 1 to 0) with old one. To fix this issue, press "eraser" button in bottom right corner and click "Ok" button in pop up window. After erasing is finished "Device memory" should show only FFFFFFFF. Flash new firmware after that. 

# Thanks

Many thanks to Terje who made this project possible by adding another datastream to grblHAL.