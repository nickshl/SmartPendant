// Facing parameters: Variable name, scaler, units, min value, max value
int start_x_position = GetAxisPosX(); // Face Start X; 1000; mm; -1000000; 1000000
int start_y_position = GetAxisPosY(); // Face Start Y; 1000; mm; -1000000; 1000000
int end_x_position = GetAxisPosX();   // Face End X; 1000; mm; -1000000; 1000000
int end_y_position = GetAxisPosY();   // Face End Y; 1000; mm; -1000000; 1000000
int step = 3000;        // Step for pass; 1000; mm; 0; 1000000
int z_clearance = 5000; // Z clearance; 1000; mm; 1000; 1000000
int speed = 0;          // Speed; 1; rpm; 1; 3000
int feed = 60;          // Feed; 1; mm/min; 1; 1000

main()
{
  // Calculate distance between start and end points along Y axis
  int y_distance = end_y_position - start_y_position;
  // Calculate number of passes for facing
  int passes_cnt = abs(y_distance / step) + (y_distance % step ? 1 : 0);

  // Get start Z position
  int z_position = GetAxisPosZ();
  // Calculate safe Z position
  int z_safe_position = z_position + z_clearance;

  // If allocation is successful - generate G-code
  println("G90; Absolute mode");
  println("G21; Metric mode");

  // Set speed for the rough passes(if any)
  if(speed != 0) println("M3 S", speed);

  // Cycle for facing passes
  for(int pass = 0; pass < passes_cnt + 1; pass++)
  {
    // Move tool away from part
    println("G0 Z", printfp(z_safe_position, 1000));
    // Return tool to the next start point
    println("G0 X", printfp(start_x_position, 1000), " Y", printfp(passes_cnt ? start_y_position + (y_distance * pass) / passes_cnt : start_y_position, 1000));
    // Move tool down in position
    println("G1 Z", printfp(z_position, 1000), " F", feed);
    // Make a pass
    println("G1 X", printfp(end_x_position, 1000), " F", feed);
  }
  // Stop spindle
  println("M5");
}
