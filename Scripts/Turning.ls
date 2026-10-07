// Turning: steps along X and cuts along Z for "Turn Length" from current
// position in negative direction(positive if length is negative). Tool above
// "Turn Diameter" makes outside turn, tool below it - inside turn.
// Speed 0 - spindle isn't started by the script.

// Turning parameters: Variable name, scaler, units, min value, max value
int turn_length = 10000;   // Turn Length; 1000; mm; -1000000; 1000000
int turn_diameter = 10000; // Turn Diameter; 1000; mm; 0; 1000000
int rough_step = 250;      // Rough Step; 1000; mm; 10; 1000
int rough_feed = 120;      // Rough Feed; 1; mm/min; 1; 1000
int rough_speed = 0;       // Rough Speed; 1; rpm; 0; 3000
int finish_step = 100;     // Finish Step; 1000; mm; 0; 1000
int finish_feed = 60;      // Finish Feed; 1; mm/min; 1; 1000
int finish_speed = 0;      // Finish Speed; 1; rpm; 0; 3000

main()
{
  // Get start Z position
  int start_z_position = GetMetricAxisPosZ();
  // Calculate end Z position
  int end_z_position = start_z_position - turn_length;
  // Get current diameter. If machine in radius mode - multiply axis position by 2.
  int current_diameter = GetMetricAxisPosX() * (IsLatheDiameterMode() ? 1 : 2);
  // Find cut distance. Difference in diameters divided by 2.
  int cut_distance = abs(current_diameter - turn_diameter) / 2;
  // Number of rough passes and rough step
  int rough_pass_cnt = 0;
  int rough_pass_distance = 0;

  // Roughing passes are needed only if cut distance is greater than finishing pass
  if(cut_distance > finish_step)
  {
    // Find number of rough passes
    rough_pass_cnt = ((cut_distance - finish_step) / rough_step + 1);
    // Find rough step to keep desired thickness of finishing pass
    rough_pass_distance = (cut_distance - finish_step) / rough_pass_cnt;
  }

  // Find if we want to make outside or inside turn
  int is_outside = (current_diameter > turn_diameter);

  // Start X axis position
  int start_x_position = current_diameter / 2;
  // Current X axis position
  int x_position = start_x_position;
  // X axis position to move tool away from part
  int retract_x_position = start_x_position;

  // Generate G-code only if there is something to cut
  if(current_diameter != turn_diameter)
  {
    // Save modal state to restore it at the end
    println("M70; Save modal state");

    println("G90; Absolute mode");
    println("G21; Metric mode");
    println("G94; Feed per minute mode");
    println("G40; Cutter compensation off");
    println("G50; Scaling off");
    println("G8; Radius mode");

    for(int i = 0; i < rough_pass_cnt; i++)
    {
      // Set speed for the rough passes(if any)
      if((i == 0) && (rough_speed != 0)) println("G97 M3 S", rough_speed);
      // Cutting radius
      x_position += is_outside ? -rough_pass_distance : rough_pass_distance;
      // Move tool to cut position
      println("G1 X", printfp(x_position, 1000), " F", rough_feed);
      // Make a pass
      println("G1 Z", printfp(end_z_position, 1000), " F", rough_feed);
      // Move tool away from part(1 mm clearance), not further than start position for inside turn
      retract_x_position = x_position + (is_outside ? 1000 : -1000);
      if(!is_outside && (retract_x_position < start_x_position)) retract_x_position = start_x_position;
      println("G0 X", printfp(retract_x_position, 1000));
      // Return tool to start point
      println("G0 Z", printfp(start_z_position, 1000));
    }
    // Set speed for the final pass
    if(finish_speed != 0) println("G97 M3 S", finish_speed);
    // Final pass
    x_position = turn_diameter / 2;
    // Move tool to cut position
    println("G1 X", printfp(x_position, 1000), " F", finish_feed);
    // Make a pass
    println("G1 Z", printfp(end_z_position, 1000), " F", finish_feed);
    // Move tool away from part(1 mm clearance), not further than start position for inside turn
    retract_x_position = x_position + (is_outside ? 1000 : -1000);
    if(!is_outside && (retract_x_position < start_x_position)) retract_x_position = start_x_position;
    println("G0 X", printfp(retract_x_position, 1000));
    // Return tool to start point
    println("G0 Z", printfp(start_z_position, 1000));
    // Move tool to desired diameter
    println("G1 X", printfp(x_position, 1000), " F", finish_feed);

    // Restore modal state. Spindle state is restored too, so stop it after that.
    println("M72; Restore modal state");

    // Stop spindle
    println("M5");
  }
}
