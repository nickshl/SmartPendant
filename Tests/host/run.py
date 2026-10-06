"""Build fresh sanitizer binaries from actual firmware sources with HAL/RTOS stubs."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
TESTS = ROOT / "Tests/host"
BUILD = ROOT / "build/host-tests"
BUILD.mkdir(parents=True, exist_ok=True)
stage = Path(tempfile.mkdtemp(prefix="p1-", dir=BUILD))
compiler = os.environ.get("CXX", "/usr/bin/g++")

for stub in (TESTS / "stubs").glob("*.h"):
    shutil.copy(stub, stage)
for path in ["Application/Little-C.cpp", "Application/Little-C.h",
             "Application/GrblComm.cpp", "Application/GrblComm.h",
             "Application/FramedUart.cpp", "Application/FramedUart.h",
             "DevCore/Framework/Result.h", "DevCore/Interfaces/IUart.h"]:
    text = (ROOT / path).read_text()
    # Expose state to fault injection only in the isolated host build.
    if path.endswith("GrblComm.h"):
        text = text.replace("private:", "public:")
    (stage / Path(path).name).write_text(text)

flags = [compiler, "-std=c++17", "-g", "-O1", "-fno-omit-frame-pointer",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-Wno-register", "-Wno-format",
         "-I", str(stage)]
sources = [str(stage / name) for name in ["Little-C.cpp", "GrblComm.cpp", "FramedUart.cpp"]]
# Compile the complete streaming timer against minimal UI/SD doubles; no
# streaming decisions are reimplemented in the harness.
sender = (ROOT / "Application/ProgramSender.cpp").read_text()
timer = sender[sender.index("Result ProgramSender::TimerExpired("):
               sender.index("Result ProgramSender::ProcessSpeedFeed(")]
(stage / "sender_timer.cpp").write_text('#include "sender.h"\n' + timer)
shutil.copy(TESTS / "sender.h", stage)
binary = stage / "regression"
subprocess.run(flags + sources + [str(stage / "sender_timer.cpp"), str(TESTS / "regression.cpp"),
                                  "-o", str(binary)], check=True)
env = dict(os.environ, ASAN_OPTIONS="detect_leaks=1", UBSAN_OPTIONS="halt_on_error=1")
subprocess.run([str(binary)], check=True, timeout=30, env=env)

# Compare stock script output with the version before this fix batch. Override
# BASELINE_REF when rerunning after committing the fixes.
baseline_ref = os.environ.get("BASELINE_REF", "HEAD")
baseline = stage / "baseline"
baseline.mkdir()
for name in ["Little-C.cpp", "Little-C.h"]:
    content = subprocess.check_output(["git", "show", f"{baseline_ref}:Application/{name}"], cwd=ROOT)
    (baseline / name).write_bytes(content)
old_binary = baseline / "scripts"
new_binary = stage / "scripts"
common = [str(stage / "GrblComm.cpp"), str(stage / "FramedUart.cpp"), str(TESTS / "scripts.cpp")]
subprocess.run(flags[:-2] + ["-I", str(baseline), "-I", str(stage), str(baseline / "Little-C.cpp")] + common
               + ["-o", str(old_binary)], check=True)
subprocess.run(flags + [str(stage / "Little-C.cpp")] + common + ["-o", str(new_binary)], check=True)
for script in sorted((ROOT / "Scripts").iterdir()):
    if script.suffix not in [".ms", ".ls"]:
        continue
    old = subprocess.check_output([str(old_binary), str(script)], timeout=30, env=env)
    new = subprocess.check_output([str(new_binary), str(script)], timeout=30, env=env)
    assert old == new, f"Generated output changed: {script.name}"
    print(f"Unchanged script output: {script.name} ({len(new)} bytes)", flush=True)
print("All P1 host checks passed", flush=True)
