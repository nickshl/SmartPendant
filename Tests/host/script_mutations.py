#!/usr/bin/env python3
"""Proves that script_checks.py is able to fail.

Each mutation below puts one bug into a copy of Scripts/, or into the firmware
sources the runner is built from - bugs the bundled scripts really had and
bugs that an earlier version of the checks missed - and runs the check that
has to catch it. A mutation that survives means the check lost its teeth. Run
this after editing script_checks.py; see README.md.

    python3 Tests/host/script_mutations.py           every mutation against its check
    python3 Tests/host/script_mutations.py -k facing  only mutations whose script or check name contains "facing"
    python3 Tests/host/script_mutations.py -a         also list every other check that notices(slow)
"""
from pathlib import Path
import argparse
import shutil
import sys
import tempfile

sys.dont_write_bytecode = True # keep __pycache__ out of the source tree
import script_checks as checks

# File, text to find(must be there exactly once unless a count is given),
# its replacement, check that must fail, what the bug does. A name without a
# directory is a script in Scripts/; a path is a firmware source: the runner
# is rebuilt with it changed, which takes about ten seconds.
MUTATIONS = [
    ("Application/GrblComm.h", 'return ((metric * 100 + ((metric >= 0) ? 127 : -127)) / 254);', 'return ((metric * 100) / 254);',
     "unit_getters", "um to 0.0001 inch conversion truncates"),
    ("Application/GrblComm.cpp", '(grbl_position[axis] - grbl_offset[axis]).ToFixedPoint(', '(grbl_position[axis] + grbl_offset[axis]).ToFixedPoint(',
     "work_offset", "work offset added instead of subtracted"),
    ("Application/GrblComm.cpp", '    val.FromString(data);', '    val = (float)atof(data);',
     "position_reading", "position goes through a float: long numbers lose their last digits"),
    ("DevCore/Math/Decimal32.h", '      if(own_scal != scal)\n', '      if(false)\n',
     "position_reading", "decimals aren't counted: 12.7 is read as 0.127"),
    ("DevCore/Math/Decimal32.h", 'val = (val * (int64_t)scal) / (int64_t)own_scal;', 'val = (val * (int64_t)scal + (int64_t)own_scal - 1) / (int64_t)own_scal;',
     "position_reading", "extra decimals are rounded up instead of cut"),
    ("Application/Little-C.cpp", 'ConvertUnitsToMetric(GrblComm::GetInstance().GetAxisPosition(GrblComm::AXIS_Y))', 'ConvertUnitsToMetric(GrblComm::GetInstance().GetAxisPosition(GrblComm::AXIS_X))',
     "unit_getters", "GetMetricAxisPosY() reads the X axis"),
    ("Application/Little-C.cpp", 'if(partial_data.value == 0) result = sntx_err(DIV_BY_ZERO, result);\n          else data.value /= partial_data.value;', 'data.value /= partial_data.value;',
     "bad_parameters", "division by zero isn't caught by the interpreter"),
    ("Application/Little-C.h", '#define TOKEN_BUDGET 250000', '#define TOKEN_BUDGET 25000',
     "large_jobs", "token budget too small for a real job"),
    ("Application/GCodeGeneratorScr.cpp", 'for(uint32_t i = 0u; i < n; i++)', 'for(uint32_t i = 0u; i < n + 1u; i++)',
     "menu_parser", "menu comment parser writes one byte past its buffer"),
    ("Drilling.ms", 'println("G94; Feed per minute mode");', '',
     "structure", "feed mode left as the controller had it"),
    ("Drilling.ms", 'drill_step_distance = (drill_distance % stepover) + clearance;', 'drill_step_distance = stepover + clearance;',
     "drilling", "last peck is a full one: hole deeper than asked"),
    ("Drilling.ms", 'println("G0 Z", printfp(drill_progress, 1000));', 'println("G0 Z", printfp(drill_progress - 1, 1000));',
     "drilling", "retract is 1 um short of the start"),
    ("Drilling.ms", 'println("G0 Z", printfp(-(drill_progress - clearance), 1000));', 'println("G0 Z", printfp(-drill_progress, 1000));',
     "drilling", "rapid down to the bottom of the hole, no clearance"),
    ("Drilling.ms", 'int speed = 0; ', 'int speed = 1000; ',
     "menu_fields", "spindle starts by default"),
    ("Drilling.ms", '// Dive clearance; 1000; mm; 0; 5000', '// Dive clearance; 1000; mm; 0',
     "menu_fields", "comment without the maximum"),
    ("Drilling.ms", '" F", feed);', '" F0");',
     "structure", "zero feed: grblHAL stops with error 22"),
    ("Drilling.ms", '" F", feed);', '" F", feed * 10);',
     "drilling", "ten times the feed"),
    ("Drilling.ms", 'if(coolant == 0) println("M8");', 'if(coolant == 0) println("M7");',
     "structure", "mist instead of flood"),
    ("Drilling.ms", 'println("M3 S", speed);', 'println("M4 S", speed);',
     "structure", "spindle runs backwards"),
    ("Drilling.ms", '" F", feed);', '" F", speed ? speed : feed);',
     "drilling", "feed taken from Speed when Speed is set"),
    ("Drilling.ms", '    println("G0 Z", printfp(drill_progress, 1000));', '    if(drill_progress == drill_distance) println("M5");\n    println("G0 Z", printfp(drill_progress, 1000));',
     "structure", "spindle stopped at the bottom of the hole"),
    ("Drilling.ms", '    stepover = drill_distance;\n', '    stepover = drill_distance;\n    drill_stepover = drill_distance;\n',
     "parameters_kept", "stepover 0 is the drill distance in the menu after Generate"),
    ("Drilling.ms", '// Drill distance; 1000; mm;', '// Drill distance; 100; mm;',
     "menu_fields", "menu shows 350.00 mm for a distance of 35 mm"),
    ("Drilling.ms", '// Drill feed; 1; mm/min; 1; 1000', '// Drill feed; 1; mm/min; 0; 1000',
     "menu_limits", "menu lets the feed be 0"),
    ("Enlarging.ms", '  return result;', '  return feed * 1000;',
     "enlarging", "no feed compensation: tool edge moves faster than Feed"),
    ("Enlarging.ms", '(enlarge_progress + stepover * 2) < hole_diameter_corrected', '(enlarge_progress + stepover) < hole_diameter_corrected',
     "enlarging", "spiral runs past the hole diameter"),
    ("Enlarging.ms", 'println("G17; XY plane");', '',
     "structure", "arcs in whatever plane was active"),
    ("Enlarging.ms", 'println("G0 X", printfp(-center, 1000));', '',
     "enlarging", "tool left at the wall"),
    ("Enlarging.ms", 'println("G0 X", printfp(-center, 1000));', 'println("G1 X", printfp(-center, 1000), " F", feed * 1000);',
     "enlarging", "return to the center is a feed move at a made-up feed"),
    ("Enlarging.ms", 'if(coolant == 1) println("M7"); // Mist', 'if(coolant >= 1) println("M7"); // Mist',
     "structure", "Coolant None turns mist on"),
    ("Enlarging.ms", '(diameter < 2000) ?', '(diameter < 5000) ?',
     "enlarging", "feed overflows 32 bits at a high Feed"),
    ("Enlarging.ms", '  if(result < 1) result = 1;\n', '',
     "enlarging", "feed rounds down to F0 on small arcs"),
    ("Enlarging.ms", '    println("G17; XY plane");\n', '    println("G17; XY plane");\n    println("G92 X0 Y0");\n',
     "structure", "G92 in the program: the work offset is moved for good"),
    ("ThreadMilling.ms", 'thread_direction ? -thread_pitch : thread_pitch', 'thread_direction ? thread_pitch : -thread_pitch',
     "thread_milling", "thread of the other hand"),
    ("ThreadMilling.ms", 'println("G03 X-", printfp(helix_diameter / 2, 1000)," I-", printfp(helix_diameter / 4, 1000)', 'println("G03 X-", printfp(helix_diameter / 4, 1000)," I-", printfp(helix_diameter / 8, 1000)',
     "thread_milling", "lead-out ends off the hole center"),
    ("ThreadMilling.ms", '(thread_depth % thread_pitch) ? thread_pitch : 0', '0',
     "thread_milling", "tool comes out below or above the start"),
    ("ThreadMilling.ms", 'if(thread_direction == 0) println("G1 Z", printfp(-thread_depth, 1000), " F", feed);', 'if(thread_direction == 0) println("G0 Z", printfp(-thread_depth, 1000));',
     "thread_milling", "rapid plunge to the thread depth"),
    ("ThreadMilling.ms", 'println("G03 X", printfp(helix_diameter / 2, 1000), " Y0 I"', 'println("G02 X", printfp(helix_diameter / 2, 1000), " Y0 I"',
     "thread_milling", "lead-in meets the helix head-on"),
    ("ThreadMilling.ms", '    if(thread_direction == 0) println("G1 Z", printfp(-thread_depth, 1000), " F", feed);\n', '',
     "thread_milling", "no plunge: thread milled at the surface"),
    ("ThreadMilling.ms", '    println("G03 X", printfp(helix_diameter / 2, 1000), " Y0 I", printfp(helix_diameter / 4, 1000)," J0 F", printfp(center_feed, 1000));\n',
     '    println("G03 X", printfp(helix_diameter / 2, 1000), " Y0 I", printfp(helix_diameter / 4, 1000)," J0 F", printfp(center_feed, 1000));\n    println("G1 Z-1 F", feed);\n    println("G1 Z1 F", feed);\n',
     "thread_milling", "Z move with the tool at the thread wall"),
    ("ThreadMilling.ms", '// Thread major diameter; 1000; mm; 1000; 200000', '// Thread major diameter; 1000; mm; 1000; 1000000',
     "menu_fields", "label and the largest value fill the whole menu row"),
    ("ThreadMilling.ms", '    if(coolant == 1) println("M7"); // Mist\n', '',
     "structure", "Coolant Mist gives no coolant"),
    ("Facing.ms", 'pass < passes_cnt + 1', 'pass < passes_cnt',
     "facing_mill", "last strip isn't cut", 2),
    ("Facing.ms", '  println("G0 Z", printfp(z_safe_position, 1000));\n\n', '\n',
     "facing_mill", "tool left on the surface"),
    ("Facing.ms", 'println("G1 Z", printfp(z_position, 1000), " F", feed);', 'println("G0 Z", printfp(z_position, 1000));',
     "facing_mill", "rapid plunge to the cutting depth", 2),
    ("Facing.ms", 'println("G1 X", printfp(end_x_position, 1000), " F", feed);', 'println("G1 X", printfp(end_x_position, 1000), " F", feed * 10);',
     "facing_mill", "ten times the feed"),
    ("Facing.ms", 'int z_safe_position = z_position + z_clearance;', 'int z_safe_position = z_position + 5000;',
     "facing_mill", "Z clearance parameter ignored"),
    ("Facing.ms", 'int passes_cnt = abs(y_distance / step) + (y_distance % step ? 1 : 0);', 'int passes_cnt = abs(y_distance / (step / 2)) + (y_distance % step ? 1 : 0);',
     "facing_mill", "twice the passes, each half of Step"),
    ("Facing.ms", 'if(passes_cnt) y_position += (y_distance / passes_cnt) * pass + ((y_distance % passes_cnt) * pass) / passes_cnt;', 'if(passes_cnt) y_position += (y_distance * pass) / passes_cnt;',
     "facing_mill", "pass position overflows 32 bits on the largest area"),
    ("Facing.ms", '  println("G0 Z", printfp(z_safe_position, 1000));\n\n', '  println("G53 G0 Z", printfp(z_safe_position, 1000));\n\n',
     "structure", "last retract in machine coordinates"),
    ("Facing.ms", 'int z_position = GetMetricAxisPosZ();', 'int z_position = GetAxisPosZ();',
     "controller_independence", "position taken in controller units"),
    ("Turning.ls", 'if(!is_outside && (retract_x_position < start_x_position)) retract_x_position = start_x_position;', '',
     "turning", "boring bar retracts into the opposite wall", 2),
    ("Turning.ls", 'retract_x_position = x_position + (is_outside ? 1000 : -1000);', 'retract_x_position = x_position;',
     "turning", "rapid back along Z with the tool on the surface", 2),
    ("Turning.ls", 'rough_pass_cnt = ((cut_distance - finish_step) / rough_step + 1);', 'rough_pass_cnt = ((cut_distance - finish_step) / rough_step);',
     "turning", "passes deeper than Rough Step"),
    ("Turning.ls", 'rough_pass_distance = (cut_distance - finish_step) / rough_pass_cnt;', 'rough_pass_distance = cut_distance / rough_pass_cnt;',
     "turning", "nothing left for the finish pass"),
    ("Turning.ls", 'println("G1 X", printfp(x_position, 1000), " F", rough_feed);', 'println("G0 X", printfp(x_position, 1000));',
     "turning", "rapid to the cutting diameter"),
    ("Turning.ls", 'println("G1 Z", printfp(end_z_position, 1000), " F", finish_feed);', 'println("G1 Z", printfp(end_z_position, 1000), " F", rough_feed);',
     "turning", "finish pass at the rough feed"),
    ("Turning.ls", 'if((i == 0) && (rough_speed != 0))', 'if((i == 1) && (rough_speed != 0))',
     "turning", "first pass with the spindle not started"),
    ("Turning.ls", 'if(current_diameter != turn_diameter)', 'if(1)',
     "turning", "program generated when there is nothing to cut"),
    ("Turning.ls", 'rough_pass_cnt = ((cut_distance - finish_step) / rough_step + 1);', 'rough_pass_cnt = ((cut_distance - finish_step) / rough_step + 1) * 2;',
     "turning", "twice the rough passes, each half of Rough Step"),
    ("Turning.ls", 'retract_x_position = x_position + (is_outside ? 1000 : -1000);', 'retract_x_position = x_position + (is_outside ? 1 : -1);',
     "turning", "tool moves 0.001 mm off the surface instead of 1 mm", 2),
    ("Turning.ls", '    println("G8; Radius mode");\n', '    println("G8; Radius mode");\n    println("M70; Save modal state");\n',
     "structure", "second M70: M72 restores the state of the script, not of the user"),
    ("Turning.ls", '    if(finish_speed != 0) println("G97 M3 S", finish_speed);\n', '    if(finish_speed != 0) println("G97 M3 S", finish_speed);\n    println("G95; Feed per revolution");\n',
     "structure", "G95 before the finish pass: F60 is 60 mm per revolution"),
    ("Turning.ls", '// Rough Feed; 1; mm/min; 1; 1000', '// Finish Feed; 1; mm/min; 1; 1000',
     "menu_fields", "rough feed is labelled Finish Feed"),
    ("Turning.ls", 'println("G8; Radius mode");', 'println("G7;  Diameter mode");',
     "turning", "radius values sent in diameter mode"),
    ("Turning.ls", 'println("G97 M3 S", rough_speed);', 'println("M3 S", rough_speed);',
     "structure", "spindle started in whatever speed mode was active"),
    ("Turning.ls", 'println("G97 M3 S", finish_speed);', 'println("G96 M3 S", finish_speed);',
     "structure", "rpm sent as surface speed"),
    ("Facing.ls", 'int face_x_position = end_x_position + (is_outside ? wall_step : -wall_step);', 'int face_x_position = end_x_position - (is_outside ? wall_step : -wall_step);',
     "facing_lathe", "facing passes go past the diameter"),
    ("Facing.ls", 'if(retract_z_position > start_z_position) retract_z_position = start_z_position;', '',
     "facing_lathe", "retract from the corner goes past the start position"),
    ("Facing.ls", 'if(face_x_position != start_x_position) println("G0 Z", printfp(start_z_position, 1000));', '',
     "facing_lathe", "rapid to the wall through the material"),
    ("Facing.ls", 'for(int i = 0; i < rough_pass_cnt; i++)', 'for(int i = 0; i < rough_pass_cnt - 1; i++)',
     "facing_lathe", "last layer is a rough one and Finish Step together"),
    ("Facing.ls", 'println("G0 Z", printfp(z_position + 1000, 1000));', 'println("G0 Z", printfp(z_position, 1000));',
     "facing_lathe", "rapid back with the tool dragged along the new face", 2),
    ("Facing.ls", 'println("G1 X", printfp(face_x_position, 1000), " F", finish_feed);', 'println("G1 X", printfp(face_x_position, 1000), " F", rough_feed);',
     "facing_lathe", "finish pass at the rough feed"),
    ("Facing.ls", 'if((i == 0) && (rough_speed != 0))', 'if((i == 1) && (rough_speed != 0))',
     "facing_lathe", "first pass with the spindle not started"),
    ("Facing.ls", '      println("G0 X", printfp(retract_x_position, 1000), " Z", printfp(retract_z_position, 1000));\n', '',
     "facing_lathe", "no retract from the corner: rapid back along the finished wall"),
    ("Facing.ls", 'rough_pass_cnt = ((face_length - finish_step) / rough_step + 1);', 'rough_pass_cnt = ((face_length - finish_step) / rough_step + 1) * 2;',
     "facing_lathe", "twice the rough layers, each half of Rough Step"),
    ("Facing.ls", 'println("G0 Z", printfp(z_position + 1000, 1000));', 'println("G0 Z", printfp(z_position + 1, 1000));',
     "facing_lathe", "tool lifted 0.001 mm instead of 1 mm before the rapid back", 2),
    ("Facing.ls", '// Rough Step; 1000; mm; 10; 1000', '// Rough Step; 1000; mm; 0; 1000',
     "menu_limits", "menu lets Rough Step be 0: division by zero"),
    ("Facing.ls", 'GetMetricAxisPosX() / (IsLatheDiameterMode() ? 2 : 1)', 'GetMetricAxisPosX()',
     "controller_independence", "diameter taken for a radius"),
    ("Grooving.ls", 'int distance = pass_distance + ((i < pass_remainder) ? 1 : 0);', 'int distance = pass_distance;',
     "grooving", "groove stops short of the diameter"),
    ("Grooving.ls", 'int is_dive = is_outside ? (dive_x_position < start_x_position) : (dive_x_position > start_x_position);', 'int is_dive = 1;',
     "grooving", "first move goes away from the part"),
    ("Grooving.ls", 'int pass_cnt = cut_distance / cut_step + 1;', 'int pass_cnt = cut_distance / cut_step;',
     "grooving", "passes deeper than the step"),
    ("Grooving.ls", 'println("G1 X", printfp(current_x_position, 1000), " F", cut_feed);', 'println("G1 X", printfp(current_x_position, 1000), " F", cut_feed * 10);',
     "grooving", "ten times the feed", 2),
    ("Grooving.ls", 'if(cut_distance != 0)', 'if(1)',
     "grooving", "program generated when there is nothing to cut"),
    ("Grooving.ls", 'if((i == 0) && (cut_speed != 0))', 'if((i == 1) && (cut_speed != 0))',
     "grooving", "first pass with the spindle not started"),
    ("Grooving.ls", 'int dive_x_position = current_x_position + (is_outside ? 1000 : -1000);', 'int dive_x_position = current_x_position;',
     "grooving", "rapid to the bottom of the groove"),
    ("Grooving.ls", 'if(cut_width != 0)', 'if(cut_width > 1000)',
     "grooving", "Cut Width of 1 mm and less is ignored"),
    ("Grooving.ls", 'int pass_cnt = cut_distance / cut_step + 1;', 'int pass_cnt = (cut_distance / cut_step + 1) * 2; if(pass_cnt > cut_distance) pass_cnt = cut_distance;',
     "grooving", "twice the passes, each half of the step"),
    ("Grooving.ls", 'println("G0 Z", printfp(z_position, 1000));', 'println("G0 Z", printfp(z_position, 1000), "; Move the tool back to the initial Z location for the next cut pass..");',
     "structure", "line over 80 characters once Z is negative"),
    ("Threading.ls", 'println("G0 X", printfp(start_diameter + 1000, 1000));', 'println("G0 X", printfp(start_diameter, 1000));',
     "threading", "return along Z with the tool touching the crests"),
    ("Threading.ls", 'if((new_height - current_height) < 25) new_height = current_height + 25;', '',
     "threading", "no minimum infeed: passes that only rub"),
    ("Threading.ls", 'step * step)', 'step * step * 9)',
     "threading", "constant area passes three times Step"),
    ("Threading.ls", 'println("G1 Z", printfp(start_z_position + full_shift - shift, 1000), " F60");\n      }\n      else if', 'println("G1 Z", printfp(start_z_position + shift, 1000), " F60");\n      }\n      else if',
     "threading", "flank infeed walks out of the thread profile"),
    ("Threading.ls", 'if(infeed == 1)', 'if(infeed == 3)',
     "threading", "Flank infeed cuts radially"),
    ("Threading.ls", 'flank_side = !flank_side;', '',
     "threading", "incremental infeed stays on one flank"),
    ("Threading.ls", 'println("G1 X", printfp(current_diameter, 1000), " F60");', 'println("G0 X", printfp(current_diameter, 1000));',
     "threading", "rapid to the cutting depth"),
    ("Threading.ls", '" K", printfp(pitch, 1000));', '" K", printfp(pitch / 100 * 100, 1000));',
     "threading", "pitch rounded down to 0.1 mm"),
    ("Threading.ls", 'printfp(start_z_position - length, 1000)', 'printfp(start_z_position - length / 1000 * 1000, 1000)',
     "threading", "thread length rounded down to whole mm"),
    ("Threading.ls", 'println("G97 M3 S", speed);', 'println("G97 M3 S200");',
     "threading", "Speed parameter ignored"),
    ("Threading.ls", 'step * step)', 'step * step / 4)',
     "threading", "constant area passes half of Step"),
    ("Threading.ls", '      spring_passes_left--;', '      spring_passes_left--; spring_passes--;',
     "parameters_kept", "Spring Passes is 0 in the menu after Generate"),
    ("Threading.ls", '    println("G0 X", printfp(start_diameter + 1000, 1000));\n', '    println("G0 X", printfp(start_diameter + 1000, 1000));\n    if(current_diameter == minor_diameter) println("M5");\n',
     "structure", "spindle stopped before the spring passes"),
    ("Threading.ls", 'println("G97 M3 S", speed);', 'println("G97 M4 S", speed);',
     "structure", "spindle runs backwards"),
    ("Threading.ls", 'int start_z_position = GetMetricAxisPosZ();', 'int start_z_position = GetAxisPosZ();',
     "controller_independence", "position taken in controller units"),
    ("Threading.ls", '// Infeed; 0;', '// Thread Infeed Method Selection; 0;',
     "menu_fields", "label doesn't fit the menu line"),
]

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-a", action="store_true", help="run every check on every mutation(slow) and list the ones that fail")
    ap.add_argument("-k", metavar="NAME", help="only mutations whose script or check name contains NAME(case doesn't matter)")
    args = ap.parse_args()
    by_name = {fn.__name__: fn for fn in checks.CHECKS}
    todo = [m for m in MUTATIONS if not args.k or args.k.lower() in (m[0] + " " + m[3]).lower()]
    if not todo:
        ap.error("no mutation matches " + args.k)
    checks.build()
    original = checks.SCRIPTS
    work = Path(tempfile.mkdtemp(prefix="mutation-", dir=checks.RUNNER["dir"]))

    def failures(name):
        c = checks.run_check(by_name[name])
        del checks.FAILURES[:], checks.KNOWN_FAILURES[:]
        return c.failed

    # A check that fails on the scripts as they are can't tell anything
    names = list(by_name) if args.a else sorted({m[3] for m in todo})
    broken = [name for name in names if failures(name)]
    if broken:
        print("fails on the unchanged scripts, fix that first(script_checks.py): " + ", ".join(broken), flush=True)
        return 1

    survived = stale = 0
    runner = dict(checks.RUNNER)
    for script, old, new, expected, what, *count in todo:
        label = "%-17s %-58s" % (Path(script).name, what)
        try:
            if "/" in script:
                # Firmware source: another build, with the text replaced
                checks.build(mutate=(script, old, new))
            else:
                text = (original / script).read_text()
                if text.count(old) != (count[0] if count else 1):
                    raise LookupError("text found %d times" % text.count(old))
                shutil.rmtree(work)
                shutil.copytree(original, work)
                (work / script).write_text(text.replace(old, new))
                checks.SCRIPTS = work
        except LookupError as e:
            # The file was edited: this mutation has to be rewritten for the new text
            print(label, "STALE: %s" % e, flush=True)
            stale += 1
            continue
        caught = [name for name in ([expected] + [n for n in names if n != expected] if args.a else [expected]) if failures(name)]
        checks.SCRIPTS = original
        checks.RUNNER.update(runner)
        if expected in caught:
            print(label, "caught by " + ", ".join(caught), flush=True)
        else:
            print(label, "SURVIVED %s%s" % (expected, (", caught only by " + ", ".join(caught)) if caught else ""), flush=True)
            survived += 1
    if survived or stale:
        print("\n%d mutation(s) survived, %d stale" % (survived, stale), flush=True)
        return 1
    print("All %d mutations caught" % len(todo), flush=True)
    return 0

if __name__ == "__main__":
    sys.exit(main())
