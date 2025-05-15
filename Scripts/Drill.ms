// Drilling parameters: Variable name, scaler, units, min value, max value
int drill_distance = 100000; // Drill distance; 1000; mm; -1000000; 1000000
int drill_stepover = 10000;  // Drill steopver; 1000; mm; -100000; 100000
int drill_feed = 50;         // Drill feed; 1; mm/min; 1; 1000
int clearance = 1000;        // Dive clearance; 1000; mm; 0; 5000

main()
{
  // If stepover set to zero or or greater than drill distance
  if((drill_stepover == 0) || (drill_stepover >  drill_distance))
  {
    // Drill all way in one go
    drill_stepover = drill_distance;
  }

  // Drill progress and clearance - 0 by default for first step
  int drill_progress = 0;
  int clr = 0;

  println("G91; Relative mode");
  println("G21; Metric mode");

  while(drill_progress != drill_distance)
  {
    // Do not move drill down first iteration
    if(drill_progress != 0)
    {
      // Start apply clearance after first iteration
      clr = clearance;
      // Calculate position
      int pos = drill_progress - clr;
      // Output line
      println("G0 Z", printfp(-pos, 1000));
    }
    // Update progress
    drill_progress += drill_stepover;
    // Check if it more than we need and correct it
    if(drill_progress > drill_distance) drill_progress = drill_distance;
    // Drill piece
    println("G1 Z", printfp(-(drill_stepover + clr), 1000), " F", drill_feed);
    // Rapid move out to clear chips
    println("G0 Z", printfp(drill_progress, 1000));
  }
}
