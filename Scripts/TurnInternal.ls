// Turning parameters: Variable name, scaler, units, min value, max value
int turn_length = 10000;   // Turn Length; 1000; mm; -1000000; 1000000
int turn_diameter = 10000; // Turn Diameter; 1000; mm; 0; 1000000
int rough_step = 200;      // Rough Step; 1000; mm; 0; 1000
int rough_feed = 120;      // Rough Feed; 1; mm/min; 1; 1000
int rough_speed = 0;       // Rough Speed; 1; rpm; 1; 3000
int finish_step = 100;     // Finish Step; 1000; mm; 0; 1000
int finish_feed = 60;      // Finish Feed; 1; mm/min; 1; 1000
int finish_speed = 0;      // Finish Speed; 1; rpm; 1; 3000

main()
{
  // Get start Z position
  int start_z_position = GetAxisPosZ();
  // Get start X position
  int start_x_position = GetAxisPosX();
  // Calculate end Z position
  int end_z_position = start_z_position - turn_length;
  // Get current diameter. If machine in radius mode - multiply axis position by 2.
  int current_diameter = GetAxisPosX() * (IsLatheDiameterMode() ? 1 : 2);
  // Find number of rough passes
  int rough_pass_cnt = ((turn_length - finish_step) / rough_step + 1);
  // Find rough step to keep desired thickness of finishing pass
  int rough_pass_distance = (turn_length - finish_step) / rough_pass_cnt;

  // Current X axis position
  int z_cut_position = start_z_position;

  // If we need to cut distance less than finishing pass - there shouldn't be any roughing passes
  if(turn_length <= finish_step)
  {
    rough_pass_cnt = 0;
  }

  // If allocation is successful and we can determinate if it outside or inside turn - generate G-code
  if(current_diameter < turn_diameter)
  {
    println("G90; Absolute mode");
    println("G21; Metric mode");
    println("G8; Radius mode");

    // Move tool to cut position
    println("G0 X", printfp(start_x_position, 1000));

    for(int i = 0; i < rough_pass_cnt; i++)
    {
      // Cutting radius
      z_cut_position -= rough_pass_distance;
      // Set speed for the rough passes(if any)
      if((i == 0) && (rough_speed != 0)) println("M3 S", rough_speed);
      // Move tool to cut position
      println("G1 Z", printfp(z_cut_position, 1000), " F", rough_feed);
      // Make a pass
      println("G1 X", printfp(turn_diameter / 2, 1000), " F", rough_feed);
      // Move tool to base position
      println("G1 Z", printfp(start_z_position, 1000), " F", rough_feed);
      // Move tool to cut position
      println("G0 X", printfp(start_x_position, 1000));
    }
    // Set speed for the final pass
    if(finish_speed != 0) println("M3 S", finish_speed);
    // Final pass
    z_cut_position = end_z_position;
    // Move tool to cut position
    println("G1 Z", printfp(z_cut_position, 1000), " F", finish_feed);
    // Make a pass
    println("G1 X", printfp(turn_diameter / 2, 1000), " F", finish_feed);
    // Move tool to base position
    println("G1 Z", printfp(start_z_position, 1000), " F", finish_feed);
    // Move tool to cut position
    println("G0 X", printfp(start_x_position, 1000));
    // Stop spindle
    println("M5");
  }
}
