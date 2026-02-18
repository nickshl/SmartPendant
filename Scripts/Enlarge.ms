// Enlarge parameters:  Variable name, scaler, units, min value, max value
// For enum parameters: Variable name, 0 to indicate enum, first value, ..., last value
int endmill_diameter = 6000; // Endmill diameter; 1000; mm; 0; 25400
int hole_diameter = 10000;   // Hole diameter; 1000; mm; 0; 1000000
int stepover = 1000;         // Steopver; 1000; mm; 0; 5000
int feed = 30;               // Feed; 1; mm/min; 1; 1000
int speed = 0;               // Speed; 1; rpm; 1; 5000
int direction = 0;           // Direction; 0; ClockWise; CounterClockWise
int coolant = 0;             // Coolant; 0; Flood; Mist; None

main()
{
  if(endmill_diameter < hole_diameter)
  {
    int hole_diameter_corrected = hole_diameter - endmill_diameter;

    // Clear enlarge progress and center
    int enlarge_progress = 0;
    int center = 0;

    // Switch to relative mode
    println("G91; Relative mode");
    println("G21; Metric mode");

    // Set speed for the rough passes(if any)
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
      println(direction ? "G3" : "G2", " I", printfp(center, 1000), " X", printfp(enlarge_progress, 1000), " F", (feed * enlarge_progress) / (enlarge_progress + endmill_diameter));
      // Increase diameter by stepover
      enlarge_progress += stepover;
      // Find new I value for the second arc
      center = enlarge_progress / 2;
      // Arc(half circle)
      println(direction ? "G3" : "G2", " I-", printfp(center, 1000), " X-", printfp(enlarge_progress, 1000), " F", (feed * enlarge_progress) / (enlarge_progress + endmill_diameter));
    }

    // Find diameter of last enlarge arc - we need add radius of current circle and radius of desired circle.
    enlarge_progress = (enlarge_progress / 2) + (hole_diameter_corrected / 2);
    // Find new I value for the last enlarge arc
    center = enlarge_progress / 2;
    // Last enlarge arc(half circle)
    println(direction ? "G3" : "G2", " I", printfp(center, 1000), " X", printfp(enlarge_progress, 1000), " F", (feed * enlarge_progress) / (enlarge_progress + endmill_diameter));

    // Find new I value for the final circle
    center = hole_diameter_corrected / 2;
    // Do full circle with desired diameter
    println(direction ? "G3" : "G2", " I-", printfp(center, 1000), " X-", printfp(hole_diameter_corrected, 1000), " F", (feed * hole_diameter_corrected) / (hole_diameter_corrected + endmill_diameter));
    println(direction ? "G3" : "G2", " I", printfp(center, 1000), " X", printfp(hole_diameter_corrected, 1000), " F", (feed * hole_diameter_corrected) / (hole_diameter_corrected + endmill_diameter));

    // Return to center
    println("G0 X", printfp(-center, 1000));

    // Stop coolant
    println("M9");
    // Stop spindle
    println("M5");
  }
}
