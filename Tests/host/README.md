# Host tests

Three programs, all run on a PC from the repository root. They need Python 3,
a Linux C++17 compiler(`CXX` overrides `/usr/bin/g++`) and, for `run.py`, a git
checkout with the `DevCore` submodule.

| command | what it checks | time |
|---|---|---|
| `python3 Tests/host/run.py` | interpreter, communication layer and streaming timer(P1 regression checks) | ~15 s |
| `python3 Tests/host/script_checks.py` | G-code produced by the scripts in `Scripts/` | ~30 s |
| `python3 Tests/host/script_mutations.py` | that `script_checks.py` is able to fail | 5-10 min |

Each ends with exit code 0 only if everything passed. On Windows run them
through WSL:

```powershell
wsl -- /usr/bin/python3 /mnt/c/path/to/SmartPendant/Tests/host/run.py
```

Every run compiles fresh binaries in a unique directory under
`build/host-tests/`(git-ignored). Compilation failures stop the run; existing
binaries are never reused. `script_checks.py` and `script_mutations.py`
remove their directory when they end; `run.py` leaves its `p1-*` directory.

What to run after a change:

| changed | run |
|---|---|
| a script in `Scripts/` | `script_checks.py` |
| `Little-C.*`, `GrblComm.*`, `FramedUart.*`, `ProgramSender.cpp` | `run.py` and `script_checks.py` |
| `DevCore/Math/Decimal32.h` | `run.py` and `script_checks.py` |
| menu comment parser in `GCodeGeneratorScr.cpp` | `script_checks.py` |
| `script_checks.py` itself | `script_checks.py` and `script_mutations.py` |

`sender.h` and `stubs/` are doubles of the firmware: `sender.h` mirrors the
members of `ProgramSender` that the extracted code uses, `stubs/DevCore.h`
the RTOS, the task configuration(`DevCfgUsr.h` defines) and the mutex. When
one of those changes in the firmware, change the double too, or `run.py`
doesn't compile.

## Script checks(`script_checks.py`)

Checks the bundled scripts by what the generated program does, not by what
its text was last time: the program is traced as a tool path and compared with
the cut that was asked for. An edited script keeps passing as long as it still
cuts the right thing, and a script that cuts the wrong thing fails even if
nobody saved a "good" output for those parameters.

```sh
python3 Tests/host/script_checks.py                 # all checks
python3 Tests/host/script_checks.py -k facing       # checks whose name contains "facing"
python3 Tests/host/script_checks.py -v              # every failure, not the first five of a check
python3 Tests/host/script_checks.py --grblhal-sim /path/to/grblHAL_sim
```

Output is one line per check:

```
turning                    510 cases  ok
grooving                   288 cases  3 FAILED
menu_fields                 61 cases  ok, 1 known
```

`cases` is the number of programs or menu rows looked at, `N FAILED` the
number of assertions that failed(one case can fail several), `K known` the
number of failures excused as known issues(see below). The failures follow.
Each is two lines: what is wrong, with the offending move where there is one,
and the arguments of the runner call it is about, ready to be repeated by hand
(see "Running one script"):

```
grooving: last cut isn't on the cut diameter: 2.999
    script_runner Scripts/Grooving.ls -x 3.000 -y 0.000 -z 2.000 cut_diameter=0 cut_step=100 cut_width=0 cut_feed=77 cut_speed=600
```

`check stopped by ...` means the check itself raised an exception: the runner
crashed(a sanitizer report counts), a parameter the check sets doesn't exist
any more, a program line is over 80 characters, or the program can't be
traced at all. The other checks still run. With `-k` the last line is
"Selected script checks passed": that is not the whole suite.

### What runs

`script_runner.cpp` runs one script the way the pendant does. Nothing a script
touches is a test double:

- the interpreter is the real `Little-C.cpp`;
- positions reach the script through the real `GrblComm`: the runner feeds
  `[AXS:3:XYZ]`, `$13=0|1` and a status report(`<Idle|WPos:...|D:0>`) into
  `PollSerial()`, so `$13`, diameter mode and `WPos` versus `MPos` + `WCO`
  go through the firmware's own parser and unit conversion;
- the menu is built by the comment parser taken out of `GCodeGeneratorScr.cpp`
  unchanged(`menu.h` declares the few members it needs) and formatted with the
  real `ValueToStringWithScalerAndUnits()`.

