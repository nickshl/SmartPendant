// Facing of rectangle between Start and End points at current Z with parallel
// passes. Both points are tool position by default: reset of parameter reads
// current position again.
// Speed 0 - spindle isn't started by the script.

// Facing parameters: Variable name, scaler, units, min value, max value
// For enum parameters: Variable name, 0 to indicate enum, first value(0), ..., last value(N)
int direction = 0;                    // Direction; 0; X Axis; Y Axis
int start_x_position = GetMetricAxisPosX(); // Face Start X; 1000; mm; -1000000; 1000000
int start_y_position = GetMetricAxisPosY(); // Face Start Y; 1000; mm; -1000000; 1000000
int end_x_position = GetMetricAxisPosX();   // Face End X; 1000; mm; -1000000; 1000000
int end_y_position = GetMetricAxisPosY();   // Face End Y; 1000; mm; -1000000; 1000000
int step = 3000;        // Step for pass; 1000; mm; 10; 1000000
int z_clearance = 5000; // Z clearance; 1000; mm; 1000; 1000000
int speed = 0;          // Speed; 1; rpm; 0; 3000
int feed = 60;          // Feed; 1; mm/min; 1; 1000
int coolant = 0;        // Coolant; 0; Flood; Mist; None

main()
{
  // Get start Z position
  int z_position = GetMetricAxisPosZ();
  // Calculate safe Z position
  int z_safe_position = z_position + z_clearance;

  // Generate G-code
  // Save modal state to restore it at the end
  println("M70; Save modal state");

  println("G90; Absolute mode");
  println("G21; Metric mode");
  println("G94; Feed per minute mode");
  println("G40; Cutter compensation off");
  println("G50; Scaling off");

  // Set speed(if any)
  if(speed != 0) println("M3 S", speed);
  // Set coolant
  if(coolant == 0) println("M8"); // Flood
  if(coolant == 1) println("M7"); // Mist

  // X Axis direction
  if(direction == 0)
  {
    // Calculate distance between start and end points along Y axis
    int y_distance = end_y_position - start_y_position;
    // Calculate number of passes for facing
    int passes_cnt = abs(y_distance / step) + (y_distance % step ? 1 : 0);

    // Cycle for facing passes
    for(int pass = 0; pass < passes_cnt + 1; pass++)
    {
      // Move tool away from part
      println("G0 Z", printfp(z_safe_position, 1000));
      // Start point of the pass. Distance is multiplied in two parts to avoid
      // 32-bit overflow for big areas.
      int y_position = start_y_position;
      if(passes_cnt) y_position += (y_distance / passes_cnt) * pass + ((y_distance % passes_cnt) * pass) / passes_cnt;
      // Return tool to the next start point
      println("G0 X", printfp(start_x_position, 1000), " Y", printfp(y_position, 1000));
      // Move tool down in position
      println("G1 Z", printfp(z_position, 1000), " F", feed);
      // Make a pass
      println("G1 X", printfp(end_x_position, 1000), " F", feed);
    }
  }
  else // Y Axis direction
  {
    // Calculate distance between start and end points along X axis
    int x_distance = end_x_position - start_x_position;
    // Calculate number of passes for facing
    int passes_cnt = abs(x_distance / step) + (x_distance % step ? 1 : 0);

    // Cycle for facing passes
    for(int pass = 0; pass < passes_cnt + 1; pass++)
    {
      // Move tool away from part
      println("G0 Z", printfp(z_safe_position, 1000));
      // Start point of the pass(see above)
      int x_position = start_x_position;
      if(passes_cnt) x_position += (x_distance / passes_cnt) * pass + ((x_distance % passes_cnt) * pass) / passes_cnt;
      // Return tool to the next start point
      println("G0 Y", printfp(start_y_position, 1000), " X", printfp(x_position, 1000));
      // Move tool down in position
      println("G1 Z", printfp(z_position, 1000), " F", feed);
      // Make a pass
      println("G1 Y", printfp(end_y_position, 1000), " F", feed);
    }
  }

  // Move tool away from part before spindle is stopped
  println("G0 Z", printfp(z_safe_position, 1000));

  // Restore modal state. Spindle state is restored too, so stop it after that.
  println("M72; Restore modal state");

  // Stop coolant
  println("M9");
  // Stop spindle
  println("M5");
}
