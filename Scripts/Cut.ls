// Cutting parameters: Variable name, scaler, units, min value, max value
int cut_diameter = 0; // Cut Diameter; 1000; mm; 0; 1000000
int cut_width = 0;    // Turn Width; 1000; mm; 0; 2000
int cut_step = 2000;  // Cut Depth per Step; 1000; mm; 0; 10000
int cut_feed = 120;   // Cut Feed; 1; mm/min; 1; 1000
int cut_speed = 0;    // Cut Speed; 1; rpm; 1; 3000

main()
{
  // Get start X position
  int start_x_position = GetAxisPosX() / (IsLatheDiameterMode() ? 2 : 1);
  // Calculate end X position
  int end_x_position = cut_diameter / 2;
  // Current position for X
  int current_x_position = start_x_position;
  // Find number of passes
  int pass_cnt = (start_x_position - end_x_position) / cut_step + 1;
  // Find step distance
  int pass_distance = (start_x_position - end_x_position) / pass_cnt;

  // Get start Z position
  int z_position = GetAxisPosZ();

  println("G90; Absolute mode");
  println("G21; Metric mode");
  println("G8; Radius mode");

  for(int i = 0u; i < pass_cnt; i++)
  {
    // Set speed for the rough passes(if any)
    if((i == 0) && (cut_speed != 0)) println("M3 S", cut_speed);
    // For each additional cut make fast dive(1 mm clearance) to save time
    if(i != 0) println("G0 X", printfp(current_x_position + 1000, 1000));
    // Cutting radius
    current_x_position -= pass_distance;
    // Make a pass
    println("G1 X", printfp(current_x_position, 1000), " F", cut_feed);
    // Retract tool
    println("G0 X", printfp(start_x_position, 1000));
    // Second side pass only if width is set
    if(cut_width != 0)
    {
      // Move tool to different Z location
      println("G0 Z", printfp(z_position + cut_width, 1000));
      // For each additional cut make fast dive(1 mm clearance) to save time
      if(i != 0) println("G0 X", printfp(current_x_position + pass_distance + 1000, 1000));
      // Make a pass
      println("G1 X", printfp(current_x_position, 1000), " F", cut_feed);
      // Retract tool
      println("G0 X", printfp(start_x_position, 1000));
      // Move tool to initial Z location
      println("G0 Z", printfp(z_position, 1000));
    }
  }
  // Stop spindle
  println("M5");
}