HAL, RTOS and NVM are replaced by the doubles in `stubs/`, as in `run.py`.
Two binaries are built: one with AddressSanitizer and
UndefinedBehaviorSanitizer, used for everything but the sweeps - the menu,
the limits, the common checks - and an `-O2` one for the parameter sweeps of
the single scripts, which run several thousand programs.

Like the pendant, the runner prescans the script once and then presses
Generate: global variables keep what the script did to them.

### Checks

The first twelve treat all scripts alike; the rest know one script each.

| check | asserts |
|---|---|
| `structure` | `Scripts/` holds exactly the scripts listed in `LATHE`, `MILL` and `VARIANTS`. For every script and parameter set in `VARIANTS`, and for every value of "Coolant": every line is one of `ALLOWED`(so no `G53`, `G95`, `G92`, second `M70`, ...) and stays within 80 characters with the longest coordinates; the program starts with `M70`; sets `G21`, `G94`, `G40`, `G50` and one of `G90`/`G91` before the first move(lathe also one of `G7`/`G8`, a program with arcs also `G17`), each once, none after the first move; every feed move has a positive `F` on its line; the spindle is started only by `M3 S<rpm>`(mill) or `G97 M3 S<rpm>`(lathe), with the speed parameters in their order, before the first cut, never when the parameter is 0; coolant is `M8`, `M7` or nothing as "Coolant" says, before the first cut; exactly one `M72` after the last move, followed only by `M9`(required if coolant was on) and `M5`, and neither of them earlier |
| `menu_fields` | Every parameter comment has all its fields(`Label; scaler; units; min; max` or `Label; 0; value; value; ...`) and the menu reads them back as written - a label over 23 or units over 7 characters are cut; the label has the words of the variable name(`rough_feed` is "Rough Feed"; "position" may be left out); scaler goes with the units(`mm` 1000, `mm/min`, `rpm`, `cnt` 1); default is inside min..max; a speed with minimum 0 defaults to 0; label and value fit the 32 character menu row with a space between them, for every enum value and for the minimum, default and maximum of a number |
| `menu_parser` | Comments a user may write by mistake - a label, units or enum value that is too long, missing fields, no comment - are cut to what the menu holds without writing past a buffer |
| `menu_limits` | Every number at its menu minimum and maximum(the rest at defaults) gives a program or the "job too big" message of the token budget or the output buffer; no other script error, no crash, no integer overflow, no zero feed |
| `parameters_kept` | Generate leaves every parameter as the user set it, and pressing it twice gives the same program |
| `controller_independence` | Same physical position gives the identical program when the controller reports mm or inches, radius or diameter |
| `work_offset` | Same program when it reports `MPos` + `WCO`(the grblHAL default) instead of `WPos`, in mm and in inches |
| `position_reading` | The pendant reads exactly the counts the controller printed, as `WPos` and as `MPos` + `WCO`, in mm and in inches, for numbers no binary float can hold; a number printed with fewer decimals than usual is the same number(12.7 is 12.700), one with more is cut, as `Decimal32` does everywhere |
| `unit_getters` | `GetAxisPos…()`, `GetMetricAxisPos…()`, `GetImperialAxisPos…()` for X, Y and Z, each with its own value, on a metric and on an inch controller, rounding to nearest |
| `large_jobs` | A big job fits the interpreter's token budget(`TOKEN_BUDGET`) |
| `bad_parameters` | A step of zero, which the menu can't produce, ends in a script error, not in a crash, a sanitizer report or a hang |
| `grblhal` | Optional, see below |
| `drilling` | Only Z moves; hole reaches the depth and no deeper; as many pecks as the stepover asks for, none longer; every feed move is downward at "Drill feed", whatever "Speed" is; every rapid back into the hole stops "Dive clearance" above its bottom, every retract goes to the start |
| `enlarging` | Nothing is generated if the endmill doesn't fit; the program is half circles in the asked direction and one rapid back to the center; no arc cuts wider than the stepover and there aren't more arcs than it asks for; the last two make a full circle on the hole diameter and nothing reaches further; arc feed is reduced so that the tool edge moves at "Feed", up to the largest feed, and never rounds down to 0 |
| `thread_milling` | Nothing is generated if the endmill doesn't fit; no rapids; all arcs are `G3`; full helix turns around the hole center on the right diameter, with the right lead and hand, enough of them for the depth; lead-in and lead-out are half circles between the center and the helix; arc feed is reduced for the tool center; straight moves are along Z in the hole center at "Feed"; tool returns to the start |
| `facing_mill` | As many passes as the step asks for, each from start to end at the cutting depth, at "Feed"; first and last are on the edges of the area, none is further from the previous than the step; the only other feed move is the plunge straight down from the safe height; every rapid ends at the safe height("Z clearance" above the start) and XY rapids are made there; tool ends lifted; the largest area the menu allows doesn't overflow 32 bits, in both directions |
| `turning` | Both directions. Nothing is generated at the diameter; every pass is a feed move along Z for the turn length, preceded by a feed move to its diameter; rough passes are equal, no deeper than "Rough Step" and no more than it takes; the finish pass is "Finish Step" deep and on the diameter; rough passes at "Rough Feed", finish at "Finish Feed"; after every pass the tool moves 1 mm off the surface(in a hole: not past the start X) and rapids back to start Z; rough speed is set before the first move, finish speed right before the finish pass |
| `facing_lathe` | Both directions, with and without wall pass, against a material model. Nothing is generated at the diameter; no rapid through material that is still there; layers, feeds and speeds as in `turning`; after every pass the tool backs 1 mm off the new face and rapids back to start X; face finished at the length; the wall pass dives to the diameter at start Z and runs to the face at "Finish Feed", then moves 1 mm off the corner diagonally, not past the start position |
| `grooving` | Both directions, one and two sides. Nothing is generated at the diameter; only `G0` and `G1`; as many passes as the step asks for, none deeper; every cut goes into the part at "Cut Feed" and the last is on the diameter; a rapid into the groove stops 1 mm above its bottom, a rapid out goes to the start; Z moves only with the tool at the start X; no move outside start..diameter; spindle started once, before the first move |
| `threading` | Every pass runs the thread length at the pitch and stays inside the final 60 degree profile; infeed per pass is "Step"(Constant Depth) or what removes the area of the first pass, 0.025 mm at least(Constant Area); Radial passes are on the groove center, Flank passes on one flank, Incremental passes alternate flanks; the right number of spring passes, on the center; the tool reaches cutting depth by a feed move, leaves the thread by a rapid, no rapid ends below the major diameter, and the tool is 0.5 mm clear of the crests on the way back; spindle started once at "Speed" |

