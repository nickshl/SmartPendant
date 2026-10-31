# SmartPendant

Firmware for the SmartPendant for grblHAL project.

![Image](https://github.com/Devtronic-US/SmartPendant/raw/main/Media/Devtronic_SmartPendant_Case.png "Devtronic SmartPendant Case")

## Software

To clone the project:

```
git clone --recurse-submodules https://github.com/nickshl/SmartPendant.git
```

`DevCore` is a git submodule. After pulling changes into an existing clone, update it as well:

```
git submodule update --init --recursive
```

### Building using the IDE

Import the project into [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html). Set Build configuration (Release/Debug) and build.

You can then download the firmware with [STM32CubeProg](https://www.st.com/en/development-tools/stm32cubeprog.html).

### Building from the command line

Prerequisites:

* gcc C/C++ compiler (arm-none-eabi-gcc)
* CMake (version 4.0.0 or newer)

Run CMake:

```
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/arm_none_eabi_gcc.cmake -DCMAKE_BUILD_TYPE=Debug ..
```
Now you can build the project using `make`.

### Host tests

Parts of the firmware(script interpreter, grblHAL communication, program sender, settings storage) can be built and tested on a PC with Python 3 and a C++17 compiler: `python3 Tests/host/run.py`. See [Tests/host/README.md](Tests/host/README.md) for what each test covers.

## Hardware

### Purchase

Fully assembled custom board is available here (US only): https://devtronic.square.site/

### Using ordered PCB from PCB manufacturer

PCB files available there: [https://github.com/Devtronic-US/SmartPendant](https://github.com/Devtronic-US/SmartPendant/)

### DIY version from off the shelf components

To make this project yourself, you will need three essential parts:

* [WeAct BlackPill F411 25M HSE:](https://s.click.aliexpress.com/e/_DC6TlGd)

* [3.5" Display with touchscreen based on ILI9488 LCD controller and FT6236 touch controller](https://www.aliexpress.us/item/3256804935586911.html)

* [60 mm 6 pin 100 PPR handwheel](https://s.click.aliexpress.com/e/_DCFuJHr)

This handwheel also works with 3.3V. The STM321F411 pins that the handwheel is connected to are 5V tolerant, so it can be powered from 5V.
Note that pendant itself needs 5v power supply, it won't work with 3v.

### Case

* [3D printed Case](https://github.com/Devtronic-US/SmartPendant/)

## Compatibility

* Controller running latest GrblHAL

* Available UART port

* Controller's UART port should be configured in "MPG & DRO mode"

Detailed instructions how to check compatibility are [available here.](https://www.youtube.com/watch?v=ToWwejkP6e4)

## GRBLHAL Configuration recommendations

Consider turning on these options inside GRBLHAL General section:

* Work coordinate offset

* Overrides

* Enable when homing

## Debugging

You can uncomment `#define SEND_DATA_TO_USB` line in `Application/GrblComm.h` file, recompile firmware. 
With that controller mirrors all GRBL communication protocol to USB-based com port.

## Probing

### Center Finder

You can select which axes will be used for searching. Active axes are highlighted in red. For outside center finding 2 additional parameters are available: `DISTANCE` - how far to travel before lowering probe. `CLEARANCE` - lowering depth.

### Edge Finder

Active axes are highlighted in red. You can also select which direction to travel using `+` and `-` fields.
At the end of the process probe will be positioned at the very edge. `CLEARANCE` controls how far probe will be raised after the process.

`TIP DIAMETER` shows the probe ball diameter used to compensate the measured position. It is changed in Settings -> Probe -> Ball tip.

### Precise measurement

`PRECISE` button on Center Finder and Edge Finder performs 2 measurements with the probe turned 180 deg between them and uses the average, so a bent or misaligned stylus doesn't matter. Default state of the button is set in Settings -> Probe -> Precise by default.

The pendant asks to turn the probe by hand. If the spindle motor is a servo connected to a rotary axis of the controller(it holds the spindle, so the probe can't be turned by hand), set Settings -> Probe -> Spindle axis and Spindle speed: the pendant turns the probe itself and turns it back after the measurement, so the cable doesn't wrap around the spindle. Spindle speed is in degrees per second, -360 to 360: 360 turns the probe in 0.5 s, 180 in 1 s, 90 in 2 s. The sign sets the direction of the first turn.

**Important:** the spindle axis is a setting of the pendant, not of the controller. If the pendant is connected to another machine, or the controller is reconfigured, and that axis is a rotary axis there(a rotary table, for example), precise measurement will turn it. Check Settings -> Probe -> Spindle axis every time the pendant is used with a different machine.

### Tool Offset 

After using first tool, jog over probe and click `measure base` before changing. Change to the next tool and click `measure offset`. After process is completed, new tool offset will be set to match first tool length.

If the controller has a tool table, the tab works with it. The top window shows the selected tool and its offset from the table, the encoder selects the tool, a green border marks the current tool. Click the window to enter the offset by hand.
- `measure base` - with the probe or any tool which offset is already in the table.
- `measure offset` - measures the selected tool, writes its offset to the table and makes it current.
- `SELECT TOOL`(left button) - makes the selected tool current and applies its offset(`M61`), `DESELECT TOOL` - no tool, offset disabled.
- With ATC the buttons are `CHANGE TOOL` and `UNLOAD TOOL` and the tool is changed(`T M6`), `measure offset` loads the selected tool first.

A tool with zero offset is never applied. A tool with X, Y or radius set in the table can't be selected from the pendant until its offset is measured or entered here, which clears the rest.


## Schematic

Schematic vesion 1.3(and later versions, only layout slightly changed):
![Image](https://github.com/Devtronic-US/SmartPendant/raw/main/Media/Schematic_Smart_Pendant_v1_3.png "Devtronic SmartPendant Schematic")

# Precompiled Firmware

Firmware HEX and BIN files of every version are attached to [Releases](https://github.com/nickshl/SmartPendant/releases). The latest one is always here: [latest release](https://github.com/nickshl/SmartPendant/releases/latest).

**Note:** version **0.042.1** changed the way settings are stored in EEPROM. The first start after updating from an older version resets all settings to defaults and shows "Some parameters set to default" - check the settings once after the update. Later updates keep the settings; a new setting starts with its default value and the same message is shown once.

To load new firmware [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) is used:
1) If your device programmed with version **0.027.0** or later, hold top edge (MPG) button. Otherwise (not programmed, update failure, older version) hold BOOT0 button on the back instead.
2) Connect SmartPendant to PC using USB-C cable (it should be powered only from the USB!) or hit reset button on the back if it already connected and powered.
3) Couple seconds later release MPG or BOOT0 button.
4) Open STM32CubeProgrammer. In top right corner choose "USB" from drop down list.
5) If field "Port" in "USB Configuration" show "No DFU detected" click update button near it.
6) Click "Connect" button - STM32CubeProgrammer should establish connection and show current device memory content.
7) Click "Open File" in left to corner, select firmware HEX file, then click "Download" button in top left corner.
8) When flashing is done, close STM32CubeProgrammer and power cycle the device or short press NRST button on the device to restart it. 

**Note:** If during update you get an error, you may try to use a different cable, another PC, connect device to the PC via USB hub (that usually helps for some reason) or slightly heat up STM32F411 MCU with a hair dryer.
This error appears because STM32 bootloader uses internal RC oscillator to measure crystal oscillator frequency. According to Application Note AN2606, external crystal oscillator can be in range 4...26 MHz with 1 MHz step. However higher frequencies have lower error margin which in some cases can cause incorrect frequency detection and errors during bootload process.

**Not relevant anymore:** if flashing is successful, but firmware isn't work, check STM32CubeProgrammer log. If it says "sector 0000 does not exist" erasing operation wasn't successful and new firmware "merged"(write operation can only flip bits from 1 to 0) with old one. To fix this issue, press "eraser" button in bottom right corner and click "Ok" button in pop up window. After erasing is finished "Device memory" should show only FFFFFFFF. Flash new firmware after that. 

Starting from version **0.027.0** if firmware detects that top edge (MPG or KEY on BlackPill) button is pressed, it calibrates internal RC oscillator and makes jump to bootloader, which should mitigate problem described above.

# Thanks

Many thanks to Terje who made this project possible by adding another datastream to grblHAL.
