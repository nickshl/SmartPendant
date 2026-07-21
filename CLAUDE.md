# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Firmware for the **SmartPendant** — a touchscreen MPG/DRO pendant for **grblHAL** CNC controllers. Target MCU is an **STM32F411CEU** (WeAct BlackPill). It drives a 480×320 ILI9488 SPI display with an FT6236 capacitive touch controller, a 100 PPR quadrature handwheel, three buttons, a buzzer, and MB85RC256V FRAM for settings. It talks to the grblHAL controller over UART configured in **"MPG & DRO mode"**.

## Repository setup (do this first)

`DevCore/` is a **git submodule** (https://github.com/nickshl/DevCore.git) and is **not vendored** in this repo. The base framework (`AppTask`, `DisplayDrv`, `SoundDrv`, UI widgets `UiButton`/`String`/`DataWindow`, `RtosTick`, HAL wrappers `StHal*`, display/touch/eeprom drivers, fonts) all live there.

```
git submodule update --init --recursive     # if the repo was cloned without --recurse-submodules
```

The base classes that `Application/` code inherits from and calls into are defined under `DevCore/` (see the include dirs in `CMakeLists.txt`). When a symbol isn't found in `Application/`, look in `DevCore/`.

## Building

Two supported paths (see `README.md` for full detail):

- **STM32CubeIDE** — import the project (`.cproject`/`.project`), build, flash with STM32CubeProgrammer.
- **CMake + arm-none-eabi** (CMake ≥ 4.0.0):
  ```
  mkdir build && cd build
  cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/arm_none_eabi_gcc.cmake -DCMAKE_BUILD_TYPE=Debug ..
  make
  ```
  `CMakeLists.txt` globs sources from `DevCore/ Drivers/ Middlewares/ Src/ Startup/ Application/`. Key compile defs: `STM32F411xE`, `USE_HAL_DRIVER`, `SWAP_BUTTONS=1`. Links against the flash linker script `STM32F411CEUX_FLASH.ld`. Post-build produces `.elf`, `.hex`, `.bin`, `.lst`, and a size report.

There are **no unit tests** and no host build — this is bare-metal firmware; the only "run" is flashing hardware. Verification is manual on-device.

## Flashing / bootloader entry

From **v0.027.0** on, holding the **top-edge (MPG / USR) button** at power-on calibrates the internal RC oscillator and jumps to the STM32 system bootloader (DFU over USB-C) — see `Bootloader()` / `CalibrateHSI()` in `AppMain.cpp`. Older units use the BOOT0 button. Precompiled firmware lives in `Release/`.

## Architecture

### Startup (`Application/AppMain.cpp`)
`AppMain()` is the C entry point called from the CubeMX-generated `Src/` code. It: auto-detects the crystal (8 vs 25 MHz) and configures the PLL; checks the USR button for bootloader entry; instantiates HAL-wrapper objects for every peripheral (`StHalSpi/Iic/Uart/Gpio`, `ILI9488`, `FT6236`, `Eeprom24`); then starts the FreeRTOS tasks: `NVM`, `DisplayDrv`, `SoundDrv`, `InputDrv`, and either **`Tetris`** (if the left-up button is held at boot — an easter egg) or the normal **`GrblComm` + `Application`** pair.

### Task model
Everything of substance is a **singleton FreeRTOS task** subclassing `AppTask` (from DevCore), accessed via `X::GetInstance()`. Tasks override `Setup()`, `TimerExpired(interval)`, `ProcessMessage()`, and `ProcessCallback(ptr)`, and return a `Result`. Cross-task calls are marshaled onto the target task via `AppTask::Callback(...)` so work runs in the right task context.

### Screens (`IScreen`)
The UI is a stack of screens implementing `Application/IScreen.h` (`Setup/Show/Hide/TimerExpired/ProcessCallback`). `Application` owns the screen array `scr[]` and switches between them with `ChangeScreen()` — which calls the old screen's `Hide()` **before** the new screen's `Show()`. Screen set depends on the controller's mode of operation (MILL vs LATHE). Navigation across top-level screens is via the `Header` page tabs. Screens include: DirectControl (MPG jog), OverrideCtrl, DelayControl (power feed), RotaryTable, ProgramSender (G-code sender), GCodeGenerator, Probe, Settings. `MsgBox` and `ChangeValueBox` are modal overlays shown on top of the active screen.

### Input (`Application/InputDrv.*`)
Timer-driven task reading the quadrature encoder, three GPIO buttons (debounced), and the FT6236 touch controller. Screens **register** encoder/button callbacks in `Show()` and **remove** them in `Hide()` via `Add/DeleteEncoderCallbackHandler` / `...ButtonsCallbackHandler`. Callbacks are stored in intrusive doubly-linked lists (`CallbackListEntry`). `Application` registers a persistent button handler at startup that is never removed; there is **no** persistent encoder handler.

### grblHAL communication (`Application/GrblComm.*`) — the core, and the trickiest code
Singleton UART task. Parses grblHAL real-time status reports (`<...>`), messages (`[...]`, e.g. `PRB:`/`TLO:`/`AXS:`), settings (`$...`), `ok`/`error:` responses. Maintains a **timing/handshake state machine** for gaining and releasing MPG control (`GainControl()`, `status_tx_timestamp`/`status_rx_timestamp`/`status_received`, `mpg_mode_request`, `respond_pending`). Axis data is stored in `[AXIS_CNT]` (=6, XYZABC) arrays; `number_of_axis` is what the controller reports; `GetLimitedNumberOfAxis(n)` is the safe accessor UI code uses when iterating axes. Uncomment `#define SEND_DATA_TO_USB` in `GrblComm.h` to mirror all controller traffic to a USB CDC serial port for debugging.

### Settings / NVM (`Application/NVM.*`)
Settings are a struct persisted to FRAM/EEPROM over I2C (`Eeprom24`), CRC-protected (`crc` is the last struct field; the CRC is computed over everything except itself). Parameters are addressed by the `NVM::Parameters` enum; `menu_strings[NVM::MAX_VALUES]` in `SettingsScr` is indexed by that **absolute** enum value. `EEP_VERSION` in `Version.h` guards layout migrations.

### Script-driven G-code generation — the non-obvious subsystem
`GCodeGeneratorScr` runs user **scripts** through an embedded C interpreter (`Application/Little-C.*`) to emit G-code programs, which are then handed to `ProgramSender`. Scripts live in `Scripts/` on the SD card: **`.ms` = mill**, **`.ls` = lathe** (filtered by the controller's mode of operation).

Scripts are near-C and declare their tunable parameters as **global variable declarations with a structured trailing comment** that the generator parses to build the parameter-entry UI:

```c
int step = 3000;      // Step for pass; 1000; mm; 0; 1000000
                      //   name        ; scaler; units; min; max
int coolant = 0;      // Coolant; 0; Flood; Mist; None
                      //   name  ; 0 == enum marker; enum labels...
```

`main()` emits G-code by calling built-ins: `println(...)`/`print(...)`/`puts(...)`/`putch(...)`, `GetAxisPosX/Y/Z()`, `abs()`, `sqrt()`. `println` accepts a literal string plus optional value args. The interpreter has a fixed 80-byte token buffer, a variable stack (`var_stack`, split into local-frame + global regions via `call_stack`/`functos`/`gvar_index`), and a function table. Generated output can optionally be written to `Result.nc` on the SD card when `NVM::SAVE_SCRIPT_RESULT` is enabled.

## Conventions (match these when editing)

- **File-scoped singletons**: `X::GetInstance()`. Members are initialized inline in the header.
- Functions return `Result` (`RESULT_OK`, `ERR_*`); check it.
- Unsigned literals get a `u` suffix (`300u`, `0u`); `nullptr` not `NULL`.
- Array sizes via the `NumberOf(arr)` macro, never a hard-coded count.
- MISRA-flavored style: exhaustive `if/else if/else`, with empty final branches written as `else { ; // Do nothing - MISRA rule }`.
- Dense banner comments (`// ***`) precede every function; keep the format.
- Time via `RtosTick::GetTimeMs()`; tick math uses unsigned wraparound subtraction — preserve that idiom.

## Gotchas

- **Bump the version** in `Application/Version.h` (`VERSION_MAJOR/MINOR/BUILD`) for a release build — there's a literal "DON'T FORGET TO CHANGE IT" note there.
- **`Src/` and `Inc/` are CubeMX-generated** from `SmartPendant.ioc`. Regenerating from the `.ioc` will overwrite HAL init / peripheral config — hand edits there are fragile. Application logic belongs in `Application/`.
- Malloc-failure hook is intentionally a no-op: `ProgramSender` relies on `new` returning `nullptr` when a program is too large to allocate — don't "fix" that hook to trap.
- Robustness matters: this parses live, sometimes noisy, UART data and reads arbitrary SD card filenames — guard string parsing (`strchr`/length math) and array bounds; malformed input must not fault.
