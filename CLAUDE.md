# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Firmware for the **SmartPendant** — a touchscreen MPG/DRO pendant for **grblHAL** CNC controllers. Target MCU is an **STM32F411CEU** (WeAct BlackPill): 128 kB RAM, 512 kB flash, single-precision FPU only (`double` is soft-float and pulls in `__aeabi_d*`).

Hardware: an ILI9488 SPI display (480×320 panel driven **in portrait**, see below), an FT6236 capacitive touch controller, a 100 PPR quadrature handwheel, **seven buttons** (2 face, 4 side, 1 USR on the BlackPill — see `InputDrv::ButtonType`), a buzzer, MB85RC256V FRAM (32 kB) for settings, and an SD card. It talks to the grblHAL controller over UART in **"MPG & DRO mode"**, either as a plain byte stream or through the framed transport (see `FramedUart` below).

### Screen geometry — derive from code, don't assume

The panel is 480×320 but `Application` sets `IDisplay::ROTATION_RIGHT`, so the framebuffer is **320 wide × 480 tall**. `GetScreenW()` returns 320.

| region | y |
|---|---|
| header | 0 – 40 |
| **area handed to a screen's `Setup(y, height)`** | **40 – 410 → 320 × 370 px** |
| status box | 410 – 442 |
| soft buttons | 444 – 480 |

Fonts available: `Font_4x6`, `Font_6x8`, `Font_8x8`, `Font_8x12`, `Font_10x18`, `Font_12x16` (names are width×height). A menu row fits 32 characters at `Font_10x18`.

## Repository setup (do this first)

