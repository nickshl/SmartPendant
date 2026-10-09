# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Firmware for the **SmartPendant** — a touchscreen MPG/DRO pendant for **grblHAL** CNC controllers. Target MCU is an **STM32F411CEU** (WeAct BlackPill): 128 kB RAM, 512 kB flash, single-precision FPU only (`double` is soft-float and pulls in `__aeabi_d*`).

Hardware: an ILI9488 SPI display (480×320 panel driven **in portrait**, see below), an FT6236 capacitive touch controller, a 100 PPR quadrature handwheel, **seven buttons** (2 face, 4 side, 1 USR on the BlackPill — see `InputDrv::ButtonType`), a buzzer, a 24xx256 class I2C EEPROM (32 kB) for settings, and an SD card. It talks to the grblHAL controller over UART in **"MPG & DRO mode"**, either as a plain byte stream or through the framed transport (see `FramedUart` below).

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

- `Little-C.cpp` needs only a stub `GrblComm.h` (`GetInstance()`, `GetAxisPosition()`, `IsLatheDiameterMode()`, `ConvertUnitsToMetric()`, `ConvertUnitsToImperial()` and the `AXIS_*` constants; copy the conversion helpers verbatim from the real header so the test runs the real unit math). Running all scripts in `Scripts/` through an old and a new build and diffing the emitted G-code is a strong regression test.
- `FramedUart.cpp` needs stub `DevCore.h`/`IUart.h`. A simulated controller implementing `PROTOCOL.md` plus fault injection (frame loss, bit corruption) verifies the protocol properly. Build the fuzzing suite with `-fsanitize=address,undefined` or it proves little, and change the fuzz seed before trusting a clean result — a fixed seed proves less than it looks like it does.
- `Decimal32.h` is header-only and fully host-testable.

`arm-none-eabi-gcc -Os -fstack-usage` and `arm-none-eabi-size` give real stack/flash numbers — prefer measuring to estimating. For a task's worst-case stack add `-fcallgraph-info=su`: the `.ci` files have every function's frame and its calls; virtual calls show only as `__indirect_call`, so a walk over the graph gives an upper bound, not the exact figure.

`Tests/host/run.py` builds the interpreter and communication layers with HAL/RTOS
stubs under AddressSanitizer and UndefinedBehaviorSanitizer, checks ProgramSender's
streaming timer, and compares all bundled scripts against a git baseline. See
`Tests/host/README.md`. The baseline interpreter must know every built-in the
scripts call: a `BASELINE_REF` older than the `GetMetricAxisPos…`/`GetImperialAxisPos…`
getters fails on any script that uses them.

`Tests/host/script_checks.py` checks what the bundled scripts cut: each script runs
through the real interpreter, `GrblComm` parser and menu comment parser over a sweep
of parameters, and the program is traced as a tool path(depth reached, no pass
deeper than the step, no rapid through material, modal state set and restored, menu
rows that fit). **Run it after any change to a script, the interpreter, the position
getters or the menu parser.** With `--grblhal-sim <grblHAL_sim>` every program is
also run through the real grblHAL parser. `Tests/host/script_mutations.py` puts
known bugs into the scripts and the firmware sources to prove the checks still
catch them - run it after changing a check. `Tests/host/README.md` describes each
check, the known issues the checks are told to excuse, how to run a single script
by hand and how to write a new check.

Which to run:
- `GrblComm`, `FramedUart`, `ProgramSender`, `Decimal32`, `Little-C` - `run.py`.
- A script, the interpreter, the position getters, the menu parser - `script_checks.py`(and `run.py` for the interpreter).
- A check in `script_checks.py` - `script_mutations.py`, it takes 5-10 minutes.
- `sender.h` and `stubs/` are doubles of the firmware: when a member is added to `ProgramSender` or a define to `DevCfgUsr.h` that the compiled code uses, add it there too, or `run.py` doesn't build.