The sweeps deliberately use feeds and speeds that differ from each other and
from the defaults(rough feed 111, finish feed 55 and so on), and vary the
parameters a check doesn't sweep(pitch, length, clearance) from case to case,
so that a move made at the wrong one, or a parameter that is ignored, is seen.

### Known issues

`KNOWN_ISSUES` at the top of `script_checks.py` is for things that are wrong
and not fixed yet. It is empty now. To park one: add `key: description` there
and write the assertion that trips over it as
`c.known_issue(key, condition, ...)` instead of `c.expect(condition, ...)`.
Its failures are counted as `K known` and don't fail the run. Only that
assertion is excused, so write it as narrowly as the issue allows -
everything else in the same check still counts. When it stops failing, the
run fails and says to remove the entry; an entry no check asks about fails the
run too. So a fix can't go unnoticed and the list can't go stale.

Three were found by these checks and fixed; they show what the common checks
are for:

- `position_reading`: positions were parsed into binary floats, which can't
  hold most decimals, and converted to counts by truncation: the controller
  printed `X4.035` and the pendant read 4034, `X2.5001` inch gave 25000 -
  about 1 % of mm and 6 % of inch positions, more with `MPos` + `WCO`
  (113.200 - 100.500 gave 12699). `GrblComm` now parses the text straight
  into `Decimal32` and positions are exact.
- `parameters_kept`: `Drilling.ms` assigned to its `drill_stepover`
  parameter, so a stepover of 0 came back from Generate as the drill distance
  and stayed in the menu.
- `menu_fields`: in `ThreadMilling.ms` the label "Thread major diameter" and
  the value at the old maximum, "1000.000 mm", filled the 32 character row
  with no space between them.

### Real grblHAL parser(optional)