`DevCore/` is a **git submodule** (https://github.com/nickshl/DevCore.git) and is **not vendored**. The base framework (`AppTask`, `DisplayDrv`, `SoundDrv`, UI widgets `UiButton`/`String`/`DataWindow`, `RtosTick`, HAL wrappers `StHal*`, display/touch/eeprom drivers, fonts) lives there.

```
git submodule update --init --recursive     # if cloned without --recurse-submodules
```

When a symbol isn't found in `Application/`, look in `DevCore/`.

## Building

- **STM32CubeIDE** — import the project (`.cproject`/`.project`), build, flash with STM32CubeProgrammer.
- **CMake + arm-none-eabi** (CMake ≥ 4.0.0):
  ```
  mkdir build && cd build
  cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/arm_none_eabi_gcc.cmake -DCMAKE_BUILD_TYPE=Debug ..
  make
  ```
  `CMakeLists.txt` globs sources from `DevCore/ Drivers/ Middlewares/ Src/ Startup/ Application/`. Key compile defs: `STM32F411xE`, `USE_HAL_DRIVER`, `SWAP_BUTTONS=1`. Post-build produces `.elf`, `.hex`, `.bin`, `.lst` and a size report.

### Host-testing self-contained components

The firmware as a whole only runs on hardware, but **several components build and run on a PC with small stubs**, and doing so has found real bugs that reading did not:

- `Little-C.cpp` needs only a stub `GrblComm.h` (four methods). Running all scripts in `Scripts/` through an old and a new build and diffing the emitted G-code is a strong regression test.
- `FramedUart.cpp` needs stub `DevCore.h`/`IUart.h`. A simulated controller implementing `PROTOCOL.md` plus fault injection (frame loss, bit corruption) verifies the protocol properly. Build the fuzzing suite with `-fsanitize=address,undefined` or it proves little, and change the fuzz seed before trusting a clean result — a fixed seed proves less than it looks like it does.
- `Decimal32.h` is header-only and fully host-testable.

`arm-none-eabi-gcc -Os -fstack-usage` and `arm-none-eabi-size` give real stack/flash numbers — prefer measuring to estimating.

`Tests/host/run.py` builds the interpreter and communication layers with HAL/RTOS
stubs under AddressSanitizer and UndefinedBehaviorSanitizer, checks ProgramSender's
streaming timer, and compares all bundled scripts against a git baseline. See
`Tests/host/README.md`.

**When verifying, delete the old binaries before rebuilding.** Stale executables printing "all tests passed" after a failed compile has caused false confidence more than once.

## Flashing / bootloader entry

From **v0.027.0** on, holding the **top-edge (MPG / USR) button** at power-on calibrates the internal RC oscillator and jumps to the STM32 system bootloader (DFU over USB-C) — see `Bootloader()` / `CalibrateHSI()` in `AppMain.cpp`. Older units use the BOOT0 button. Precompiled firmware lives in `Release/`.

## Architecture

### Startup (`Application/AppMain.cpp`)
`AppMain()` is the C entry point called from the CubeMX-generated `Src/` code. It auto-detects the crystal (8 vs 25 MHz) and configures the PLL; checks the USR button for bootloader entry; instantiates HAL-wrapper objects for every peripheral (`StHalSpi/Iic/Uart/Gpio`, `ILI9488`, `FT6236`, `Eeprom24`); reads settings from FRAM; then starts the FreeRTOS tasks: `DisplayDrv`, `SoundDrv`, `InputDrv`, and either **`Tetris`** (if the left-up button is held at boot — an easter egg) or the normal **`GrblComm` + `Application`** pair.

`NVM` is **not** a task — it's a plain class. `NVM::ReadData()` reaches the EEPROM through blocking `HAL_I2C_Mem_Read` with no RTOS primitives, so it is safe to call from `AppMain` before the scheduler starts. That is what makes settings available in time to configure and choose the UART link layer.

### Task model
Everything of substance is a **singleton FreeRTOS task** subclassing `AppTask` (from DevCore), accessed via `X::GetInstance()`. Tasks override `Setup()`, `TimerExpired(interval)`, `ProcessMessage()` and `ProcessCallback(ptr)`, returning a `Result`. Cross-task calls are marshaled via `AppTask::Callback(...)`.

A message that can't be handled now is **re-queued to the front by `ProcessMessage()` itself** (see `GrblComm`: on `ERR_BUSY`/`ERR_UART_BUSY` it calls `SendTaskMessage(&rcv_msg, true)` and delays a tick). `AppTask` does not re-queue for you, but nothing is dropped either — this is the documented pattern.

### Screens (`IScreen`)
A stack of screens implementing `Application/IScreen.h` (`Setup/Show/Hide/TimerExpired/ProcessCallback`). `Application` owns `scr[]` and switches with `ChangeScreen()`, which calls the old screen's `Hide()` **before** the new screen's `Show()`. The screen set depends on the controller's mode of operation (MILL vs LATHE). Navigation is via `Header` page tabs (`Header::MAX_PAGES` = 8). Screens: DirectControl (MPG jog), OverrideCtrl, DelayControl (power feed), RotaryTable, ProgramSender, GCodeGenerator, Probe, Settings. `MsgBox` and `ChangeValueBox` are modal overlays.

`Tabs` (in `Application/`, not DevCore) has a `MAX_TABS` limit and `SetParams()` **silently clamps** to it — a tab beyond the limit just never appears.

Anything a screen leaves set in `Hide()` outlives it: screens are also torn down by the settings-changed re-init in `Application::TimerExpired()`, which bypasses `DisableScreenChange()`. Reset run/sequence state in `Hide()`.

### Input (`Application/InputDrv.*`)
Timer-driven task reading the quadrature encoder, buttons (debounced) and the FT6236. Screens **register** encoder/button callbacks in `Show()` and **remove** them in `Hide()`. Callbacks live in intrusive doubly-linked lists guarded by a mutex; new handlers are inserted at the **head**, and only the **first matching** handler is notified — so the most recently shown object wins, which is why a modal must be shown *after* the screen beneath it.

### grblHAL communication (`Application/GrblComm.*`) — the core, and the trickiest code
Singleton UART task, 1 ms tick. Parses real-time status reports (`<...>`), messages (`[...]`, e.g. `PRB:`/`TLO:`/`AXS:`), settings (`$...`), and `ok`/`error:` responses. Maintains a timing/handshake state machine for gaining and releasing MPG control. Axis data is stored in `[AXIS_CNT]` (=6, XYZABC) arrays; `GetLimitedNumberOfAxis(n)` is the safe accessor for iterating axes. Uncomment `#define SEND_DATA_TO_USB` in `GrblComm.h` to mirror traffic to USB CDC.

`InitTask()` takes an **`IUart&`**, not a concrete UART, which is what lets `AppMain` hand it either the raw hardware UART or a `FramedUart`. `GrblComm` is unaware of which it got.

Three things that are easy to get wrong:
- **`[PRB:...]` is always in machine coordinates**, regardless of the `$10` WPos/MPos setting (grblHAL `report_probe_parameters()`). The axis-position getters convert by report frame; the probe getters must not.
- **Real-time commands are always a single-byte write** (`msg.id == 0`, `msg.cmd[1] = '\0'`), while g-code lines always carry a terminator and are ≥2 bytes. The framed transport relies on this to pick its channel — keep the invariant.
- **`PollSerial()` reassembles lines across read boundaries** and must keep doing so. Two control bytes are handled there, and **neither branch is dead code** even though nothing in `GrblComm` ever sends them — `FramedUart` injects both (see below). `ASCII_CAN` (0x18) clears the partial line and sets `skip_until_lf`, which discards everything up to the next terminator. `ASCII_NAK` (0x15) means the transport gave up on a command whose delivery is uncertain; it is handled like an `error:` response so a caller streaming a program stops rather than moving on.

### UART link layer (`Application/FramedUart.*`)
`FramedUart` is an `IUart` that wraps another `IUart`, adding framing, CRC, sequencing, acknowledgement and retransmission for the grblHAL MPG link. The controller side is the **`Plugin_mpg_transport`** plugin; its `PROTOCOL.md` is normative, and when the document and `mpg_transport.c` disagree, **the code is correct**. `AppMain` decides which object to hand over, after reading settings and before creating the task. **Reboot only**; both ends must be configured to match, there is no negotiation and no fallback.

The block comment at the top of `FramedUart.h` is the design summary — read it first. Design points worth not re-deriving:

- Channel is chosen by write length (1 byte → real-time channel), never by inspecting content.
- Back pressure lives in `Write()` returning `ERR_UART_BUSY` per channel. `IsTxComplete()` means "can accept something" and is false only when *both* channels are waiting — gating it on acknowledgement alone delays a feed hold behind a retransmitting g-code frame (measured 2 ms → 302 ms).
- The acknowledge timeout is derived from the baud rate, so `SetBaudRate()` must be called **on the `FramedUart`**, not on the wrapped hardware UART. `NVM::ACK_MIN_MS` raises it and can never lower it — the timing rule is one-sided, and lowering it below the derived value makes every frame go out twice.
- `MAX_ATTEMPTS` counts **transmissions**, not retries. Timeouts and Naks share one budget, so a peer that keeps sending Nak can't hold a channel forever. Both paths go through `RetransmitOrDrop()`.
- Sequence 0 is reserved for the first frame after a reset; rotation is 1..255 wrapping to 1 (`NextSeq()`). Duplicate detection is the **sequence number alone** — adding the CRC would make suppression depend on the peer retransmitting byte-identically, and a peer that rebuilds a frame would get a move executed twice.
- **A command is always one frame and is never split.** The controller counts an inbound gap but deliberately does not act on it, so a split command lost mid-way would leave a fragment in grblHAL's line buffer that can still parse as valid g-code.
- When frames are lost, `FramedUart` writes `RESYNC_MARKER` (0x18) into the receive buffer ahead of the next payload, **atomically with it** — a payload must never reach the reader without it. This keeps the transport unaware of its reader; `GrblComm` already gave that byte the right meaning.
- When a **command** frame is given up on, `FramedUart` writes `CMD_LOST_MARKER` (0x15).
  This reports failure even while status reports keep arriving, when the status
  watchdog would not fire. Delivery is uncertain: the controller may have received
  the command and lost its acknowledgements. Only the command channel reports
  (`tx_channel_t::report_loss`); a real time frame has no response outstanding.
  The byte is **retried rather than dropped** if the receive buffer is full.

### Settings / NVM (`Application/NVM.*`)
Settings are a struct persisted to FRAM over I2C (`Eeprom24`), CRC-protected (`crc` is the last field; the CRC covers everything before it). Parameters are addressed by the `NVM::Parameters` enum, and `menu_strings[NVM::MAX_VALUES]` in `SettingsScr` is indexed by that **absolute** enum value — the two must stay in step.

Link parameters are `BAUD_RATE`, `TRANSPORT`, `FRAME_ATTEMPTS` and `ACK_MIN_MS`. The last two mirror controller settings and should be set to the same values; they are applied immediately, while baud and transport need a reboot.

**Adding a parameter resets every setting to defaults.** The array is sized `MAX_VALUES`, so a new entry changes `sizeof(data)`, which moves both the CRC's coverage and its position; the check then fails and defaults load. The `EEP_VERSION` block in `ReadData()` is currently empty, so it provides no migration. If this becomes painful, the cheapest fix is a fixed-size storage array (e.g. 128 slots) with unused slots written as a sentinel, so appending a parameter preserves the others.

### Script-driven G-code generation
`GCodeGeneratorScr` runs user **scripts** through an embedded C interpreter (`Application/Little-C.*`) to emit G-code, handed to `ProgramSender`. Scripts live in `Scripts/` on the SD card: **`.ms` = mill**, **`.ls` = lathe** (filtered by mode of operation).

Scripts are near-C and declare tunable parameters as global variable declarations with a structured trailing comment that the generator parses to build the parameter-entry UI:

```c
int step = 3000;      // Step for pass; 1000; mm; 0; 1000000
                      //   name        ; scaler; units; min; max
int coolant = 0;      // Coolant; 0; Flood; Mist; None
                      //   name  ; 0 == enum marker; enum labels...
```

`main()` emits G-code via built-ins: `println(...)`/`print(...)`/`puts(...)`/`putch(...)`, `GetAxisPosX/Y/Z()`, `abs()`, `sqrt()`. The interpreter has a fixed 80-byte token buffer (so **no string literal in a script may reach 80 characters** — build long output lines from several `print`/`println` calls), a variable stack split into local-frame and global regions, a function table, and a nesting-depth limit that bounds recursion. Output can be written to `Result.nc` when `NVM::SAVE_SCRIPT_RESULT` is enabled.

Each prescan, execution, and global-value reset also receives a **250,000-token
budget** (`LittleC::TOKEN_BUDGET`). Exhaustion returns a script error instead of
freezing the Application task. Tokenization finishes normally; statement/expression
entry points unwind the failure without invalidating token pointers. This is a
work limit, not a wall-clock deadline or a hardware watchdog.

Probe reports are parsed separately from ordinary axis status: all configured
coordinates must be finite and followed by exactly `:0]` or `:1]`. Malformed
reports invalidate freshness and success without changing cached coordinates.

## Conventions (match these when editing)

- **File-scoped singletons**: `X::GetInstance()`. Members initialized inline in the header.
- Functions return `Result` (`RESULT_OK`, `ERR_*`); check it.
- Unsigned literals get a `u` suffix (`300u`, `0u`); `nullptr` not `NULL`.
- Array sizes via the `NumberOf(arr)` macro, never a hard-coded count.
- **MISRA empty else**: required only to terminate an `if … else if` chain (Rule 15.7). A plain `if` with no `else if` does **not** need `else { ; // Do nothing - MISRA rule }` — don't add them.
- `inline` on member functions **defined in the class body with a single statement**; omitted on multi-line ones. (In-class definitions are implicitly inline; the keyword is a readability convention here.)
- Prefer a `bool` parameter with named `static constexpr bool` constants over a small enum.
- **Dense banner comments (`// ***`) precede every function — they are navigation, not decoration.** Keep the format and don't "clean them up": they are how a human scrolls a 660-line header. Scope label is `Public:`, `Private:`, or `Global:` for file-scope functions.
- Time via `RtosTick::GetTimeMs()`; tick math uses unsigned wraparound subtraction — preserve that idiom. For "has N ms passed since", use `RtosTick::CheckTimeDifferenceMs(timestamp, ms)` rather than comparing an absolute deadline, which breaks at the 49.7-day rollover.
- Comments should explain *why*. In reusable DevCore-style classes, keep them free of machine-specific units and assumptions.

## Gotchas

- **Bump the version** in `Application/Version.h` (`VERSION_MAJOR/MINOR/BUILD`) for a release build — there's a literal "DON'T FORGET TO CHANGE IT" note there.
- **`Src/` and `Inc/` are CubeMX-generated** from `SmartPendant.ioc`. Regenerating overwrites HAL init / peripheral config — hand edits there are fragile. Application logic belongs in `Application/`.
- **Allocations that may fail must use `new(std::nothrow)`.** DevCore overrides global `operator new` to call `Break()` on failure, which is `bkpt #0` — a hard fault on a unit with no debugger attached. `ProgramSender` deliberately allocates the largest free block and falls back to line-by-line streaming when it can't, which only works with the nothrow form.
- Robustness matters: this parses live, sometimes noisy, UART data and reads arbitrary SD card filenames — guard string parsing (`strchr`/length math) and array bounds; malformed input must not fault.
- G-code lines are limited to **80 characters** (`TextBox::MAX_LINE_LEN`). Programs are checked at load; a longer line means the file is refused, not truncated mid-run.
- The status watchdog sets `grbl_state = UNKNOWN` after 300 ms without a report, but does **not** clear `grbl_mpgMode`, so `IsInControl()` stays true after a link loss.
- **Command completion must fail closed.** Watchdog recovery preserves `send_id`
  and publishes `Status_Comm_Error`; unsolicited late `ok` cannot clear it.
  ProgramSender advances only on `Status_OK`, and `IsStatusReceivedAfterCmd()`
  rejects superseded results. Publish pending state only after UART acceptance:
  framed writes can return `ERR_UART_BUSY` while another channel is available.
  A non-OK status closes the command gate in `ProcessMessage()`. It reopens only
  through the Stop/Reset/Unlock triple (status OK, pending cleared,
  `send_id = next_id`), through `ReleaseControl()`, or automatically for
  `Status_Comm_Error` alone when the state returns from UNKNOWN. Always advance
  `send_id` when clearing, so the failed command reads
  `Status_Next_Cmd_Executed`, never OK.

### Invariants nothing enforces

Break any of these and it still compiles.

- **`FramedUart::RESYNC_MARKER` == `GrblComm::ASCII_CAN`** (both 0x18) and **`FramedUart::CMD_LOST_MARKER` == `GrblComm::ASCII_NAK`** (both 0x15). They can't share a header — the transport must not know its reader.
- **The wire format block at the top of `FramedUart.h` is a COPY** of the plugin's `mpg_transport.h` (constants, frame type enum, CRC, frame builder). The pendant can't include a grblHAL header, so nothing checks they agree. Names are deliberately identical so the block can be diffed; expected differences are house formatting only. The host conformance suite checks the CRC and frame vectors from `PROTOCOL.md`, which catches a real divergence.
- **Statistics counters must stay `uint32_t`.** `SettingsScr` reads them from the Application task while the comm task writes them, without a lock — safe only because a 32-bit aligned word loads and stores atomically on this core.
- **General tab rows must stay contiguous from `NVM::TX_CONTROL`.** `SettingsScr` maps menu index to NVM index by fixed offset in two places; inserting a parameter mid-enum without adding the row in the matching position silently misroutes every row below it.
- **`NVM::Parameters`, the defaults array, and `SettingsScr::menu_strings[]` must stay index-for-index aligned.**
- **`GrblComm::msg_t::cmd[]` must not exceed the transport's maximum payload.** At 128 there is exactly one byte of margin. Grow it and long commands are silently dropped in framed mode — `Write()` returns `ERR_BAD_PARAMETER`, which `ProcessMessage()` does not re-queue — while plain mode keeps working.
- **`Menu` passes row 0 as `(void*)0`, which is `nullptr`.** Adding an `if(ptr != nullptr)` guard to a menu callback silently disables the first row.