**When verifying, delete the old binaries before rebuilding.** Stale executables printing "all tests passed" after a failed compile has caused false confidence more than once.

## Flashing / bootloader entry

From **v0.027.0** on, holding the **top-edge (MPG / USR) button** at power-on calibrates the internal RC oscillator and jumps to the STM32 system bootloader (DFU over USB-C) — see `Bootloader()` / `CalibrateHSI()` in `AppMain.cpp`. Older units use the BOOT0 button. Precompiled firmware lives in `Release/`.

## Architecture

### Startup (`Application/AppMain.cpp`)
`AppMain()` is the C entry point called from the CubeMX-generated `Src/` code. It auto-detects the crystal (8 vs 25 MHz) and configures the PLL; checks the USR button for bootloader entry; instantiates HAL-wrapper objects for every peripheral (`StHalSpi/Iic/Uart/Gpio`, `ILI9488`, `FT6236`, `Eeprom24`); reads settings from EEPROM; then starts the FreeRTOS tasks: `DisplayDrv`, `SoundDrv`, `InputDrv`, and either **`Tetris`** (if the left-up button is held at boot — an easter egg) or the normal **`GrblComm` + `Application`** pair.

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

Numbers the controller reports(positions, offsets, probe result, feed, rpm, `$110`..`$115`) are stored as **`Decimal32`**, parsed from the text by `ParseDecimal()` with `Decimal32::FromString()` - never through `atof()`/`float`. A binary float can't hold most decimals: `4.035` read through one and scaled to um was 4034. `FromString()` returns how many characters the number took(0 - there is none), which is how `ParseProbeReport()` finds the separator after it. The getters turn a number into counts with `Decimal32::ToFixedPoint(scaler)`: exact for what the controller prints(3 decimals for mm and degrees, 4 for inches); extra decimals are cut, not rounded, like everywhere in `Decimal32`. There is no floating point left in `GrblComm`; keep it that way - dropping `atof()`/`strtof()` also made the image about 10 kB smaller.

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
Settings are a struct persisted to a **24xx256 class I2C EEPROM** (`Eeprom24`), CRC-protected (`crc` is the last field; the CRC covers everything before it). Early boards had MB85RC256V FRAM and some comments still said so — it was replaced with EEPROM to cut cost, and the two behave nothing alike. The EEPROM gives ~1,000,000 erase/write cycles **per 64 byte page** with a 5 ms self-timed write; the record is 108 bytes at address 0, so **every save cycles pages 0 and 1 and blocks for ~10 ms** on the I2C bus shared with the touch controller. 510 of the 512 pages have never been written. Two consequences: don't persist anything that changes at machine rate without spreading writes, and note that a torn write (power loss inside that 10 ms) fails the CRC and resets **every** setting, because there is only one copy. Parameters are addressed by the `NVM::Parameters` enum, and `menu_strings[NVM::MAX_VALUES]` in `SettingsScr` is indexed by that **absolute** enum value — the two must stay in step.

Link parameters are `BAUD_RATE`, `TRANSPORT`, `FRAME_ATTEMPTS` and `ACK_MIN_MS`. The last two mirror controller settings and should be set to the same values; they are applied immediately, while baud and transport need a reboot.

**Adding a parameter resets every setting to defaults.** The array is sized `MAX_VALUES`, so a new entry changes `sizeof(data)`, which moves both the CRC's coverage and its position; the check then fails and defaults load. The `EEP_VERSION` block in `ReadData()` is currently empty, so it provides no migration. If this becomes painful, the cheapest fix is a fixed-size storage array (e.g. 128 slots) with unused slots written as a sentinel, so appending a parameter preserves the others.

### Script-driven G-code generation
`GCodeGeneratorScr` runs user **scripts** through an embedded C interpreter (`Application/Little-C.*`) to emit G-code, handed to `ProgramSender`. Scripts live in the `Scripts` folder on the SD card: **`.ms` = mill**, **`.ls` = lathe**; the list shows the ones for the controller's mode of operation(all of them when it isn't connected). The bundled ones are in `Scripts/` of this repository and are the best examples: read one of the same kind before writing a new one.