The tracer in `script_checks.py` is this project's reading of G-code. With
`--grblhal-sim` every program is also sent to
[grblHAL Simulator](https://github.com/grblHAL/Simulator) in check mode(`$C`),
which is the real grblHAL parser. The check asserts that no line is rejected
and that `M70`/`M72` bring back the modal state the program started from. The
start state is deliberately wrong for the script: `G20 G91`, on a lathe also
`G7 G18 G96`.

```sh
git clone --recurse-submodules https://github.com/grblHAL/Simulator
cmake -S Simulator -B Simulator/build -DCMAKE_BUILD_TYPE=Release && make -C Simulator/build
python3 Tests/host/script_checks.py --grblhal-sim Simulator/build/grblHAL_sim
```

It accepts or rejects lines; it can't tell a wrong cut from a right one. A
script that sends radius values in diameter mode passes here.

Things that look like script bugs and aren't:

- The simulator has no spindle encoder, so it rejects `G33`. The check
  replaces `G33 Z.. K..` with `G1 Z.. F100`; threading moves are validated by
  the `threading` check only.
- After one rejected line grblHAL rejects every following line until it gets
  a `$` command. One bad line therefore takes the rest of the program with
  it, `M72` included - the failure shows the first rejected line only, and
  "modal state not restored" then follows from it.
- `M72` doesn't restore the motion mode(`G0`/`G1`) and does restore the
  spindle state, which is why the scripts send `M5` after it. The comparison
  ignores the motion mode and the spindle words.
- `G7`/`G8`/`G96`/`G97` are accepted only in lathe mode(`$32=2`).
- The check talks to the simulator over its telnet port(`-p`); feeding stdin
  hangs, and `grblHAL_validator` from the same build crashes on these
  programs.

### Running one script

```sh
R=$(python3 Tests/host/script_checks.py --runner)   # builds it from the current sources, ~10 s
$R Scripts/Grooving.ls -m                                 # parameter menu
$R Scripts/Grooving.ls -x 12.700 -z 2.000 cut_diameter=20000 cut_step=1000
$R Scripts/Grooving.ls -x 1.0000 -i -d cut_diameter=20000 # inch controller in diameter mode
$R Scripts/Grooving.ls -x 12.500 -w 100.5,0,-20           # MPos + WCO instead of WPos
```

`--runner` leaves the sanitizer build at `build/host-tests/script_runner`.
It is a copy: build it again after changing the firmware sources. The
arguments are what a failure prints after `script_runner`. The one line
scripts of `position_reading`, `unit_getters` and `menu_parser` are left in
`build/host-tests/` for that.

- `-x -y -z` are the **work** position exactly as the controller prints it:
  mm with three decimals, or inches with four decimals when `-i` is given; X
  is a diameter when `-d` is given. Defaults are 10, 20, 30.
- `-w X,Y,Z` is a work offset in the same units: the controller then reports
  `MPos`(work position plus offset) and `WCO`.
- `name=value` sets a parameter to the **raw integer** the script variable
  holds(`cut_diameter=20000` is 20.000 mm with scaler 1000), with no range
  check. An enum is its index.
- `-m` prints one tab separated line per parameter: variable, label,
  `num`|`enum`, scaler, units, min, max, value, and the 32 character menu
  row as the pendant draws it.
- `-g N` presses Generate N times before printing: `-g 2` is the program of
  the second press, `-m -g 1` the menu after one.
- Exit code 0: the program is on stdout. Nothing on stdout means the script
  decided there is nothing to cut. 3: script error, the interpreter's message
  is on stderr. 2: bad arguments, an unknown parameter name among them.
  Anything else is a crash; sanitizer reports exit with 1.

### Changing a script

1. Run `script_checks.py` before the change, so a failure afterwards is yours.
2. Edit the script. Parameters are global `int` variables; their order is the
   menu order and their comment is the menu row(see `CLAUDE.md`).
3. Run `script_checks.py`. Repeat a failing case with the runner and read the
   program. If the program is right and the check is wrong, see "Writing a
   check" - don't adjust the script to the check.
4. If a variable was renamed: `grep -rn old_name Tests/host` and rename it
   where it belongs to that script - `VARIANTS`, the script's own check,
   `large_jobs`, `bad_parameters`, `script_mutations.py` and the examples in
   this file. Several scripts use the same names(`rough_step`, `feed`,
   `speed`): leave the other scripts' lines alone. A name that was missed
   shows up as `check stopped by RuntimeError: script_runner refused its
   arguments`.
5. If code was rewritten, `script_mutations.py` reports `STALE` for the
   mutations that don't find their text. Rewrite them for the new code.

The common checks rely on names. Keep to them, in a new script too:

- a spindle speed parameter is named `speed` or `..._speed`, and they are
  declared in the order they are used;
- the coolant parameter is named `coolant`, with the values Flood, Mist, None;
- the label has the words of the variable name;
- a line of a kind the bundled scripts don't send yet(a dwell, say) has to
  be added to `ALLOWED`, and to `trace()` if it moves the tool.

A new script needs an entry in `LATHE` or `MILL` and in `VARIANTS` -
`structure` fails until it has them: a few parameter sets, the first one
empty for the defaults, each giving a program at X10 Y20 Z30. That puts it
under the common checks and `grblhal`. A script that reads the position
belongs in `POSITION_SCRIPTS` too. Its geometry needs a check of its own, and
that check needs mutations.

### Writing a check

A check is a function decorated with `@check` that takes a `Counter`:

```python
@check
def chamfering(c):
    """One line that says what is asserted"""
    for depth, feed in itertools.product([100, 1000, 5000], [30, 300]):
        lines = program(c, "Chamfering.ms", pos=(0, 0, 0), depth=depth, feed=feed)
        if lines is None:
            continue                      # script error, already reported
        moves = trace(lines, (0, 0, 0))
        if not c.expect(moves, "program without a move"):
            continue
        c.expect(all(m.f == feed for m in moves if m.kind == "G1"), "cut that isn't at Feed")
        c.expect(abs(moves[-1].z1) < EPS, "does not return to start:", moves[-1])
```

- `run(script, pos=, inch=, dia=, wco=, generate=, checked=, **params)`
  returns `(ok, lines, error)`; `ok` is False for a script error. `pos` is the
  work position in **um** with X as a **radius**, whatever `inch` and `dia`
  say - `run()` converts it to what such a controller would print.
  `wco="x,y,z"` is in controller units. `checked=True` uses the sanitizer
  build. A crash and a program line over 80 characters raise.
- `program(c, script, ...)` is `run()` that counts a case and reports a
  script error; it returns the lines or None.
- `trace(lines, start)` follows `G90`/`G91`, `G7`/`G8` and the motion modes.
  `start` is in um like `pos`; the `Move` objects it returns are in **mm**
  with X as a radius: `kind`(`G0`..`G3`, `G33`), `start`, `end`, `x0`..`z1`,
  `words`, `f`, `index`(line number in the program). `arc_geometry(move)`
  gives center, radii and turns of an arc.
- `c.expect(condition, text, *details)` records a failure and returns the
  condition. The runner arguments of the last `run()` are added to the
  message, so details only need what isn't there: the move, the value found.

Rules that keep the checks worth having:

- Assert the result of the cut(where material is removed, where the tool may
  not be), not the list of lines. Don't compare with stored output.
- Sweep the parameters, including the awkward ones: zero, a value that
  doesn't divide evenly, start position already at the target, both
  directions. Most bugs these scripts had showed up only off the defaults.
- An assertion over a list passes when the list is empty: `all(...)` of
  nothing is true. Assert that the passes are there before asserting things
  about them.
- State what every move must be, not only what the important ones are: a
  cutting move sent as `G0`, or at the wrong feed, is a bug too.
- Give a quantity both limits. "No pass deeper than the step" is also true
  for twice as many passes as needed; "moves off the surface" is true for
  0.001 mm.
- Don't leave a parameter at its default through a whole sweep: a script
  that ignores it passes.
- When a check fails on a script that is right, fix the model in the check
  and say in a comment why the move is fine. Never loosen a tolerance or drop
  a case to get to green.
- After adding or changing a check, add a mutation for it and run
  `script_mutations.py`.

## Mutations(`script_mutations.py`)

A check that can't fail is worse than none. `script_mutations.py` puts one
bug at a time into a copy of `Scripts/`, or into the firmware sources the
runner is built from - bugs the scripts really had, and bugs earlier versions
of the checks let through - and runs the check that has to catch it.

```sh
python3 Tests/host/script_mutations.py            # each mutation against its check
python3 Tests/host/script_mutations.py -k facing  # mutations whose file or check name contains "facing"
python3 Tests/host/script_mutations.py -a         # also list every other check that notices(half an hour)
```

- `caught by <check>` - as it should be.
- `SURVIVED` - the check doesn't see the bug. Fix the check.
- `STALE` - the text to replace isn't in the file: it was edited. Rewrite the
  mutation for the new text; don't delete it.
- `fails on the unchanged scripts` - the check is red before anything was
  mutated, so it can't tell anything. Get `script_checks.py` green first.

A mutation is a line in `MUTATIONS`: file, text to find(exactly once, or the
given count), replacement, the check that must fail, what the bug does. A
name without a directory is a script; a path such as
`Application/GrblComm.cpp` is a firmware source, and the runner is rebuilt
with it changed.

The list is a sample, not a proof. Twice an independent review tried some
forty new bugs against the checks as they were then, and both times more than
thirty got through. All of those are caught now; the next forty will find
more. When a bug gets past the checks, fix the check and add the bug here.

## P1 regression checks(`run.py`)

The complete `Little-C.cpp`, `GrblComm.cpp`, and `FramedUart.cpp` are compiled
with their actual headers, with AddressSanitizer and
UndefinedBehaviorSanitizer. HAL, RTOS, and NVM dependencies are replaced with
small doubles. Private communication state is exposed only in the copied host
header. The complete `ProgramSender::TimerExpired()` function and the line
handling functions that follow it in the file are extracted without changing
their bodies and compiled against UI/SD doubles(`sender.h`: a model of the
text box and of a file on the SD card).

Covered behaviors:

- A lost command ACK followed by a successful command response, a busy next
  write, expedited real-time traffic, and a delayed ACK: retry sends the next
  command exactly once.
- Status timeout, late `ok`, hard UART write failure, and superseded command
  results cannot authorize successful progress.
- Program streaming advances only on `Status_OK`, waits while pending, and
  stops without sending another line for unknown/failed results.
- `IsStatusReceivedAfterCmd()` answers only for a status that was requested
  after the command(and after the next one for a command that was followed by
  others), doesn't go back to false while the next status is requested, and
  never answers for commands that were dropped, stopped, reset or left behind
  by a lost link or released control(`abort_id`). The mutex double is not
  recursive, so a second lock fails the run.
- Program lines are sent without `;` comments and with `N<line>` in front,
  comments in parentheses stay; the program's own `N` words are removed. The
  selection follows `Ln:` from the status report, for a program in memory and
  for a streamed one(a simulated controller that executes three lines
  behind), never takes the number left by an earlier program, waits while the
  controller reports no number, goes to the last line when the program ends,
  and follows the line sent next when `$10` disables line numbers. An SD error
  on the text box's file position doesn't stop the program; one on the sending
  position does. Run is disabled while a controller error is pending and for
  a message shown instead of a program.
- Probe coordinates update together only for complete, finite reports;
  repeated identical reports remain fresh and `:0` remains a valid no-contact
  report. Random malformed inputs exercise memory bounds with a fresh seed.
- `Decimal32::FromString()` reports how many characters a number took and
  leaves the value alone when there is none; `ToFixedPoint()` gives exact
  counts, cuts extra decimals and clamps.
- Switch return/break/continue, case fall-through, and error propagation.
- Infinite while/for/do loops and continue-through-switch terminate with a
  budget error. A following execution receives a fresh budget.
- All bundled scripts generate identical output to the baseline.

The last item compares the interpreter of the working tree with the one of a
git revision, on the scripts of the working tree: it finds an interpreter
change that alters script output. It says nothing about whether the output is
right - that is what `script_checks.py` is for. The baseline defaults to
`HEAD`, so once an interpreter change is committed it is compared with itself;
name the commit before it to compare across the change:

```sh
BASELINE_REF=1f45c2e python3 Tests/host/run.py
```

The baseline interpreter must know every built-in the scripts call: a
`BASELINE_REF` older than the `GetMetricAxisPos…` getters fails on any script
that uses them.

## What host tests don't cover

Host tests do not emulate DMA timing, interrupts, task preemption, physical
buttons, or SD hardware. ARM-specific `%ld` formatting assumes 32-bit `long`;
host format warnings are suppressed. The paths the scripts use(`printfp()`,
menu values) print correctly on a 64-bit host and are covered by
`script_checks.py`; the rest is not tested. The output buffer on the pendant is
the largest free heap block, on the host it is 1 MB: a program that fits here
may still be too big for the device.

`script_checks.py` checks each parameter over a range with the others at a
few values, not every combination, and it knows the eight bundled scripts:
what a label promises beyond its words, or whether the cut is good for the
machine, the tool and the material, is not something it can tell. Target
builds and a hardware bench check remain necessary.
