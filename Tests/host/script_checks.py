#!/usr/bin/env python3
"""Checks of the bundled G-code scripts in Scripts/.

Every script is run through the real interpreter, the real GrblComm and the
real menu comment parser(see script_runner.cpp) and the generated program is
traced as a tool path. The checks assert what the cut must look like, not
what the output text was last time, so they keep working when a script is
edited. See README.md for how to use and extend this file.

    python3 Tests/host/script_checks.py                 all checks
    python3 Tests/host/script_checks.py -k facing        only checks whose name contains "facing"
    python3 Tests/host/script_checks.py -v               print every failure, not the first five of a check
    python3 Tests/host/script_checks.py --runner         only build script_runner and print its path
    python3 Tests/host/script_checks.py --grblhal-sim /path/to/grblHAL_sim
                                                         also feed every program to a real grblHAL parser
"""
from pathlib import Path
import argparse
import atexit
import itertools
import math
import os
import random
import re
import shutil
import signal
import socket
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
TESTS = ROOT / "Tests/host"
SCRIPTS = ROOT / "Scripts"
LATHE = ["Facing.ls", "Grooving.ls", "Threading.ls", "Turning.ls"]
MILL = ["Drilling.ms", "Enlarging.ms", "Facing.ms", "ThreadMilling.ms"]
EPS = 1e-9
TAN30 = 0.57735

# ******************************************************************************
# ***   Build   ****************************************************************
# ******************************************************************************
RUNNER = {} # "checked": runner with sanitizers, "fast": without, "dir": scratch directory of this run
ENV = dict(os.environ, ASAN_OPTIONS="detect_leaks=1", UBSAN_OPTIONS="halt_on_error=1")

def build(mutate=None):
    """Fresh binaries from the actual firmware sources, in a directory that is
    removed when this process ends. Staging is the same as in run.py: real
    sources, HAL/RTOS/NVM doubles from stubs/, GrblComm.h with its private
    state exposed. mutate is (path, old, new): that text is replaced in the
    staged copy of the file - script_mutations.py breaks the firmware with it."""
    def source(path):
        text = (ROOT / path).read_text()
        if mutate and mutate[0] == path:
            if text.count(mutate[1]) != 1:
                raise LookupError("text found %d times in %s" % (text.count(mutate[1]), path))
            text = text.replace(mutate[1], mutate[2])
        return text
    out = ROOT / "build/host-tests"
    out.mkdir(parents=True, exist_ok=True)
    stage = Path(tempfile.mkdtemp(prefix="scripts-", dir=out))
    atexit.register(shutil.rmtree, stage, ignore_errors=True)
    compiler = os.environ.get("CXX", "/usr/bin/g++")
    for stub in (TESTS / "stubs").glob("*.h"):
        shutil.copy(stub, stage)
    for path in ["Application/Little-C.cpp", "Application/Little-C.h",
                 "Application/GrblComm.cpp", "Application/GrblComm.h",
                 "Application/FramedUart.cpp", "Application/FramedUart.h",
                 "DevCore/Framework/Result.h", "DevCore/Interfaces/IUart.h",
                 "DevCore/Math/Decimal32.h"]:
        text = source(path)
        if path.endswith("GrblComm.h"):
            text = text.replace("private:", "public:")
        (stage / Path(path).name).write_text(text)
    # The menu comment parser, extracted from GCodeGeneratorScr.cpp unchanged
    gen = source("Application/GCodeGeneratorScr.cpp")
    part = gen[gen.index("bool GCodeGeneratorScr::GetGlobalVariableCommentString("):
               gen.index("// ***   Private constructor")]
    (stage / "menu_parser.cpp").write_text('#include "menu.h"\n' + part[:part.rindex("// ****")])
    shutil.copy(TESTS / "menu.h", stage)
    sources = [str(stage / n) for n in ["Little-C.cpp", "GrblComm.cpp", "FramedUart.cpp", "menu_parser.cpp"]]
    sources.append(str(TESTS / "script_runner.cpp"))
    common = [compiler, "-std=c++17", "-Wno-register", "-Wno-format", "-I", str(stage)]
    checked, fast = stage / "script_runner", stage / "script_runner_fast"
    subprocess.run(common + ["-g", "-O1", "-fno-omit-frame-pointer", "-fsanitize=address,undefined",
                             "-fno-sanitize-recover=all"] + sources + ["-o", str(checked)], check=True)
    # Parameter sweeps run thousands of programs: the same sources without sanitizers
    subprocess.run(common + ["-O2"] + sources + ["-o", str(fast)], check=True)
    RUNNER.update(checked=checked, fast=fast, dir=stage)

# ******************************************************************************
# ***   Running a script   *****************************************************
# ******************************************************************************
LAST = [""] # arguments of the last runner call: printed with a failure to rerun the case by hand