How a script is used: the operator picks it, the generator runs `Prescan()` and shows **every global variable as a menu row** - those are the parameters. The operator edits them(a number in a value box limited to min..max, an enum from its list) and presses Generate(only in Idle or Unknown state): `Execute()` runs `main()`, whatever it prints is the program. It goes to `ProgramSender` as a loaded program and, if `NVM::SAVE_SCRIPT_RESULT` is on, to `Result.nc`. The output buffer is the largest free block of the heap, tens of kB.

#### Parameters
Every global is a parameter, so **working variables must be locals**. Parameters are `int`, their order is the menu order, and the trailing comment is the menu row:

```c
int rough_step = 3000;  // Rough step; 1000; mm; 0; 1000000
                        //   label   ; scaler; units; min; max
int coolant = 0;        // Coolant; 0; Flood; Mist; None
                        //   label; 0 marks an enum; values shown for 0, 1, 2, ...
```

- The value is an integer in scaler units: with scaler 1000 and `mm`, 3000 is 3.000 mm. The scaler is a power of 10 and sets the decimals shown and entered. Use `mm` with 1000(um), `mm/min`, `rpm` and `cnt` with 1. min and max are in the same units as the value.
- An enum parameter holds the index of the chosen value: 0 for the first.
- The label is at most 23 characters, the units at most 7, and label plus value must fit the 32 character menu row, for the minimum, default and maximum and for every enum value. The label contains every word of the variable name, in any case, and may add some(`rough_step` - "Rough step", `clearance` - "Dive clearance"; "position" may be left out: `start_x_position` - "Face Start X").
- The menu has room for 31 parameters(the last row is Generate).
- A parameter's initializer is evaluated by `Prescan()`, so it can be a getter: `int start_x_position = GetMetricAxisPosX();` offers the current position as the default(`Facing.ms`).
- A direction can be a signed value(a negative length cuts toward -X) or an enum(`Facing.ms` has one); say which in the comment at the top.
- Use the names the bundled scripts use for the same thing: `feed`, `..._feed`, `speed`/`..._speed`(0 - the script doesn't start the spindle; minimum 0, default 0), `coolant`(Flood, Mist, None), `clearance`.
- Pick realistic limits for a machine this pendant drives - they are what keeps a typo from making a program that can't be run or doesn't fit the buffer.

#### The language
Little-C is a small subset of C, interpreted. What a script can use:

- types `int`(32 bit) and `char`; no arrays, pointers, structs, floating point, preprocessor;
- `if`/`else`, `while`, `do`/`while`, `for`, `switch`/`case`/`default`, `break`, `continue`, `return`;
- functions with `int` parameters returning `int`(`int f(int a, int b) { ... }`), `main()` without a type; recursion is possible but see the nesting limit;
- operators `+ - * / %`(integer: division cuts toward zero), `= += -= *= /= %=`, `++ --`, `== != < <= > >=`, `&& || !`, `?:`, unary `-`; **no** bit operators(`& | ^ ~ << >>`);
- locals declared in a block or in `for(int i = 0; ...)`, several in one line and with an initializer(`int a = 1, b;`);
- `return 0;` from `main()` ends the script early - with nothing printed it is an empty program, the way to say there is nothing to cut;
- `//` and `/* */` comments, character constants `'x'`, string literals only as arguments of the print functions.

Built-ins:

| function | does |
|---|---|
| `print(a, b, ...)` | prints its arguments one after another: a string literal as it is, an `int` as a number, a `char` as a character |
| `println(a, b, ...)` | the same and a new line |
| `printfp(value, scaler)` | used inside `print`/`println`: prints `value / scaler` with as many decimals as the scaler has zeros(`printfp(-1500, 1000)` - `-1.500`) |
| `puts("text")`, `putch(c)` | a string and a new line, one character |
| `abs(x)`, `sqrt(x)` | integer; `sqrt` cuts, 0 for negative |
| `GetMetricAxisPosX/Y/Z()` | position in um whatever the controller reports |
| `GetImperialAxisPosX/Y/Z()` | position in 0.0001 inch whatever the controller reports |
| `GetAxisPosX/Y/Z()` | position in the units the controller reports(`$13`) - avoid, the script then depends on the setting |
| `IsLatheDiameterMode()` | 1 if the lathe X is in diameter(G7) |

Limits that scripts run into:

- the token buffer is 80 bytes, so **no string literal may reach 80 characters**; build a long line from several arguments;
- a **program line** must stay within 80 characters(`TextBox::MAX_LINE_LEN`), with the longest numbers the parameters allow, or the program is refused;
- blocks, function calls and parentheses share a nesting depth of 10(`NEST_DEPTH_MAX`): keep nesting shallow, prefer a loop to recursion;
- 200 variables(`NUM_VARS`, globals and the locals of all active calls together) and 100 functions(`NUM_FUNC`);
- every run has a budget of 250,000 tokens(see below): a pass of a few moves costs a hundred or two, so a job of more than about a thousand passes ends in a "Script execution limit exceeded" error, not in a program - set the limits so that is a typo, not a normal job;
- `int` overflow isn't detected: a product of two values in um overflows above about 46 mm(46,340 um each), so divide before multiplying, or work in coarser units for a product.

#### What a program must look like
The bundled scripts follow these rules, and `Tests/host/script_checks.py` checks them on every bundled script:

- start with `M70`(save modal state) and set every mode the moves depend on before the first move, each once: `G21`(the bundled scripts are metric and use the metric getters), `G90` or `G91`, `G94`, `G40`, `G50`; a lathe script also `G7` or `G8`, one with arcs also `G17`;
- the spindle is started only if the speed parameter isn't 0: `M3 S<rpm>`(mill), `G97 M3 S<rpm>`(lathe); coolant `M8`(Flood), `M7`(Mist) or nothing, both before the first cut;
- every feed move(`G1`/`G2`/`G3`) has a positive `F` on the same line;
- the tool path starts at the current position: read it with the getters or move relative(`G91`), and never rapid into material - retract first; end with the tool out of the material, at or above the height it started from;
- end with `M72`(restore modal state), then `M9`(the bundled scripts send it whether coolant was on or not) and `M5`;
- a line of G-code may carry a comment after `;`(`println("G91; Relative mode")`) - it isn't sent to the controller.

Style: the file name says what it does with an -ing word(`Drilling.ms`, `Turning.ls`); a short comment at the top says what the script cuts, from where, and what 0 means for its parameters; short comments in the code, an empty line between logic blocks.

#### Testing a script
Add it to `LATHE` or `MILL` and to `VARIANTS` in `Tests/host/script_checks.py` - until then `structure` fails: a few parameter sets, the first one empty for the defaults. A script that reads the position also goes to `POSITION_SCRIPTS`. Then run the checks; they cover the common rules above, not the shape of what the script cuts - that needs a check of its own with mutations(`Tests/host/README.md`, "Changing a script" and "Writing a check"). To look at the program while writing, run the script by hand with `script_runner`("Running one script"); it builds under `build/host-tests/`, which is git-ignored.

Generate calls `Execute()` without a new `Prescan()`, so global variables keep what `main()` did to them and the menu shows it: **a script must not assign to its parameters**. Work on a local copy with a name of its own instead(`int stepover = drill_stepover;` in `Drilling.ms`) - not with the name of the parameter: that compiles, but a local that hides a global is easy to misread.

Each prescan, execution, and global-value reset also receives a fresh **250,000-token budget** (`TOKEN_BUDGET` in `Little-C.h`; `INT32_MAX` disables the limit). When it is spent, `get_token()` reports `EXECUTION_LIMIT` and returns `false` **without lexing** — `token`, `tok` and `token_type` still describe the previous token. A runaway script therefore ends in a script error instead of freezing the Application task, but only because every caller propagates that result: **never call `get_token()` without using what it returns**, and every scan loop needs `&& result` in its condition, or it spins forever on the stale token. This is a work limit, not a wall-clock deadline or a hardware watchdog.

Interpreter errors go through `sntx_err(error, result)`: pass the current `result`, and the message is written only while it is still `true`. The first error is the one the operator sees; checks made while unwinding cannot overwrite it. That is why a check is written `if(*token != ';') result = sntx_err(SEMI_EXPECTED, result);` rather than guarded with `if(result && ...)`. The one-argument form always reports. Braces are delimiters to the tokenizer, so `else{` and `do{` need no space.

### Program streaming (`Application/ProgramSender.*`)
A program is either in memory(`p_text`, also what a script generates) or, if it doesn't fit, streamed from the SD card line by line. Lines are sent one at a time, the next one after `ok`.

The controller executes a line long after it acknowledged it - it keeps tens of lines in its planner - so **the selection in the text box is not the place lines are taken from**. Sending has its own position(`p_send` in memory, `SDFile` on the card, `send_line` counts lines), and the selection follows the line the controller reports as executing:

- `BuildCommand()` makes the command from a program line: a `;` comment is removed(the controller only cuts it off), comments in parentheses stay(the controller handles them: `(MSG,...)` goes to the operator, plugins act on others) and `N<line>` is put **in front** - after the block delete `/` if there is one. In front, because that is the only place grblHAL takes a line number for every kind of line(flow control lines are parsed differently after the O word). Lines starting with `$`, `[` or `%` aren't g-code and go out untouched. A line with nothing left isn't sent, but it is counted: numbers are positions in the program.
- The program's own `N` words are removed when it is read(`StripLineNumbers()`), outside comments, `$` lines and `<names>`: two `N` words on a line are an error, and the numbers on screen would not be the ones reported.
- grblHAL reports `|Ln:n` in the status report(`$10` bit 2, on by default) for the line being executed. When nothing moves(dwell, spindle start, tool change, idle) builds since 20260126 report the last line parsed; **older ones report no number at all**, so a report without `Ln:` says nothing about whether the controller reports numbers. `GrblComm::GetLineNumber()` returns it, 0 if the last report had none. Numbers above 9999999 are refused by the controller, so such lines go without.
- Whether the selection follows the executed line is decided when Run is pressed, from `$10` bit 2(`GrblComm::IsLineNumberReportEnabled()`): `LINE_EXECUTED` if the controller reports line numbers, otherwise `LINE_SENT` - the line to be sent next, as it always did. **Do not decide it from what the reports contain or from when they arrive**: a controller may report no number while nothing moves, and while lines are sent every tick no report is "after the command" for `IsStatusReceivedAfterCmd()` - the selection sat on the first line until the planner filled up. A report without a number changes nothing. When the program has ended and the controller is idle the selection goes to the last line: the last lines without motion may never be reported. The selection never goes back.
- The number of the previous program can still be in `GrblComm` when Run is pressed. It is not used until the first numbered line is acknowledged; at that moment `ClearLineNumber()` forgets it, so any number seen later came in a report received after that `ok`, and such a report is about this program.
- Run is enabled only from the first line and only while `GrblComm::GetStatusCode()` is `Status_OK`: after `error:N` the command gate stays closed until Stop/Reset/Unlock, and the first line of the next program would fail with the old error.
- The selector tells what it shows: blue for a program in memory, red for a streamed one; filled for the executed line, a 2 pixel frame for the line to be sent next(`LINE_SENT` while running). `ProgramSender` sets it only when something changes, through `TextBox::SetSelectorColor()`/`SetSelectorFill()`: `ResetSelector()` before new text is shown, the fill when Run is pressed and when the program ends normally. The text box itself no longer picks the color.
- A streamed program is open **twice**: `SDFile` is read for sending, `p_disp_file` for the text box. Both only move forward, so there are no seeks. The second `FIL` is allocated while such a program is open - it holds a 512 byte sector buffer. A read error on it ends the following of lines and nothing else; a read error on `SDFile` stops the program. `CloseFiles()` closes both - use it, a file left open keeps one of the two `_FS_LOCK` slots.

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

- **RAM is full.** `.data` + `.bss`(the 63768 byte FreeRTOS heap is in it) take all but about 100 bytes of the 128 kB. The heap got that big because the USB CDC buffers(`APP_RX_DATA_SIZE`/`APP_TX_DATA_SIZE`) are 64 bytes: debug output over USB sends from its own buffers. Static RAM freed later should go to `configTOTAL_HEAP_SIZE` - programs are loaded into the largest free heap block. A new static member of a few hundred bytes fails at link time with `region RAM overflowed`. Allocate what is needed only in some mode(`new(std::nothrow)`), or take it from `configTOTAL_HEAP_SIZE`.
- **Bump the version** in `Application/Version.h` (`VERSION_MAJOR/MINOR/BUILD`) for a release build — there's a literal "DON'T FORGET TO CHANGE IT" note there.
- **`Src/` and `Inc/` are CubeMX-generated** from `SmartPendant.ioc`. Regenerating overwrites HAL init / peripheral config — hand edits there are fragile. Application logic belongs in `Application/`.
- **Allocations that may fail must use `new(std::nothrow)`.** DevCore overrides global `operator new` to call `Break()` on failure, which is `bkpt #0` — a hard fault on a unit with no debugger attached. `ProgramSender` deliberately allocates the largest free block and falls back to line-by-line streaming when it can't, which only works with the nothrow form.
- Robustness matters: this parses live, sometimes noisy, UART data and reads arbitrary SD card filenames — guard string parsing (`strchr`/length math) and array bounds; malformed input must not fault.
- G-code lines are limited to **80 characters** (`TextBox::MAX_LINE_LEN`). Programs are checked at load; a longer line means the file is refused, not truncated mid-run.
- **Convert between um and 0.0001 inch only through `GrblComm::ConvertMetricToImperial()` / `ConvertImperialToMetric()`** or the `ConvertMetricToUnits()` / `ConvertUnitsToMetric()` / `ConvertUnitsToImperial()` wrappers. They round to nearest. An open-coded `* 100 / 254` truncates, and a value stored in um then comes back one count lower every time it is shown and saved in imperial.
- The status watchdog sets `grbl_state = UNKNOWN` after 300 ms without a report, but does **not** clear `grbl_mpgMode`, so `IsInControl()` stays true after a link loss.
- **Command completion must fail closed.** Watchdog recovery preserves `send_id`
  and publishes `Status_Comm_Error`; unsolicited late `ok` cannot clear it.
  ProgramSender advances only on `Status_OK`. `IsStatusReceivedAfterCmd()` is a
  freshness check, not a result check - callers test `Status_OK` themselves - and
  see `abort_id` below for which superseded IDs it answers for. Fresh means the
  last received status was **requested** after the command, not only received
  after it: grblHAL can answer a request sent before the command with a report
  made right after its `ok` and before the motion is started, with state still
  Idle. `status_req_timestamp` is the request time of the last *received*
  status, so the answer doesn't go back to false when the next status is
  requested. Publish pending
  state only after UART acceptance:
  framed writes can return `ERR_UART_BUSY` while another channel is available.
  A non-OK status closes the command gate in `ProcessMessage()`. It reopens only
  through the Stop/Reset/Unlock triple (status OK, pending cleared,
  `AbandonCmdIds()`), through `ReleaseControl()`, or automatically for
  `Status_Comm_Error` alone when the state returns from UNKNOWN. Always advance
  `send_id` when clearing, so the failed command reads
  `Status_Next_Cmd_Executed`, never OK.
  **`abort_id` separates "superseded" from "abandoned".** A command that later
  ones have superseded reads `Status_Next_Cmd_Executed` whether they really were
  sent one after another - each only after the previous was acknowledged OK - or
  the queue was given up on. `IsStatusReceivedAfterCmd()` answers "was a status
  report requested after it was sent" for the first kind only, using
  `cmd_tx_timestamp` of the newest sent command (never earlier than this one's);
  that is how a caller that queued several commands can still ask about the
  first. IDs below `abort_id` are never reported. Keep it that way: **never write
  `send_id` outside the transmit path in `ProcessMessage()`** - move it past
  everything with `AbandonCmdIds()` (Stop/Reset/Unlock, `ReleaseControl()`, the
  `ParseState()` recovery) and drop a command that is not sent with
  `DropCmd()`; both write `abort_id` before `send_id`. **Both must be called
  with `mutex` locked** - they run in two tasks(Stop/Reset/Unlock/
  `ReleaseControl()` come from the UI task) and `abort_id` is checked and
  written in two steps. The mutex is not recursive: the callers lock it, and
  two of them hold it already(`ParseState()` runs under the lock taken in
  `PollSerial()`, the write-failure path in `ProcessMessage()` under its own).
  `IsStatusReceivedAfterCmd()` reads under the same lock. Do not add a branch that
  reads `Status_Next_Cmd_Executed` as success. Covered by
  `status_after_cmd_tests()` in `Tests/host/regression.cpp`.
  **A closed gate discards every command, not just program lines.** The `else`
  branch in `ProcessMessage()` drops the message and sets `send_id` to its ID,
  so MPG jog dies too - the handwheel turns and the machine does not move. The
  automatic reopen only fires on the UNKNOWN-to-known transition, so it covers
  the status watchdog but **not** the two paths that leave the state known:
  a failed `uart->Write()`, and `ASCII_NAK` from `FramedUart`. Those hold the
  gate shut until the operator uses Stop/Reset/Unlock. That is deliberate for
  a lost g-code line, whose delivery is genuinely uncertain; just be aware the
  only feedback is `status_str` reading "Comm Error" (`Application.cpp`
  refreshes it every tick) - `grbl_changed.error` is set in three places and
  read nowhere.

### Invariants nothing enforces

Break any of these and it still compiles.

- **`FramedUart::RESYNC_MARKER` == `GrblComm::ASCII_CAN`** (both 0x18) and **`FramedUart::CMD_LOST_MARKER` == `GrblComm::ASCII_NAK`** (both 0x15). They can't share a header — the transport must not know its reader.
- **The wire format block at the top of `FramedUart.h` is a COPY** of the plugin's `mpg_transport.h` (constants, frame type enum, CRC, frame builder). The pendant can't include a grblHAL header, so nothing checks they agree. Names are deliberately identical so the block can be diffed; expected differences are house formatting only. The host conformance suite checks the CRC and frame vectors from `PROTOCOL.md`, which catches a real divergence.
- **Statistics counters must stay `uint32_t`.** `SettingsScr` reads them from the Application task while the comm task writes them, without a lock — safe only because a 32-bit aligned word loads and stores atomically on this core.
- **General tab rows must stay contiguous from `NVM::TX_CONTROL`.** `SettingsScr` maps menu index to NVM index by fixed offset in two places; inserting a parameter mid-enum without adding the row in the matching position silently misroutes every row below it.
- **`NVM::Parameters`, the defaults array, and `SettingsScr::menu_strings[]` must stay index-for-index aligned.**
- **`GrblComm::msg_t::cmd[]` must not exceed the transport's maximum payload.** At 128 there is exactly one byte of margin. Grow it and long commands are silently dropped in framed mode — `Write()` returns `ERR_BAD_PARAMETER`, which `ProcessMessage()` does not re-queue — while plain mode keeps working.
- **`Menu` passes row 0 as `(void*)0`, which is `nullptr`.** Adding an `if(ptr != nullptr)` guard to a menu callback silently disables the first row.
