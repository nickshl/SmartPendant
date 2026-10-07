// Grooving and parting off: cuts along X from current position to "Cut
// Diameter" - toward the center or, for internal groove, away from it.
// "Cut Width" adds second cut shifted along Z in positive direction.
// Speed 0 - spindle isn't started by the script.

// Cutting parameters: Variable name, scaler, units, min value, max value
int cut_diameter = 0; // Cut Diameter; 1000; mm; 0; 1000000
int cut_width = 0;    // Cut Width; 1000; mm; 0; 2000
int cut_step = 2000;  // Cut Depth per Step; 1000; mm; 10; 10000
int cut_feed = 120;   // Cut Feed; 1; mm/min; 1; 1000
int cut_speed = 0;    // Cut Speed; 1; rpm; 0; 3000

main()
{
  // Get start X position
  int start_x_position = GetMetricAxisPosX() / (IsLatheDiameterMode() ? 2 : 1);
  // Calculate end X position
  int end_x_position = cut_diameter / 2;
  // Find cut distance
  int cut_distance = abs(start_x_position - end_x_position);
  // Find if we want to make outside or inside cut(internal grooving tool)
  int is_outside = (start_x_position > end_x_position);

  // Generate G-code only if there is something to cut
  if(cut_distance != 0)
  {
    // Current position for X
    int current_x_position = start_x_position;
    // Find number of passes
    int pass_cnt = cut_distance / cut_step + 1;
    // Find step distance
    int pass_distance = cut_distance / pass_cnt;
    // Step distance is rounded down - find what is left to add it to the first passes
    int pass_remainder = cut_distance % pass_cnt;

    // Get start Z position
    int z_position = GetMetricAxisPosZ();

    // Save modal state to restore it at the end
    println("M70; Save modal state");

    println("G90; Absolute mode");
    println("G21; Metric mode");
    println("G94; Feed per minute mode");
    println("G40; Cutter compensation off");
    println("G50; Scaling off");
    println("G8; Radius mode");

    for(int i = 0; i < pass_cnt; i++)
    {
      // Position for fast dive(1 mm clearance) to save time
      int dive_x_position = current_x_position + (is_outside ? 1000 : -1000);
      // Dive only if it is deeper than start position
      int is_dive = is_outside ? (dive_x_position < start_x_position) : (dive_x_position > start_x_position);
      // Distance for this pass
      int distance = pass_distance + ((i < pass_remainder) ? 1 : 0);

      // Set speed(if any)
      if((i == 0) && (cut_speed != 0)) println("G97 M3 S", cut_speed);
      // Make fast dive
      if(is_dive) println("G0 X", printfp(dive_x_position, 1000));
      // Cutting radius
      current_x_position += is_outside ? -distance : distance;
      // Make a pass
      println("G1 X", printfp(current_x_position, 1000), " F", cut_feed);
      // Retract tool
      println("G0 X", printfp(start_x_position, 1000));
      // Second side pass only if width is set
      if(cut_width != 0)
      {
        // Move tool to different Z location
        println("G0 Z", printfp(z_position + cut_width, 1000));
        // Make fast dive
        if(is_dive) println("G0 X", printfp(dive_x_position, 1000));
        // Make a pass
        println("G1 X", printfp(current_x_position, 1000), " F", cut_feed);
        // Retract tool
        println("G0 X", printfp(start_x_position, 1000));
        // Move tool to initial Z location
        println("G0 Z", printfp(z_position, 1000));
      }
    }

    // Restore modal state. Spindle state is restored too, so stop it after that.
    println("M72; Restore modal state");

    // Stop spindle
    println("M5");
  }
}
