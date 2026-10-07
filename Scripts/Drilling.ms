// Peck drilling along Z from current position: drills "Drill stepover", moves
// out to clear chips, dives fast to "Dive clearance" above the bottom.
// Stepover 0 - drill in one go. Speed 0 - spindle isn't started by the script.

// Drilling parameters: Variable name, scaler, units, min value, max value
// For enum parameters: Variable name, 0 to indicate enum, first value, ..., last value
int drill_distance = 35000; // Drill distance; 1000; mm; 0; 1000000
int drill_stepover = 10000; // Drill stepover; 1000; mm; 0; 100000
int clearance = 1000;       // Dive clearance; 1000; mm; 0; 5000
int feed = 50;              // Drill feed; 1; mm/min; 1; 1000
int speed = 0;              // Speed; 1; rpm; 0; 5000
int coolant = 0;            // Coolant; 0; Flood; Mist; None

main()
{
  // If stepover set to zero or greater than drill distance
  if((drill_stepover == 0) || (drill_stepover >  drill_distance))
  {
    // Drill all the way in one go
    drill_stepover = drill_distance;
  }

  // Drill progress and clearance - 0 by default for first step
  int drill_progress = 0;
  int drill_step_distance = drill_stepover;

  // Save modal state to restore it at the end
  println("M70; Save modal state");

  println("G91; Relative mode");
  println("G21; Metric mode");
  println("G94; Feed per minute mode");
  println("G40; Cutter compensation off");
  println("G50; Scaling off");

  // Set speed(if any)
  if(speed != 0) println("M3 S", speed);
  // Set coolant
  if(coolant == 0) println("M8"); // Flood
  if(coolant == 1) println("M7"); // Mist

  while(drill_progress < drill_distance)
  {
    // Do not move drill down first iteration
    if(drill_progress != 0)
    {
      // Output line for fast dive to drilled depth minus clearance
      println("G0 Z", printfp(-(drill_progress - clearance), 1000));
      // Start applying clearance after first iteration
      drill_step_distance = drill_stepover + clearance;
    }
    // Update progress
    drill_progress += drill_stepover;
    // Check if it is more than we need and correct it
    if(drill_progress > drill_distance)
    {
      // We shouldn't dive more than requested, we also have to rise to the starting point
      drill_progress = drill_distance;
      // Find last drill step distance
      drill_step_distance = (drill_distance % drill_stepover) + clearance;
    }
    // Drill piece
    println("G1 Z", printfp(-(drill_step_distance), 1000), " F", feed);
    // Rapid move out to clear chips
    println("G0 Z", printfp(drill_progress, 1000));
  }

  // Restore modal state. Spindle state is restored too, so stop it after that.
  println("M72; Restore modal state");

  // Stop coolant
  println("M9");
  // Stop spindle
  println("M5");
}
