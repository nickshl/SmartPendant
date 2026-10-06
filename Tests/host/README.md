# P1 host regression checks

Run from the repository using Python 3 and a Linux C++17 compiler:

```sh
python3 Tests/host/run.py
```

On Windows host:

```powershell
wsl  -- /usr/bin/python3 /mnt/c/Users/Vlad/Documents/work/SmartPendant/Tests/host/run.py
```

The runner compiles fresh binaries in a unique directory under
`build/host-tests/`, with AddressSanitizer and UndefinedBehaviorSanitizer.
Compilation failures stop the run; existing binaries are never reused.
Set `CXX` to override `/usr/bin/g++`.

The complete `Little-C.cpp`, `GrblComm.cpp`, and `FramedUart.cpp` are compiled
with their actual headers. HAL, RTOS, and NVM dependencies are replaced with
small doubles. Private communication state is exposed only in the copied host
header. The complete `ProgramSender::TimerExpired()` function is extracted
without changing its body and compiled against UI/SD doubles.

Covered behaviors:

- A lost command ACK followed by a successful command response, a busy next
  write, expedited real-time traffic, and a delayed ACK: retry sends the next
  command exactly once.
- Status timeout, late `ok`, hard UART write failure, and superseded command
  results cannot authorize successful progress.
- Program streaming advances only on `Status_OK`, waits while pending, and
  stops without sending another line for unknown/failed results.
- Probe coordinates update together only for complete, finite reports;
  repeated identical reports remain fresh and `:0` remains a valid no-contact
  report. Random malformed inputs exercise memory bounds with a fresh seed.
- Switch return/break/continue, case fall-through, and error propagation.
- Infinite while/for/do loops and continue-through-switch terminate with a
  budget error. A following execution receives a fresh budget.
- All nine bundled scripts generate identical output to the baseline.

The script comparison defaults to `HEAD`. To compare against the version
before these fixes after committing them:

```sh
BASELINE_REF=1f45c2e python3 Tests/host/run.py
```

Host tests do not emulate DMA timing, interrupts, task preemption, physical
buttons, or SD hardware. ARM-specific `%ld` formatting assumes 32-bit `long`;
host format warnings are suppressed and those formatting paths are not tested.
Target builds and a hardware bench check remain necessary.
