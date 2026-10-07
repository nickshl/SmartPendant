// Enlarging of existing hole with endmill from its center at current Z: spiral
// of half circles, then full circle at "Hole diameter".
// "Feed" is feed of the cutting edge: feed of tool center is reduced.
// Speed 0 - spindle isn't started by the script.

// Enlarge parameters:  Variable name, scaler, units, min value, max value
// For enum parameters: Variable name, 0 to indicate enum, first value, ..., last value
int endmill_diameter = 6000; // Endmill diameter; 1000; mm; 0; 25400
int hole_diameter = 10000;   // Hole diameter; 1000; mm; 0; 1000000
int stepover = 1000;         // Stepover; 1000; mm; 10; 5000
int feed = 30;               // Feed; 1; mm/min; 1; 1000
int speed = 0;               // Speed; 1; rpm; 0; 5000
int direction = 0;           // Direction; 0; Clockwise; Counterclockwise
int coolant = 0;             // Coolant; 0; Flood; Mist; None

// Feed for the tool center, mm/min * 1000: it moves along smaller circle than
// the cutting edge and should move slower
int center_feed(int diameter)
{
  // Formula in two forms: precision for small diameters, no overflow for large ones
  int result = (diameter < 2000) ? (feed * 1000 * diameter) / (diameter + endmill_diameter) : feed * ((diameter * 1000) / (diameter + endmill_diameter));
  // Feed can't be zero
  if(result < 1) result = 1;
  return result;
}

main()
{
  if(endmill_diameter < hole_diameter)
  {
    int hole_diameter_corrected = hole_diameter - endmill_diameter;

    // Clear enlarge progress and center
    int enlarge_progress = 0;
    int center = 0;

    // Switch to relative mode
    // Save modal state to restore it at the end
    println("M70; Save modal state");

    println("G91; Relative mode");
    println("G21; Metric mode");
    println("G94; Feed per minute mode");
    println("G40; Cutter compensation off");
    println("G50; Scaling off");
    println("G17; XY plane");

    // Set speed(if any)
    if(speed != 0) println("M3 S", speed);
    // Set coolant
    if(coolant == 0) println("M8"); // Flood
    if(coolant == 1) println("M7"); // Mist

    // Generate cycle
    while((enlarge_progress + stepover * 2) < hole_diameter_corrected)
    {
      // Increase diameter by stepover
      enlarge_progress += stepover;
      // Find new I value for the first arc
      center = enlarge_progress / 2;
      // Arc(half circle) with enlarge operation.
      println(direction ? "G3" : "G2", " I", printfp(center, 1000), " X", printfp(enlarge_progress, 1000), " F", printfp(center_feed(enlarge_progress), 1000));
      // Increase diameter by stepover
      enlarge_progress += stepover;
      // Find new I value for the second arc
      center = enlarge_progress / 2;
      // Arc(half circle)
      println(direction ? "G3" : "G2", " I-", printfp(center, 1000), " X-", printfp(enlarge_progress, 1000), " F", printfp(center_feed(enlarge_progress), 1000));
    }

    // Find diameter of last enlarge arc - we need to add radius of current circle and radius of desired circle.
    enlarge_progress = (enlarge_progress / 2) + (hole_diameter_corrected / 2);
    // Find new I value for the last enlarge arc
    center = enlarge_progress / 2;
    // Last enlarge arc(half circle)
    println(direction ? "G3" : "G2", " I", printfp(center, 1000), " X", printfp(enlarge_progress, 1000), " F", printfp(center_feed(enlarge_progress), 1000));

    // Find new I value for the final circle
    center = hole_diameter_corrected / 2;
    // Do full circle with desired diameter
    println(direction ? "G3" : "G2", " I-", printfp(center, 1000), " X-", printfp(hole_diameter_corrected, 1000), " F", printfp(center_feed(hole_diameter_corrected), 1000));
    println(direction ? "G3" : "G2", " I", printfp(center, 1000), " X", printfp(hole_diameter_corrected, 1000), " F", printfp(center_feed(hole_diameter_corrected), 1000));

    // Return to center
    println("G0 X", printfp(-center, 1000));

    // Restore modal state. Spindle state is restored too, so stop it after that.
    println("M72; Restore modal state");

    // Stop coolant
    println("M9");
    // Stop spindle
    println("M5");
  }
}