def fmt(counts, inch):
    """Counts(um, or 0.0001 inch) -> text exactly as the controller prints it"""
    scale = 10000 if inch else 1000
    return "%s%d.%0*d" % ("-" if counts < 0 else "", abs(counts) // scale, 4 if inch else 3, abs(counts) % scale)

def to_tenths(um):
    """um -> 0.0001 inch, rounded to nearest"""
    return int(math.copysign((abs(um) * 100 + 127) // 254, um))

def run(script, pos=(10000, 20000, 30000), counts=None, text=None, inch=False, dia=False, wco=None, menu=False, generate=None, checked=False, buf=None, **params):
    """Run a script: a name in Scripts/ or a Path. pos is the work position in
    um(X is a radius): it is converted to what a controller in the given mode
    would print. counts, if given, replace it: the numbers the controller
    prints, in its own units, taken as they are. text replaces both: three
    strings sent as the controller's numbers. wco is "x,y,z" in controller
    units: the controller then reports MPos and this offset instead of WPos.
    generate is how many times Generate is pressed(1, or 0 for the menu).
    Parameters are raw integers, as stored in the script variables. Returns
    (ok, list of output lines, error text); ok is False for a script error.
    A crash of the runner, a sanitizer report or a program line over 80
    characters(the pendant refuses such a program) raise."""
    x, y, z = pos
    if dia:
        x *= 2
    if inch:
        x, y, z = to_tenths(x), to_tenths(y), to_tenths(z)
    if counts is not None:
        x, y, z = counts
    path = script if isinstance(script, Path) else SCRIPTS / script
    x, y, z = text if text is not None else (fmt(x, inch), fmt(y, inch), fmt(z, inch))
    args = [str(path), "-x", x, "-y", y, "-z", z]
    args += ["-i"] if inch else []
    args += ["-d"] if dia else []
    args += ["-w", wco] if wco else []
    args += ["-m"] if menu else []
    args += ["-g", str(generate)] if generate is not None else []
    args += ["-b", str(buf)] if buf else []
    args += ["%s=%d" % kv for kv in params.items()]
    LAST[0] = " ".join(args).replace(str(ROOT) + "/", "")
    p = subprocess.run([str(RUNNER["checked" if checked else "fast"])] + args, capture_output=True, text=True, timeout=60, env=ENV)
    if p.returncode == 2:
        raise RuntimeError("script_runner refused its arguments(a parameter was renamed?): %s: %s" % (LAST[0], p.stderr.strip().splitlines()[0]))
    if p.returncode not in (0, 3):
        # Sanitizer reports end up here too: they exit with 1
        why = [l for l in p.stderr.splitlines() if "runtime error" in l or l.startswith("SUMMARY")] or p.stderr.strip().splitlines()[-1:]
        raise RuntimeError("script_runner crashed(%d): %s: %s" % (p.returncode, LAST[0], " ".join(why[:1])))
    lines = p.stdout.splitlines()
    if not menu and any(len(l) > 80 for l in lines):
        raise RuntimeError("program line longer than 80 characters: %s: %s" % (LAST[0], max(lines, key=len)))
    return p.returncode == 0, lines, p.stderr.strip()

def menu(script, **kw):
    """Parameter menu as list of dicts, see script_runner.cpp for the columns.
    Runs with sanitizers: the menu comment parser works on fixed buffers."""
    ok, lines, err = run(script, menu=True, checked=True, **kw)
    if not ok:
        raise RuntimeError("%s: %s" % (script, err))
    rows = []
    for line in lines:
        name, label, kind, scaler, units, lo, hi, value, row = line.split("\t")
        rows.append(dict(name=name, label=label, kind=kind, scaler=int(scaler), units=units,
                         min=int(lo), max=int(hi), value=int(value), row=row))
    return rows

def defaults(script):
    """Parameter values as they are without touching the menu, in menu order"""
    last = LAST[0]
    values = {r["name"]: r["value"] for r in menu(script)}
    LAST[0] = last      # a failure reported after this is about the run before it
    return values

# ******************************************************************************
# ***   Tracing a program   ****************************************************
# ******************************************************************************
class Move:
    """One motion block: positions in mm, X is a radius on a lathe. index is
    the number of its line in the program."""
    def __init__(self, kind, start, end, words, text, index):
        self.kind, self.start, self.end, self.words, self.text, self.index = kind, start, end, words, text, index
        self.f = words.get("F")
    x0 = property(lambda s: s.start[0]); y0 = property(lambda s: s.start[1]); z0 = property(lambda s: s.start[2])
    x1 = property(lambda s: s.end[0]); y1 = property(lambda s: s.end[1]); z1 = property(lambda s: s.end[2])
    def __repr__(self):
        return "'%s'" % self.text

def code(line):
    """Line of a program without its comment"""
    return line.split(";")[0].strip()

def trace(lines, start):
    """Interpret the program the way the controller does. start is the work
    position in um(X as a radius), as in run(). Returns the list of moves,
    which are in mm."""
    x, y, z = (v / 1000.0 for v in start)
    relative = diameter = False
    mode = None
    moves = []
    for index, text in enumerate(lines):
        block = code(text)
        if not block:
            continue
        gcodes = [float(v) for v in re.findall(r"G(\d+(?:\.\d+)?)", block)]
        words = {m.group(1): float(m.group(2)) for m in re.finditer(r"([XYZIJKFS])\s*(-?\d+(?:\.\d+)?)", block)}
        if 20 in gcodes:
            raise ValueError("the tracer is metric only: " + text)
        if 90 in gcodes: relative = False
        if 91 in gcodes: relative = True
        if 7 in gcodes: diameter = True
        if 8 in gcodes: diameter = False
        for g in (0, 1, 2, 3, 33):
            if g in gcodes:
                mode = "G%d" % g
        if not any(k in words for k in "XYZ"):
            continue
        if not mode:
            raise ValueError("axis word without motion mode: " + text)
        nx, ny, nz = x, y, z
        if "X" in words:
            v = words["X"] / (2 if diameter else 1)
            nx = x + v if relative else v
        if "Y" in words: ny = y + words["Y"] if relative else words["Y"]
        if "Z" in words: nz = z + words["Z"] if relative else words["Z"]
        moves.append(Move(mode, (x, y, z), (nx, ny, nz), words, text, index))
        x, y, z = nx, ny, nz
    return moves

def arc_geometry(m):
    """Center, start radius, end radius and swept turns of a G2/G3 move in XY"""
    cx, cy = m.x0 + m.words.get("I", 0.0), m.y0 + m.words.get("J", 0.0)
    r0, r1 = math.hypot(m.x0 - cx, m.y0 - cy), math.hypot(m.x1 - cx, m.y1 - cy)
    a0, a1 = math.atan2(m.y0 - cy, m.x0 - cx), math.atan2(m.y1 - cy, m.x1 - cx)
    sweep = (a0 - a1) if m.kind == "G2" else (a1 - a0)
    if sweep <= 1e-9:
        sweep += 2 * math.pi
    return (cx, cy), r0, r1, sweep / (2 * math.pi)

# ******************************************************************************
# ***   Check registry   *******************************************************
# ******************************************************************************
CHECKS = []
FAILURES = []
LIMIT = 5 # failures printed per check, -v lifts it

# Assertions that fail because of something known and not fixed yet: key ->
# what is wrong. Only the assertion made with Counter.known_issue(key, ...)
# is excused, everything else in the same check still counts. When it stops
# failing, the run fails until the entry and the known_issue() call are gone.
KNOWN_ISSUES = {
}
KNOWN_SEEN = {}  # key -> how many times it failed in this run
KNOWN_FAILURES = []

def check(fn):
    CHECKS.append(fn)
    return fn

class Counter:
    """Collects failures of one check and counts the cases it looked at"""
    def __init__(self, name):
        self.name, self.cases, self.failed, self.known = name, 0, 0, 0
    def case(self):
        self.cases += 1
    def _text(self, what, details):
        return "%s: %s\n    script_runner %s" % (self.name, " ".join([what] + [str(d) for d in details]), LAST[0])
    def fail(self, what, *details):
        self.failed += 1
        if self.failed <= LIMIT:
            FAILURES.append(self._text(what, details))
    def expect(self, condition, what, *details):
        """Record a failure unless condition holds. Returns the condition."""
        if not condition:
            self.fail(what, *details)
        return condition
    def known_issue(self, key, condition, what, *details):
        """expect() for the one assertion that fails because of KNOWN_ISSUES[key]"""
        assert key in KNOWN_ISSUES
        KNOWN_SEEN.setdefault(key, 0)
        if not condition:
            KNOWN_SEEN[key] += 1
            self.known += 1
            if KNOWN_SEEN[key] <= LIMIT:
                KNOWN_FAILURES.append(self._text(what, details))
        return condition

def program(c, script, **kw):
    """Run a script that is expected to succeed and count a case; a script
    error is reported and None is returned"""
    ok, lines, err = run(script, **kw)
    c.case()
    if not ok:
        c.fail("script error:", err.splitlines()[0] if err else "")
        return None
    return lines

# ******************************************************************************
# ***   Checks common for all scripts   ****************************************
# ******************************************************************************
# Parameter sets for the checks that treat all scripts alike. The first one is
# empty: the defaults. Every set must give a program at the default position
# (X10 Y20 Z30), and a set with a rough speed must have rough passes.
VARIANTS = {
    "Drilling.ms": [{}, dict(drill_distance=12345, drill_stepover=700, coolant=1, speed=1500), dict(drill_stepover=0, coolant=2)],
    "Enlarging.ms": [{}, dict(hole_diameter=25400, stepover=2500, direction=1, feed=200, speed=3000, coolant=1), dict(hole_diameter=6100)],
    "Facing.ms": [{}, dict(end_x_position=50000, end_y_position=20000, coolant=1), dict(direction=1, end_x_position=-20000, end_y_position=45000, step=12000, speed=2000, coolant=2)],
    "ThreadMilling.ms": [{}, dict(thread_direction=1, thread_depth=5200, thread_diameter=8000, thread_pitch=1250, speed=4000, coolant=2)],
    "Facing.ls": [{}, dict(face_length=600, wall_pass=1, rough_speed=500, finish_speed=900), dict(face_diameter=30000, face_length=700, wall_pass=1),
                  dict(face_diameter=19000, finish_step=1000, wall_pass=1, finish_speed=900)],
    "Grooving.ls": [{}, dict(cut_diameter=28000, cut_step=700, cut_width=1500, cut_speed=500)],
    "Threading.ls": [{}, dict(infeed=1, type=1, depth=1074, pitch=1750, spring_passes=0), dict(infeed=2)],
    "Turning.ls": [{}, dict(turn_diameter=30000, turn_length=-4000, rough_step=500, rough_speed=400, finish_speed=1200), dict(turn_diameter=4000, finish_speed=1200)],
}

# Every line a bundled script may send, without its comment. Anything else is
# reported: a word nobody looked at(G53, G95, a second M70) changes what the
# rest of the program does, and the tracer wouldn't know. A script that needs
# a new kind of line needs it here - and in trace(), if it moves the tool.
NUMBER = r"-?\d+(?:\.\d+)?"
MODAL = ["M70", "G90", "G91", "G21", "G94", "G40", "G50", "G17", "G7", "G8"]         # once, before the first move
ALLOWED = MODAL + ["M72", "M5", "M9", "M7", "M8", r"(?:G97 )?M3 S\d+",
                   r"G0?[0123](?:\s*[XYZIJ]%s)+(?: F%s)?" % (NUMBER, NUMBER), r"G33 Z%s K%s" % (NUMBER, NUMBER)]

def positive_feed(c, lines):
    """Every feed move carries a positive F on its own line"""
    for l in lines:
        if re.match(r"G0*[123]\b", code(l)):
            f = re.search(r"F\s*(-?[\d.]+)", code(l))
            c.expect(f and float(f.group(1)) > 0, "feed move without a positive F word:", l)

@check
def structure(c):
    """Shape every generated program must have, whatever the operation"""
    on_disk = sorted(p.name for p in SCRIPTS.iterdir() if p.suffix in (".ms", ".ls"))
    c.expect(on_disk == sorted(LATHE + MILL) == sorted(VARIANTS), "scripts in Scripts/ and in LATHE, MILL, VARIANTS differ:", on_disk)
    for script, variants in VARIANTS.items():
        lathe = script.endswith(".ls")
        # Every value of the coolant enum, on top of the listed sets
        if "coolant" in defaults(script):
            variants = variants + [dict(variants[-1], coolant=v) for v in (0, 1, 2)]
        for params in variants:
            lines = program(c, script, checked=True, **params)
            if lines is None or not c.expect(lines, "empty program"):
                continue
            values = dict(defaults(script), **params)
            codes = [code(l) for l in lines]
            for l in lines:
                c.expect(any(re.fullmatch(a, code(l)) for a in ALLOWED), "line that isn't in ALLOWED:", l)
                # The pendant refuses a program with a line over 80 characters.
                # Here the coordinates are short; the longest is -1000000.000
                longest = len(l) + sum(12 - len(v) for v in re.findall(r"[XYZIJK](%s)" % NUMBER, code(l)))
                c.expect(longest <= 80, "line would be over 80 characters with the longest coordinates:", l)
            c.expect(lines[0] == "M70; Save modal state", "program must start with M70")
            c.expect(lines.count("M72; Restore modal state") == 1, "exactly one M72 expected")
            c.expect(lines[-1] == "M5", "program must end with M5")
            motion = [i for i, l in enumerate(codes) if re.match(r"G0*[0123]\b|G33\b", l)]
            feeds = [i for i in motion if not re.match(r"G0*0\b", codes[i])]
            if not c.expect(feeds, "program without a feed move"):
                continue
            first, last = motion[0], motion[-1]

            # Modal state: nothing is taken from what the controller had, and
            # nothing is changed once the tool moves
            head = codes[:first]
            modal = [l for l in codes if l in MODAL]
            c.expect(modal == [l for l in head if l in MODAL] and len(set(modal)) == len(modal), "modal word repeated or sent after the first move:", modal)
            for word in ["G21", "G94", "G40", "G50"]:
                c.expect(word in head, "modal state not set before the first move:", word)
            c.expect(("G90" in head) != ("G91" in head), "G90 or G91 must be set before the first move")
            c.expect(("G7" in head) != ("G8" in head) if lathe else not ("G7" in head or "G8" in head), "G7 or G8 must be set by a lathe script and only by it")
            if any(re.match(r"G0*[23]\b", l) for l in codes):
                c.expect("G17" in head, "arcs need G17")
            c.expect("G40" in head and "G50" in head and head.index("G40") < head.index("G50"), "G40 must come before G50")
            if lines.count("M72; Restore modal state") == 1:
                i = lines.index("M72; Restore modal state")
                c.expect(i > last, "M72 must come after the last move")
                tail = lines[i + 1:]
                c.expect(tail in (["M5"], ["M9", "M5"]), "only coolant and spindle stop may follow M72:", tail)
                c.expect(not any(l in ("M5", "M9") for l in codes[:i]), "spindle or coolant stopped before the program is over")
                if any(l in ("M7", "M8") for l in codes):
                    c.expect("M9" in tail, "coolant isn't stopped")
            positive_feed(c, lines)

            # Spindle: started with the speed parameters in their order, each
            # at most once, never when the parameter is 0
            speeds = [v for n, v in values.items() if n.endswith("speed")]
            started = []
            for i, l in enumerate(codes):
                if re.search(r"\bM0*[34]\b", l):
                    m = re.fullmatch(r"G97 M3 S(\d+)" if lathe else r"M3 S(\d+)", l)
                    if c.expect(m, "spindle must be started with", "G97 M3 S<rpm>:" if lathe else "M3 S<rpm>:", l):
                        started.append((i, int(m.group(1))))
            wanted = iter([v for v in speeds if v != 0])
            c.expect(all(v in wanted for _, v in started), "spindle speeds aren't the '...speed' parameters in their order:", [v for _, v in started], speeds)
            if speeds and speeds[-1] != 0:
                c.expect(started and started[-1][1] == speeds[-1], "last '...speed' parameter is never set")
            if speeds and speeds[0] != 0:
                c.expect(started and started[0][0] < feeds[0], "first cut is made before the spindle is started")

            # Coolant
            on = [(i, l) for i, l in enumerate(codes) if l in ("M7", "M8")]
            want = {0: ["M8"], 1: ["M7"]}.get(values.get("coolant"), [])
            c.expect([l for _, l in on] == want, "coolant doesn't follow the 'coolant' parameter(Flood, Mist, None):", [l for _, l in on], "expected", want)
            c.expect(all(i < feeds[0] for i, _ in on), "coolant must be on before the first cut")

# Scaler that goes with the units: lengths are in um, the rest are whole numbers
UNITS = {"mm": 1000, "mm/min": 1, "rpm": 1, "cnt": 1}

@check
def menu_fields(c):
    """Parameter comments parse into a menu that shows what was written"""
    for script in LATHE + MILL:
        rows = menu(script)
        base = LAST[0]
        c.expect(0 < len(rows) <= 31, "menu holds 31 parameters plus Generate")
        # What the script author wrote: name -> comment fields
        written = {m.group(1): [f.strip() for f in m.group(2).split(";")]
                   for m in re.finditer(r"^int\s+(\w+)\s*=[^\n]*?//([^\n]*)$", (SCRIPTS / script).read_text(), re.M)}
        for r in rows:
            c.case()
            LAST[0] = base
            name, enum = r["name"], r["kind"] == "enum"
            fields = written.get(name, [])
            if not c.expect(len(fields) >= 4 if enum else len(fields) == 5,
                            "comment must be 'Label; scaler; units; min; max' or 'Label; 0; value; value; ...':", name, fields):
                continue
            c.expect(r["label"] == fields[0] != "", "label is cut by the menu(23 characters at most):", name, repr(r["label"]))
            # Variable and label say the same thing: rough_feed is "Rough Feed", not "Finish Feed"
            words = set(re.findall(r"[a-z]+", r["label"].lower()))
            c.expect(all(w in words for w in name.split("_") if w != "position"), "label doesn't have the words of the variable name:", name, repr(r["label"]))
            if enum:
                texts = fields[2:]
                c.expect(fields[1] == "0" and r["max"] == len(texts) - 1, "enum values aren't read as written:", name, r["max"] + 1, texts)
                c.expect(0 <= r["value"] <= r["max"], "enum default out of range:", name)
                values = range(len(texts))
            else:
                c.expect([str(r["scaler"]), r["units"], str(r["min"]), str(r["max"])] == fields[1:],
                         "scaler, units, min, max aren't read as written(units: 7 characters at most):", name, fields[1:])
                c.expect(UNITS.get(r["units"]) == r["scaler"], "scaler doesn't go with the units, see UNITS:", name, r["scaler"], r["units"])
                c.expect(r["min"] <= r["value"] <= r["max"], "default outside min..max:", name, r["min"], r["value"], r["max"])
                values = sorted({r["min"], r["value"], r["max"]})
                if name.endswith("speed") and r["min"] == 0:
                    c.expect(r["value"] == 0, "speed with minimum 0 must default to 0:", name)
            # The row as the pendant draws it, for every value an enum has and
            # for the shortest and the longest number
            for value in values:
                row = [x for x in menu(script, **{name: value}) if x["name"] == name][0]["row"]
                fits = len(row) == 32 and row.startswith(r["label"] + " ") and (not enum or row.endswith(" " + texts[value]))
                c.expect(fits, "label and value don't fit the 32 character menu row:", name, repr(row))

@check
def menu_parser(c):
    """Comments a user may write by mistake are cut to what the menu holds:
    no crash, no write past a buffer(the runner is the sanitizer build)"""
    path = ROOT / "build/host-tests/comments.ls"
    path.write_text("int a = 1; // A label that is much longer than a menu row can ever show; 1000; millimeters; 0; 10\n"
                    "int b = 0; // B; 0; An enum value that is far too long to fit anywhere; Short\n"
                    "int c = 5; // Fields are missing\n"
                    "int d = 5;\n"
                    "int e = 1; // ;;;;;;;;\n"
                    "int f = 2; //\n"
                    "main()\n{\n}\n")
    for name, value in [("a", 1), ("b", 0), ("b", 1), ("c", 5)]:
        rows = menu(path, **{name: value})
        c.case()
        c.expect([r["name"] for r in rows] == list("abcdef"), "every global variable is a menu row:", [r["name"] for r in rows])
        c.expect(all(len(r["label"]) <= 23 and len(r["units"]) <= 7 and len(r["row"]) <= 32 for r in rows), "label, units or row longer than the menu holds")
        c.expect(rows and rows[0]["label"] == "A label that is much lo" and rows[0]["units"] == "millime", "long label and units aren't cut to 23 and 7 characters:", rows[:1])

@check
def menu_limits(c):
    """Every number the menu lets the user set, smallest and largest, gives a
    program or a 'job too big' message - never another error, a crash, an
    overflow or a zero feed"""
    for script in LATHE + MILL:
        for r in menu(script):
            if r["kind"] != "num":
                continue
            for value in (r["min"], r["max"]):
                ok, lines, err = run(script, checked=True, **{r["name"]: value})
                c.case()
                if ok:
                    positive_feed(c, lines)
                else:
                    c.expect("execution limit exceeded" in err or "doesn't fit into buffer" in err, "script error at a value the menu allows:", err.splitlines()[0] if err else "")

@check
def parameters_kept(c):
    """Generate leaves the parameters as the user set them and gives the same program the second time"""
    for script, variants in VARIANTS.items():
        for params in variants:
            before = dict(defaults(script), **params)
            after = {r["name"]: r["value"] for r in menu(script, generate=1, **params)}
            c.case()
            for name in before:
                c.expect(after.get(name) == before[name], "parameter changed by Generate(the script assigns to its global variable):",
                         name, before[name], "->", after.get(name))
            once, twice = program(c, script, checked=True, **params), program(c, script, generate=2, checked=True, **params)
            c.expect(once is not None and once == twice, "second Generate gives another program")

# Scripts that take coordinates from the start position, and start positions
# that are whole counts in both unit systems(multiples of 0.0005 inch)
POSITION_SCRIPTS = ["Facing.ls", "Grooving.ls", "Threading.ls", "Turning.ls", "Facing.ms"]
POSITIONS = [(12700, 0, 0), (19050, 6350, -3175), (5080, -25400, 1270)]

@check
def controller_independence(c):
    """Same physical position gives the same program whatever the controller
    reports: mm or inches, radius or diameter"""
    for script in POSITION_SCRIPTS:
        for pos, params in itertools.product(POSITIONS, VARIANTS[script]):
            ref = program(c, script, pos=pos, **params)
            modes = [dict(inch=True)]
            if script.endswith(".ls"):
                modes += [dict(dia=True), dict(dia=True, inch=True)]
            for mode in modes:
                other = program(c, script, pos=pos, **mode, **params)
                c.expect(ref and other == ref, "program is empty or depends on how the controller reports:", mode)

@check
def work_offset(c):
    """Same program when the controller reports machine position and work
    offset(MPos + WCO, the grblHAL default) instead of work position"""
    for script in POSITION_SCRIPTS:
        for inch, wco, positions in [(False, "100.5,-7.25,-20", [(12500, 0, 0), (19000, 6250, -3125), (4000, -25500, 1250)]),
                                     (True, "3.5,1.25,-2", [(12700, 0, 0), (19050, 6350, -3175), (50800, -25400, 3175)])]:
            for pos, params in itertools.product(positions, VARIANTS[script]):
                ref = program(c, script, pos=pos, inch=inch, **params)
                other = program(c, script, pos=pos, inch=inch, wco=wco, **params)
                c.expect(ref and other == ref, "program is empty or differs when the controller reports MPos + WCO")

def probe(c, name, getters, counts, **mode):
    """Run a one line script that prints position getters; returns what it printed as integers"""
    path = ROOT / "build/host-tests" / name     # stays there, to repeat a failure by hand
    path.write_text("main()\n{\n  println(%s);\n}\n" % getters)
    ok, lines, err = run(path, counts=counts, checked=True, **mode)
    c.case()
    if not c.expect(ok and len(lines) == 1, "getters script failed:", err):
        return []
    return [int(v) for v in lines[0].split()]

@check
def position_reading(c):
    """The pendant reads exactly the counts the controller printed, as work
    position and as machine position with work offset, and a number printed
    with fewer or more decimals than usual is still the same number"""
    getters = 'GetAxisPosX(), " ", GetAxisPosY(), " ", GetAxisPosZ()'
    # Numbers a binary float can't hold(4.035 read through one is 4034), then a spread
    samples = [(4035, -16001, 8193), (1005, 65537, -4035), (12700, 0, 0), (25001, -3, 123457), (5003, 40351, -25001)]
    samples += [(n, -n - 1, 7 * n + 3) for n in range(0, 30000, 997)]
    # More digits than a float has at all
    samples += [(123456789, -200000001, 98765432)]
    for inch, counts, wco in itertools.product((False, True), samples, (None, "100.5,-7.25,-20")):
        got = probe(c, "position.ls", getters, counts, inch=inch, wco=wco)
        c.expect(tuple(got) == counts, "position read wrong: controller printed", counts, "pendant read", got)
    # Decimals are counted, not assumed: 12.7 is 12.700. More of them than a
    # count has are cut, the way Decimal32 does it everywhere.
    for inch, text, want in [(False, ("12.7", "-3", "0.50000"), (12700, -3000, 500)), (False, ("1.2345", "-1.2345", "1.2349"), (1234, -1234, 1234)),
                             (True, ("0.5", "-2", "1.23456"), (5000, -20000, 12345)), (True, (".25", "3.", "-0.00009"), (2500, 30000, 0))]:
        got = probe(c, "position.ls", getters, None, inch=inch, text=text)
        c.expect(tuple(got) == want, "position read wrong: controller printed", text, "pendant read", got, "expected", want)

@check
def unit_getters(c):
    """GetAxisPos...(), GetMetricAxisPos...() and GetImperialAxisPos...() on a metric and on an inch controller"""
    def half_away(n, d):
        return (abs(n) * 2 + d) // (2 * d) * (1 if n >= 0 else -1)
    text = ('GetAxisPosX(), " ", GetMetricAxisPosX(), " ", GetImperialAxisPosX(), " ", GetAxisPosY(), " ", GetMetricAxisPosY(), " ", '
            'GetImperialAxisPosY(), " ", GetAxisPosZ(), " ", GetMetricAxisPosZ(), " ", GetImperialAxisPosZ()')
    values = [0, 1, 2, 3, 50, 127, 128, 254, 1000, 3937, 5000, 10000, 12700, 123456, -1, -127, -5000, -12700]
    for n in range(len(values)):
        # A different value on every axis: a getter that reads the wrong one is seen
        counts = (values[n], values[(n + 5) % len(values)], values[(n + 11) % len(values)])
        for inch in (False, True):
            got = probe(c, "getters.ls", text, counts, inch=inch)
            want = []
            for v in counts:
                want += [v, half_away(v * 254, 100) if inch else v, v if inch else half_away(v * 100, 254)]
            c.expect(got == want, "wrong getter values(as reported, um, 0.0001 inch for X, Y, Z):", got, "expected", want)

# ******************************************************************************
# ***   Mill scripts   *********************************************************
# ******************************************************************************
@check
def drilling(c):
    """Reaches the depth, never goes deeper, pecks are no longer than asked, returns to start"""
    for dist, peck, clr in itertools.product([500, 5000, 30000, 35000, 99999], [0, 700, 2000, 10000, 100000], [0, 1000, 5000]):
        lines = program(c, "Drilling.ms", pos=(0, 0, 0), drill_distance=dist, drill_stepover=peck, clearance=clr, feed=77, speed=1500)
        if lines is None:
            continue
        moves = trace(lines, (0, 0, 0))
        if not c.expect(moves, "program without a move"):
            continue
        depth = dist / 1000.0
        c.expect(abs(min(m.z1 for m in moves) + depth) < 0.0005, "depth not reached or exceeded")
        c.expect(abs(moves[-1].z1) < 0.0005, "does not return to start Z")
        c.expect(all(m.x1 == 0 and m.y1 == 0 for m in moves), "X or Y moved")
        c.expect(all(m.kind in ("G0", "G1") for m in moves), "only G0 and G1 are expected")
        new = [m for m in moves if m.kind == "G1"]
        c.expect(new and all(m.z1 < m.z0 and m.f == 77 for m in new), "feed move that isn't downward at Drill feed")
        step = (peck if 0 < peck <= dist else dist) / 1000.0
        c.expect(len(new) == -(-dist // round(step * 1000)), "wrong number of pecks:", len(new))
        deepest = 0.0
        for m in moves:
            if m.kind == "G1":
                c.expect(deepest - m.z1 <= step + 0.0005, "peck longer than stepover:", m)
                c.expect(m.z0 >= deepest - 0.0005, "feed starts below drilled depth:", m)
            elif abs(m.z0) < 0.0005:
                # From the start back into the hole: Dive clearance above the bottom
                c.expect(abs(m.z1 - (deepest + clr / 1000.0)) < 0.0005, "rapid into the hole doesn't stop Dive clearance above its bottom:", m)
            else:
                c.expect(abs(m.z1) < 0.0005, "retract that doesn't go to the start:", m)
            deepest = min(deepest, m.z1)

@check
def enlarging(c):
    """Closed spiral that ends on the hole diameter, with the edge feed kept"""
    for mill, hole in [(6000, 6000), (6000, 3100), (10000, 7000)]:
        ok, lines, _ = run("Enlarging.ms", endmill_diameter=mill, hole_diameter=hole)
        c.case(); c.expect(ok and lines == [], "nothing must be generated when endmill isn't smaller than the hole")
    cases = list(itertools.product([3000, 6000, 6350, 10000], [3100, 6100, 7000, 10000, 12345, 25400, 50001], [100, 250, 333, 1000, 2500, 5000], [0, 1], [30, 300, 1000]))
    # Smallest feed on the smallest arcs: the feed must not round down to F0
    cases += [(25400, 25500, 10, 0, 1), (25400, 25430, 10, 1, 2)]
    for mill, hole, step, cw, feed in cases:
        if mill >= hole:
            continue
        lines = program(c, "Enlarging.ms", pos=(0, 0, 0), endmill_diameter=mill, hole_diameter=hole, stepover=step, direction=cw, feed=feed)
        if lines is None:
            continue
        moves = trace(lines, (0, 0, 0))
        path = (hole - mill) / 1000.0
        arcs = moves[:-1]
        if not c.expect(len(arcs) >= 3 and all(m.kind == ("G3" if cw else "G2") for m in arcs) and moves[-1].kind == "G0",
                        "program must be arcs in the asked direction and one rapid back"):
            continue
        c.expect(abs(moves[-1].x1) < 0.0015 and abs(moves[-1].y1) < 0.0015, "does not return to the center")
        c.expect(all(m.z1 == 0 for m in moves), "Z moved")
        # One half circle more than the stepover asks for is fine, twice as many isn't
        c.expect(len(arcs) <= 2 * (-(-(hole - mill) // (2 * step))) + 3, "more arcs than the stepover asks for:", len(arcs))
        reach = 0.0; prev_reach = 0.0
        for m in arcs:
            (cx, cy), r0, r1, turns = arc_geometry(m)
            c.expect(abs(r0 - r1) < 0.0015, "arc start and end are at different radius:", m)
            c.expect(abs(turns - 0.5) < 0.01, "arc isn't a half circle:", m)
            far = math.hypot(cx, cy) + r0
            c.expect(far - prev_reach <= step / 1000.0 + 0.0015, "arc cuts wider than stepover:", m)
            prev_reach, reach = max(prev_reach, min(abs(m.x0), abs(m.x1))), max(reach, far)
            # Feed is for the cutting edge: tool center moves along a circle of
            # arc diameter d, the edge along d + endmill diameter
            d = 2 * r0 * 1000.0
            want = feed * d / (d + mill)
            c.expect(m.f and m.f > 0 and abs(m.f - want) <= max(0.02 * want, 0.0015), "wrong feed for the arc:", m, "want %.3f" % want)
        c.expect(abs(reach - path / 2) < 0.0015, "hole diameter not reached or exceeded:", "%.4f" % reach)
        last = [arc_geometry(m) for m in arcs[-2:]]
        c.expect(all(abs(r0 - path / 2) < 0.0015 and math.hypot(cx, cy) < 0.0015 for (cx, cy), r0, _, _ in last), "no full circle at hole diameter")

@check
def thread_milling(c):
    """Full helix turns at the right radius, exact depth, back to start"""
    for mill, dia in [(4000, 3000), (7900, 5000), (8000, 8000)]:
        ok, lines, _ = run("ThreadMilling.ms", endmill_diameter=mill, thread_diameter=dia)
        c.case(); c.expect(ok and lines == [], "nothing must be generated when endmill isn't smaller than the thread")
    for n, (mill, dia, pitch, depth, left) in enumerate(itertools.product([2400, 4000, 7900], [3000, 5000, 8000, 12000], [500, 700, 1250, 1750], [0, 1000, 5000, 5200, 12345], [0, 1])):
        if mill >= dia:
            continue
        feed = (100, 1000, 7)[n % 3]
        lines = program(c, "ThreadMilling.ms", pos=(0, 0, 0), endmill_diameter=mill, thread_diameter=dia, thread_pitch=pitch, thread_depth=depth, thread_direction=left, feed=feed)
        if lines is None:
            continue
        moves = trace(lines, (0, 0, 0))
        radius = (dia - mill) / 2000.0
        # No rapids: the tool is inside the hole all the time. Climb milling
        # with a right hand spindle: every arc is counterclockwise.
        if not c.expect(moves and all(m.kind in ("G1", "G3") for m in moves), "only G1 and G3 are expected"):
            continue
        c.expect(all(abs(v) < 0.0015 for v in moves[-1].end), "does not return to start:", moves[-1].end)
        c.expect(abs(min(m.z1 for m in moves) + depth / 1000.0) < 0.0015, "wrong depth")
        helix = [m for m in moves if m.kind == "G3" and m.z1 != m.z0]
        full = -(-depth // pitch)
        c.expect(len(helix) == full, "wrong number of turns:", len(helix), "expected", full)
        for m in helix:
            (cx, cy), r0, r1, turns = arc_geometry(m)
            c.expect(abs(r0 - radius) < 0.0015 and math.hypot(cx, cy) < 0.0015 and abs(turns - 1) < 0.01, "helix turn isn't a full circle around the hole center:", m)
            c.expect(abs(abs(m.z1 - m.z0) - pitch / 1000.0) < 1e-6 and (m.z1 < m.z0) == bool(left), "wrong lead of the helix:", m)
        # Lead-in from the center to the helix and lead-out back: half circles
        # that meet the helix tangentially
        leads = [m for m in moves if m.kind == "G3" and m.z1 == m.z0]
        if c.expect(len(leads) == 2, "one lead-in and one lead-out arc expected"):
            for m, (a, b) in zip(leads, [(0.0, radius), (radius, 0.0)]):
                (cx, cy), r0, r1, turns = arc_geometry(m)
                c.expect(abs(math.hypot(m.x0, m.y0) - a) < 0.0015 and abs(math.hypot(m.x1, m.y1) - b) < 0.0015 and abs(turns - 0.5) < 0.01
                         and abs(r0 - radius / 2) < 0.0015, "lead arc isn't a half circle between the center and the helix:", m)
        want = feed * (dia - mill) / dia
        for m in moves:
            if m.kind == "G3":
                c.expect(m.f and abs(m.f - want) <= 0.02 * want + 0.0015, "arc feed isn't reduced for the tool center:", m, "want %.3f" % want)
            if m.kind == "G1":
                # Up and down only in the middle of the hole, clear of the thread
                c.expect(m.f == feed and m.x0 == m.x1 and m.y0 == m.y1 and math.hypot(m.x0, m.y0) < 0.0015, "straight move must be along Z in the hole center at Feed:", m)

@check
def facing_mill(c):
    """Passes cover the area from start to end, not further apart than the step"""
    cases = list(itertools.product([0, 1], [0, 10000], [0, -5000], [50000, -33333, 10000], [20000, -77777, -5000, 9000], [333, 3000, 12000]))
    # The largest area the menu allows: position math must not overflow 32 bits
    cases += [(0, 0, -1000000, 50000, 1000000, 1500), (1, -1000000, 0, 1000000, 50000, 1500)]
    for n, (direction, sx, sy, ex, ey, step) in enumerate(cases):
        pos = (1000, 2000, -500)
        clr, feed = (5000, 20000, 1000)[n % 3], (77, 500)[n % 2]
        lines = program(c, "Facing.ms", pos=pos, direction=direction, start_x_position=sx, start_y_position=sy, end_x_position=ex, end_y_position=ey, step=step, z_clearance=clr, feed=feed)
        if lines is None:
            continue
        moves = trace(lines, pos)
        if not c.expect(moves, "program without a move"):
            continue
        cut_z, safe_z = -0.5, (clr - 500) / 1000.0
        along, across = (0, 1) if direction == 0 else (1, 0)
        a0, a1 = ((sx, ex) if direction == 0 else (sy, ey))
        b0, b1 = ((sy, ey) if direction == 0 else (sx, ex))
        width = abs(b1 - b0)
        a0, a1, b0, b1 = a0 / 1000.0, a1 / 1000.0, b0 / 1000.0, b1 / 1000.0
        cuts = []
        for m in moves:
            if m.kind == "G0":
                # Rapids lift the tool or move it at the safe height; going down is a feed move
                c.expect(abs(m.z1 - safe_z) < EPS, "rapid that doesn't end at safe Z:", m)
                if (m.x0, m.y0) != (m.x1, m.y1):
                    c.expect(abs(m.z0 - safe_z) < EPS, "rapid in XY below safe Z:", m)
            elif m.kind == "G1" and m.z0 == m.z1:
                cuts.append(m)
                c.expect(m.f == feed and m.start[across] == m.end[across] and abs(m.z1 - cut_z) < EPS and abs(m.start[along] - a0) < EPS and abs(m.end[along] - a1) < EPS,
                         "pass must go from start to end at cutting depth, at Feed:", m)
            else:
                c.expect(m.kind == "G1" and m.f == feed and (m.x0, m.y0) == (m.x1, m.y1) and abs(m.z0 - safe_z) < EPS and abs(m.z1 - cut_z) < EPS,
                         "the only other feed move is the plunge from safe Z to cutting depth:", m)
        offs = [m.start[across] for m in cuts]
        if not c.expect(cuts, "no cutting pass"):
            continue
        c.expect(abs(offs[0] - b0) < EPS and abs(offs[-1] - b1) < EPS, "first or last pass isn't on the edge of the area:", offs[:1], offs[-1:])
        gaps = [abs(q - p) for p, q in zip(offs, offs[1:])]
        c.expect(all(g <= step / 1000.0 + 0.0011 for g in gaps), "passes further apart than the step")
        c.expect(all((q - p) * (b1 - b0) > 0 for p, q in zip(offs, offs[1:])), "passes don't advance toward the end")
        c.expect(len(cuts) == -(-width // step) + 1, "wrong number of passes:", len(cuts), "expected", -(-width // step) + 1)
        c.expect(abs(moves[-1].z1 - safe_z) < EPS, "tool isn't lifted before the spindle stops")

# ******************************************************************************
# ***   Lathe scripts   ********************************************************
# ******************************************************************************
# Feeds and speeds for the lathe sweeps: all different, to tell them apart
ROUGH_FEED, FINISH_FEED, ROUGH_SPEED, FINISH_SPEED = 111, 55, 700, 900
ROUGH_FINISH = dict(rough_feed=ROUGH_FEED, finish_feed=FINISH_FEED, rough_speed=ROUGH_SPEED, finish_speed=FINISH_SPEED)

def spindle_changes(c, lines, rough_last, finish_first):
    """Rough speed is set before anything else and only if there are rough
    passes; finish speed after the last rough pass and before the finish one.
    Arguments are line numbers: last line of the rough passes(None - there are
    none) and first line of the finish pass."""
    found = [(i, l) for i, l in enumerate(lines) if "M3" in l]
    want = (["G97 M3 S%d" % ROUGH_SPEED] if rough_last is not None else []) + ["G97 M3 S%d" % FINISH_SPEED]
    if c.expect([l for _, l in found] == want, "wrong spindle commands:", [l for _, l in found], "expected", want):
        first = min(i for i, l in enumerate(lines) if re.match(r"G0*[0123]\b", l))
        c.expect(found[0][0] < first, "spindle is started after the first move")
        c.expect((rough_last if rough_last is not None else -1) < found[-1][0] < finish_first, "finish speed isn't set right before the finish pass")

def rough_and_finish(c, steps, total, rough, finish, what):
    """Depths of the passes(mm): equal rough ones no deeper than Rough Step
    and no more of them than it takes, then the finish pass: Finish Step and
    the remainder of the division, 1 um per pass at most. All arguments but
    steps are in um; total is the whole depth to remove."""
    c.expect(all(EPS < s <= rough / 1000.0 + EPS for s in steps[:-1]) and max(steps[:-1] or [0]) - min(steps[:-1] or [0]) < EPS,
             what + ": rough passes deeper than Rough Step, not deeper at all or not equal:", steps[:-1])
    count = (total - finish) // rough + 1 if total > finish else 0
    c.expect(len(steps) - 1 == count, what + ": wrong number of rough passes:", len(steps) - 1, "expected", count)
    want = min(finish, total) / 1000.0
    c.expect(want - 0.001 - EPS <= steps[-1] <= want + len(steps) * 0.001 + EPS, what + ": finish pass isn't Finish Step deep:", steps[-1], "expected", want)

@check
def turning(c):
    """Roughing steps no deeper than asked, finish pass on the diameter, safe retracts"""
    for r, dia, rough, finish, length in itertools.product([3000, 6000, 7777, 10000, 25400], [2000, 9999, 12000, 30000, 60000], [50, 250, 333, 1000], [0, 100, 300], [10000, -4000]):
        if abs(2 * r - dia) > 200 * rough:      # keep the job a realistic size
            continue
        ok, lines, err = run("Turning.ls", pos=(r, 0, 2000), turn_diameter=dia, rough_step=rough, finish_step=finish, turn_length=length, **ROUGH_FINISH)
        c.case()
        if 2 * r == dia:
            c.expect(ok and lines == [], "nothing must be generated at the diameter")
            continue
        if not c.expect(ok and lines, "script error or empty program:", err):
            continue
        moves = trace(lines, (r, 0, 2000))
        start, target, zs, ze = r / 1000.0, (dia // 2) / 1000.0, 2.0, 2.0 - length / 1000.0
        outside = start > target
        passes = [i for i, m in enumerate(moves) if m.kind == "G1" and m.z1 != m.z0]
        if not c.expect(passes and passes[0] > 0, "no cutting pass"):
            continue
        c.expect(all(m.kind in ("G0", "G1") for m in moves), "only G0 and G1 are expected")
        c.expect(all(abs(moves[i].z0 - zs) < EPS and abs(moves[i].z1 - ze) < EPS and moves[i].x0 == moves[i].x1 for i in passes), "pass isn't along Z for the turn length")
        depths = [start] + [moves[i].x0 for i in passes]
        steps = [(a - b) if outside else (b - a) for a, b in zip(depths, depths[1:])]
        c.expect(abs(depths[-1] - target) < EPS, "finish pass isn't on the turn diameter:", depths[-1], target)
        rough_and_finish(c, steps, abs(2 * r - dia) // 2, rough, finish, "turning")
        for n, i in enumerate(passes):
            feed = FINISH_FEED if n == len(passes) - 1 else ROUGH_FEED
            infeed = moves[i - 1]
            c.expect(infeed.kind == "G1" and infeed.z0 == infeed.z1 == zs, "pass isn't preceded by a feed move to its diameter at start Z:", infeed)
            c.expect(infeed.f == feed and moves[i].f == feed, "wrong feed:", infeed, moves[i], "expected", feed)
            # Off the surface by 1 mm, then back along Z. In a hole the way
            # off ends at the start position: the opposite wall may be close.
            off, back = moves[i + 1:i + 3] if i + 2 < len(moves) else (None, None)
            if c.expect(off and off.kind == back.kind == "G0" and off.z0 == off.z1 and back.x0 == back.x1 and abs(back.z1 - zs) < EPS, "pass isn't followed by a rapid off the surface and a rapid back to start Z:", off, back):
                want = moves[i].x0 + 1.0 if outside else max(moves[i].x0 - 1.0, start)
                c.expect(abs(off.x1 - want) < EPS, "tool must move 1 mm off the surface(in a hole: not past the start position):", off, "expected X%.3f" % want)
        c.expect(sum(1 for m in moves if m.z0 != m.z1) == 2 * len(passes), "Z moves that are neither a pass nor the return after it")
        end = moves[-1]
        c.expect(end.kind == "G1" and end.f == FINISH_FEED and abs(end.x1 - target) < EPS and abs(end.z1 - zs) < EPS, "program must end with a feed move to the diameter at start Z:", end)
        spindle_changes(c, lines, moves[passes[-2]].index if len(passes) > 1 else None, moves[passes[-1] - 1].index)

def facing_material(c, lines, r, dia, length, rough, finish, wall):
    """Material model for Facing.ls; arguments in um as given to the script.
    The area between start X and the face diameter is faced down layer by
    layer(floor); with a wall pass a strip of Finish Step is left at the
    diameter and cut at the end(strip). No rapid may enter material that is
    still there or leave the start..diameter range."""
    xs, xe, zs, zf = r / 1000.0, (dia // 2) / 1000.0, 2.0, 2.0 - length / 1000.0
    lo, hi = min(xs, xe), max(xs, xe)
    sign = 1 if xe > xs else -1
    left = min(finish / 1000.0, abs(xe - xs)) if wall else 0.0
    xf = xe - sign * left                       # where facing passes stop
    faced = abs(xf - xs) > EPS                  # no facing passes if the wall pass removes everything
    floor = strip = zs
    def free(px, pz):
        if not (lo - EPS <= px <= hi + EPS):
            return False
        if pz >= zs - EPS:
            return True
        in_main = faced and min(xs, xf) - EPS <= px <= max(xs, xf) + EPS
        return pz >= (floor if in_main else strip) - EPS
    moves = trace(lines, (r, 0, 2000))
    # Feed moves along X: the facing passes and, last, the dive of the wall pass
    passes = [i for i, m in enumerate(moves) if m.kind == "G1" and m.x1 != m.x0 and m.z1 == m.z0]
    dive = passes.pop() if wall and passes else None
    wall_done = False
    for i, m in enumerate(moves):
        diagonal = m.x1 != m.x0 and m.z1 != m.z0
        # A facing pass backs off the new face by 1 mm, which for the first
        # layers is in front of the start position. The retract from the
        # corner after the wall pass(the only diagonal move) must not pass it.
        c.expect(m.z1 <= zs + (EPS if diagonal else 1.0 + EPS), "move too far in front of the start position:", m)
        if m.kind == "G0":
            if not all(free(m.x0 + (m.x1 - m.x0) * k / 20, m.z0 + (m.z1 - m.z0) * k / 20) for k in range(21)):
                c.fail("rapid into material:", m)
        elif m.kind != "G1":
            c.fail("only G0 and G1 are expected:", m)
        else:
            c.expect(lo - EPS <= m.x1 <= hi + EPS and m.z1 >= zf - EPS, "feed outside the job:", m)
            if i == dive:
                c.expect(abs(m.x1 - xe) < EPS and abs(m.z0 - zs) < EPS and m.f == FINISH_FEED, "wall pass must dive to the diameter at start Z at Finish Feed:", m)
            elif i in passes:
                c.expect(faced and abs(m.x1 - xf) < EPS and abs(m.x0 - xs) < EPS, "unexpected feed along X:", m)
                floor = min(floor, m.z0)
                # Off the new face by 1 mm, then back across it
                off, back = moves[i + 1:i + 3] if i + 2 < len(moves) else (None, None)
                c.expect(off and off.kind == back.kind == "G0" and off.x0 == off.x1 and abs(off.z1 - off.z0 - 1.0) < EPS and back.z0 == back.z1 and abs(back.x1 - xs) < EPS,
                         "pass isn't followed by a rapid 1 mm off the face and a rapid back to start X:", off, back)
            elif diagonal:
                c.fail("diagonal feed move:", m)
            elif dive is not None and i == dive + 1:
                c.expect(abs(m.x0 - xe) < EPS and abs(m.z0 - zs) < EPS and abs(m.z1 - zf) < EPS and m.f == FINISH_FEED, "wall pass must run from start Z to the face at Finish Feed:", m)
                strip, wall_done = zf, True
                # Off the wall and the face at once, not past the start position
                off = moves[i + 1] if i + 1 < len(moves) else None
                want_x, want_z = max(xe - 1.0, xs) if sign > 0 else min(xe + 1.0, xs), min(zf + 1.0, zs)
                c.expect(off and off.kind == "G0" and abs(off.x1 - want_x) < EPS and abs(off.z1 - want_z) < EPS,
                         "wall pass isn't followed by a rapid 1 mm off the corner(not past the start position):", off, "expected X%.3f Z%.3f" % (want_x, want_z))
            else:
                c.expect(abs(m.x0 - xs) < EPS and m.x0 == m.x1, "unexpected feed along Z:", m)
    c.expect(wall_done == bool(wall), "wall pass missing or unexpected")
    c.expect(bool(passes) == faced, "facing passes missing or unexpected")
    if passes:
        levels = [zs] + [moves[i].z0 for i in passes]
        rough_and_finish(c, [a - b for a, b in zip(levels, levels[1:])], length, rough, finish, "facing")
        c.expect(abs(levels[-1] - zf) < EPS, "face not finished at Face Length")
        for n, i in enumerate(passes):
            feed = FINISH_FEED if n == len(passes) - 1 else ROUGH_FEED
            infeed = moves[i - 1] if i else None
            c.expect(infeed and infeed.kind == "G1" and infeed.x0 == infeed.x1 and infeed.f == feed and moves[i].f == feed, "layer isn't a feed along Z and a pass along X at its feed:", infeed, moves[i], "expected", feed)
    if moves:
        first_finish = moves[passes[-1] - 1].index if passes else moves[dive].index if dive is not None else 0
        spindle_changes(c, lines, moves[passes[-2]].index if len(passes) > 1 else None, first_finish)
        end = moves[-1].end
        c.expect(abs(end[0] - xs) < EPS and abs(end[2] - (zs if wall else zf)) < EPS, "wrong end position:", end)
        if not wall:
            c.expect(moves[-1].kind == "G1" and moves[-1].f == FINISH_FEED, "tool must come to the new face at Finish Feed:", moves[-1])

@check
def facing_lathe(c):
    """Both directions, with and without the wall pass, against the material model"""
    for r, dia, length, rough, finish, wall in itertools.product([0, 3000, 4950, 5400, 10000, 25400], [0, 10000, 30000, 61001], [0, 50, 700, 3333, 10000], [250, 1000], [0, 100, 1000], [0, 1]):
        ok, lines, err = run("Facing.ls", pos=(r, 0, 2000), face_diameter=dia, face_length=length, rough_step=rough, finish_step=finish, wall_pass=wall, **ROUGH_FINISH)
        c.case()
        if r == dia // 2:
            c.expect(ok and lines == [], "nothing must be generated at the diameter")
            continue
        if c.expect(ok and lines, "script error or empty program:", err):
            facing_material(c, lines, r, dia, length, rough, finish, wall)

@check
def grooving(c):
    """Ends exactly on the diameter, no pass deeper than the step, both directions"""
    for r, dia, step, width in itertools.product([3000, 10000, 12700, 25000], [0, 5999, 14500, 20000, 46000, 61001], [100, 300, 2000, 10000], [0, 1000, 1500]):
        if abs(r - dia // 2) > 300 * step:
            continue
        ok, lines, err = run("Grooving.ls", pos=(r, 0, 2000), cut_diameter=dia, cut_step=step, cut_width=width, cut_feed=77, cut_speed=600)
        c.case()
        if r == dia // 2:
            c.expect(ok and lines == [], "nothing must be generated at the diameter")
            continue
        if not c.expect(ok and lines, "script error or empty program:", err):
            continue
        moves = trace(lines, (r, 0, 2000))
        start, target = r / 1000.0, (dia // 2) / 1000.0
        lo, hi = min(start, target), max(start, target)
        sign = 1 if target > start else -1
        c.expect(all(m.kind in ("G0", "G1") for m in moves), "only G0 and G1 are expected")
        c.expect(all(m.f == 77 for m in moves if m.kind == "G1"), "cut that isn't at Cut Feed")
        sides = [2.0, 2.0 + width / 1000.0] if width else [2.0]
        c.expect(all(any(abs(m.z1 - z) < EPS for z in sides) for m in moves), "move to a Z that isn't a side of the groove")
        for side in sides:
            here = [m for m in moves if abs(m.z0 - side) < EPS and m.x0 != m.x1]
            cuts = [m for m in here if m.kind == "G1"]
            depths = [start] + [m.x1 for m in cuts]
            steps = [(b - a) * sign for a, b in zip(depths, depths[1:])]
            c.expect(abs(depths[-1] - target) < EPS, "last cut isn't on the cut diameter:", depths[-1])
            c.expect(steps and min(steps) > 0 and max(steps) <= step / 1000.0 + EPS, "pass deeper than the step or not deeper at all")
            c.expect(len(cuts) == abs(r - dia // 2) // step + 1, "wrong number of passes:", len(cuts), "expected", abs(r - dia // 2) // step + 1)
            # Rapid into the groove stops 1 mm above what is already cut
            depth = start
            for m in here:
                if m.kind == "G1":
                    c.expect((m.x1 - m.x0) * sign > 0 and (m.x0 - depth) * sign <= EPS, "cut that starts below the groove bottom or goes out of the part:", m)
                    depth = m.x1
                elif (m.x1 - m.x0) * sign > 0:
                    c.expect((depth - m.x1) * sign >= 1.0 - EPS, "rapid into the groove closer than 1 mm to its bottom:", m)
                else:
                    c.expect(abs(m.x1 - start) < EPS, "rapid out of the groove that doesn't go to the start:", m)
        c.expect(all(lo - EPS <= m.x1 <= hi + EPS for m in moves), "move outside start..cut diameter")
        c.expect(all(m.kind == "G0" and m.x0 == m.x1 and abs(m.x0 - start) < EPS for m in moves if m.z0 != m.z1), "Z move while tool is in the groove")
        c.expect(abs(moves[-1].x1 - start) < EPS and abs(moves[-1].z1 - 2.0) < EPS, "does not return to start")
        speed = [i for i, l in enumerate(lines) if "M3" in l]
        c.expect([lines[i] for i in speed] == ["G97 M3 S600"] and speed[0] < moves[0].index, "spindle must be started once, before the first move")

@check
def threading(c):
    """Every pass stays inside the final thread profile and the tool is clear of the thread on the way back"""
    for n, (depth, step, infeed, kind, spring, dia) in enumerate(itertools.product([150, 920, 1074, 3000], [25, 100, 300], [0, 1, 2], [0, 1], [0, 2], [0, 1])):
        pos = (10000, 0, 1500)
        pitch, length, speed = (1500, 1750, 800)[n % 3], (10000, 10500, 333)[n % 3], (200, 500)[n % 2]
        lines = program(c, "Threading.ls", pos=pos, dia=bool(dia), depth=depth, step=step, infeed=infeed, type=kind, spring_passes=spring, length=length, pitch=pitch, speed=speed)
        if lines is None:
            continue
        moves = trace(lines, pos)
        start_r, start_z, full, by = 10.0, 1.5, depth / 1000.0, step / 1000.0
        passes = [m for m in moves if m.kind == "G33"]
        if not c.expect(passes, "no threading pass"):
            continue
        c.expect(all(abs(m.z1 - (start_z - length / 1000.0)) < EPS and m.x0 == m.x1 and m.words.get("K") == pitch / 1000.0 for m in passes), "pass doesn't run the thread length at the pitch")
        c.expect(all(m.kind in ("G0", "G1", "G33") for m in moves), "only G0, G1 and G33 are expected")
        c.expect(all(m.f == 60 for m in moves if m.kind == "G1"), "positioning move that isn't at F60")
        prev = 0.0; cutting = 0
        for m in passes:
            h, shift = start_r - m.x0, m.z0 - start_z
            # Tool tip may be anywhere inside the final 60 degree groove: at
            # depth h it can sit up to (full - h) * tan(30) off the groove center
            side = (full - h) * TAN30
            c.expect(abs(shift) <= side + 0.0015, "pass cuts outside the thread profile:", m)
            c.expect(h <= full + EPS and h >= prev - EPS, "pass depth out of order:", m)
            if h > prev + EPS:
                # Constant Depth: Step. Constant Area: as much as removes the
                # area of the first pass, 0.025 mm at least. The last cutting
                # pass takes what is left.
                want = min(by if kind == 1 else max(math.sqrt(prev * prev + by * by) - prev, 0.025), full - prev)
                c.expect(abs(h - prev - want) <= 0.0011, "wrong infeed:", m, "%.4f" % (h - prev), "expected %.4f" % want)
                # The script uses 0.577 for tan(30) and whole um: allow for both
                if infeed == 0:
                    c.expect(abs(shift) < EPS, "radial infeed must not shift along Z:", m)
                else:
                    c.expect(abs(abs(shift) - side) <= 0.004, "pass isn't on the flank of the thread:", m)
                    if side > 0.004:
                        c.expect((shift > 0) == (infeed == 1 or cutting % 2 == 0), "pass is on the wrong flank:", m)
                cutting += 1
            else:
                c.expect(abs(shift) < EPS, "spring pass isn't on the groove center:", m)
            prev = h
        c.expect(abs(prev - full) < EPS and abs(passes[-1].z0 - start_z) < EPS, "last pass isn't at full depth on the groove center")
        c.expect(len(passes) - cutting == spring, "wrong number of spring passes:", len(passes) - cutting)
        for m, nxt in zip(moves, moves[1:]):
            if nxt.kind == "G33":
                c.expect(m.kind == "G1" and m.z0 == m.z1, "pass isn't preceded by a feed move to its depth:", m)
            if m.kind == "G33":
                c.expect(nxt.kind == "G0" and nxt.z0 == nxt.z1, "pass isn't followed by a rapid out of the thread:", nxt)
        for m in moves:
            if m.kind == "G0":
                c.expect(m.x1 >= start_r - EPS, "rapid below the major diameter:", m)
                if m.z0 != m.z1:
                    c.expect(m.x0 >= start_r + 0.5 - EPS and abs(m.z1 - start_z) < EPS, "return along Z isn't to start Z with the tool clear of the thread:", m)
        c.expect(abs(moves[-1].x1 - start_r) < EPS and abs(moves[-1].z1 - start_z) < EPS, "does not return to start")
        found = [i for i, l in enumerate(lines) if "M3" in l]
        c.expect([lines[i] for i in found] == ["G97 M3 S%d" % speed] and found[0] < moves[0].index, "spindle must be started once at Speed, before the first move")

# ******************************************************************************
# ***   Limits   ***************************************************************
# ******************************************************************************
@check
def large_jobs(c):
    """A big job still fits the interpreter's token budget(TOKEN_BUDGET in
    Little-C.h). The budget is not far above these: 1 um steps over a long
    distance are refused, with a message."""
    jobs = [("Drilling.ms", (0, 0, 0), dict(drill_distance=100000, drill_stepover=1000)),
            ("Enlarging.ms", (0, 0, 0), dict(hole_diameter=50000, stepover=500)),
            ("ThreadMilling.ms", (0, 0, 0), dict(thread_depth=30000, thread_pitch=500, thread_diameter=8000, endmill_diameter=6000)),
            ("Facing.ms", (0, 0, 0), dict(end_x_position=100000, end_y_position=200000, step=1000)),
            ("Grooving.ls", (50000, 0, 0), dict(cut_step=250)),
            ("Facing.ls", (50000, 0, 0), dict(face_length=20000, rough_step=100, face_diameter=0)),
            ("Turning.ls", (50000, 0, 0), dict(turn_diameter=20000, rough_step=250)),
            ("Threading.ls", (20000, 0, 0), dict(depth=5000, step=25, type=1, spring_passes=5))]
    for script, pos, params in jobs:
        lines = program(c, script, pos=pos, **params)
        if lines is not None:
            c.expect(len(lines) > 60, "job smaller than intended:", len(lines), "lines")

@check
def bad_parameters(c):
    """A step of zero, which the menu can't produce, still ends in a script
    error and not in a crash or a hang"""
    for script, params in [("Threading.ls", dict(type=1, step=0)), ("Enlarging.ms", dict(stepover=0)), ("Drilling.ms", dict(drill_stepover=-10000)),
                           ("Turning.ls", dict(rough_step=0)), ("Facing.ls", dict(rough_step=0)), ("Grooving.ls", dict(cut_step=0)),
                           ("ThreadMilling.ms", dict(thread_pitch=0)), ("Facing.ms", dict(step=0, end_y_position=50000))]:
        # A crash, a sanitizer report or a timeout raises in run()
        ok, lines, err = run(script, checked=True, buf=65536, **params)
        c.case()
        c.expect(not ok and err, "expected a script error")

# ******************************************************************************
# ***   Optional: the real grblHAL parser   ************************************
# ******************************************************************************
class Simulator:
    """grblHAL Simulator(github.com/grblHAL/Simulator) over its raw telnet port"""
    def __init__(self, binary):
        self.dir = tempfile.mkdtemp(prefix="sim-", dir=RUNNER["dir"])
        self.sock = self.proc = None
        for _ in range(5):
            port = random.randint(20000, 60000)
            self.proc = subprocess.Popen([binary, "-n", "-t", "0", "-p", str(port)], cwd=self.dir, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            for _ in range(50):
                try:
                    self.sock = socket.create_connection(("127.0.0.1", port), timeout=1)
                    break
                except OSError:
                    time.sleep(0.1)
            if self.sock:
                break
            self.proc.kill()
            self.proc.wait()
        if not self.sock:
            raise RuntimeError("can't connect to the simulator")
        self.sock.settimeout(0.25)
        self._read(1.0)
    def _read(self, wait):
        data, end = b"", time.time() + wait
        while time.time() < end:
            try:
                chunk = self.sock.recv(4096)
            except socket.timeout:
                continue
            if not chunk:
                break
            data += chunk
            text = data.decode(errors="replace").replace("\r", "")
            if text.rstrip().endswith("ok") or text.rstrip().split("\n")[-1].startswith("error:"):
                break
        return [l for l in data.decode(errors="replace").replace("\r", "").split("\n") if l]
    def send(self, line):
        self.sock.sendall((line + "\n").encode())
        # Returns as soon as the answer is there; the wait is for a busy machine
        return self._read(10.0)
    def close(self):
        self.proc.send_signal(signal.SIGKILL)
        self.proc.wait()
        shutil.rmtree(self.dir, ignore_errors=True)

def grblhal(c, binary):
    """Every line of every program is accepted by grblHAL, and M70/M72 bring
    back the modal state the program started from. The simulator has no
    spindle encoder, so G33 is replaced with G1: threading moves themselves
    are not checked here."""
    for script, variants in VARIANTS.items():
        lathe = script.endswith(".ls")
        for params in variants:
            lines = program(c, script, pos=(10000, 0, 0), **params)
            if lines is None:
                continue
            lines = [l.replace("G33", "G1").split(" K")[0] + " F100" if l.startswith("G33") else l for l in lines]
            sim = Simulator(binary)
            try:
                # A state as far from what the script needs as possible. G96
                # refuses to move with a stopped spindle, so start it.
                setup = ["$32=2", "$C", "G20", "G91", "G7", "G18", "G96 S100 M3"] if lathe else ["$32=0", "$C", "G20", "G91", "G19"]
                for line in setup:
                    sim.send(line)
                before = sim.send("$G")
                rejected = []
                for line in lines:
                    reply = sim.send(line)
                    if not reply:
                        raise RuntimeError("no answer from the simulator to: " + line)
                    if "ok" not in reply:
                        rejected.append((line, reply))
                after = sim.send("$G")
                c.expect(not rejected, "grblHAL rejected(look at the first line only, the rest follows from it):", rejected[:1])
                if c.expect(before and after and before[0].startswith("[GC:") and after[0].startswith("[GC:"), "no modal state report:", before, after):
                    # M72 restores everything but the motion mode; the spindle is stopped by the script
                    strip = lambda s: re.sub(r"\[GC:G\d+ ", "[GC:", re.sub(r" M[345] ", " ", re.sub(r" S[\d.]+\]", "]", s)))
                    c.expect(strip(before[0]) == strip(after[0]), "modal state not restored:", before[0], after[0])
                    c.expect(" M5 " in after[0], "spindle left running")
            finally:
                sim.close()

# ******************************************************************************
# ***   Main   *****************************************************************
# ******************************************************************************
def run_check(fn):
    """Run one check. An exception inside it is a failure of that check."""
    c = Counter(fn.__name__)
    try:
        fn(c)
    except Exception as e:
        c.fail("check stopped by %s: %s" % (type(e).__name__, str(e).strip().splitlines()[0] if str(e).strip() else ""))
    return c

def main():
    global LIMIT
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-k", metavar="NAME", help="run only checks whose name contains NAME")
    ap.add_argument("-v", action="store_true", help="print every failure, also of the known issues")
    ap.add_argument("--runner", action="store_true", help="build build/host-tests/script_runner from the current sources, print its path and exit")
    ap.add_argument("--grblhal-sim", metavar="PATH", help="grblHAL_sim binary: also validate programs with the real parser")
    args = ap.parse_args()
    if args.v:
        LIMIT = 1 << 30
    sim = None
    if args.grblhal_sim:
        sim = Path(args.grblhal_sim).resolve()
        if not (sim.is_file() and os.access(sim, os.X_OK)):
            ap.error("not an executable: %s" % sim)
    todo = [fn for fn in CHECKS if not args.k or args.k in fn.__name__]
    if sim and (not args.k or args.k in "grblhal"):
        todo.append(lambda c: grblhal(c, str(sim)))
        todo[-1].__name__ = "grblhal"
    if not todo and not args.runner:
        ap.error("no check matches " + args.k)
    build()
    if args.runner:
        target = ROOT / "build/host-tests/script_runner"
        shutil.copy2(RUNNER["checked"], target)
        print(target)
        return 0
    for fn in todo:
        c = run_check(fn)
        result = "%d FAILED" % c.failed if c.failed else "ok"
        print("%-24s %5d cases  %s%s" % (c.name, c.cases, result, ", %d known" % c.known if c.known else ""), flush=True)
    for key in KNOWN_ISSUES:
        if KNOWN_SEEN.get(key) == 0:
            FAILURES.append("known issue '%s' doesn't fail any more: remove it from KNOWN_ISSUES and turn its known_issue() into expect()" % key)
        elif key not in KNOWN_SEEN and not args.k:
            FAILURES.append("known issue '%s' is in KNOWN_ISSUES, but no check asks about it: remove it" % key)
    hit = [key for key, count in KNOWN_SEEN.items() if count]
    if hit:
        print("\n".join(["", "Known issues(not counted as failures):"] + ["%s(%d): %s" % (key, KNOWN_SEEN[key], KNOWN_ISSUES[key]) for key in hit]
                        + (KNOWN_FAILURES if args.v else [])), flush=True)
    if FAILURES:
        print("\n".join(["", "Failures(-v shows all of them):"] + FAILURES), flush=True)
        return 1
    if not sim:
        print("grblhal                  skipped: pass --grblhal-sim to check programs with the real grblHAL parser", flush=True)
    print("All script checks passed" if not args.k else "Selected script checks passed", flush=True)
    return 0

if __name__ == "__main__":
    sys.exit(main())
