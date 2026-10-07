// Facing: steps along Z in negative direction and cuts along X from current
// position to "Face Diameter" - toward the center or away(from inside).
// "Wall Pass" - facing stops "Finish Step" before the diameter, then one pass
// along Z is made at the diameter and tool returns to start position.
// Speed 0 - spindle isn't started by the script.

// Facing parameters: Variable name, scaler, units, min value, max value
// For enum parameters: Variable name, 0 to indicate enum, first value, ..., last value
int face_length = 10000;   // Face Length; 1000; mm; 0; 1000000
int face_diameter = 10000; // Face Diameter; 1000; mm; 0; 1000000
int rough_step = 250;      // Rough Step; 1000; mm; 10; 1000
int rough_feed = 120;      // Rough Feed; 1; mm/min; 1; 1000
int rough_speed = 0;       // Rough Speed; 1; rpm; 0; 3000
int finish_step = 100;     // Finish Step; 1000; mm; 0; 1000
int finish_feed = 60;      // Finish Feed; 1; mm/min; 1; 1000
int finish_speed = 0;      // Finish Speed; 1; rpm; 0; 3000
int wall_pass = 0;         // Wall Pass; 0; Disabled; Enabled

main()
{
  // Get current diameter. If lathe in Diameter mode - divide by two to get radius.
  int start_x_position = GetMetricAxisPosX() / (IsLatheDiameterMode() ? 2 : 1);
  // Calculate end X position(it can be on either side of start position)
  int end_x_position = face_diameter / 2;
  // Find if we want to face from outside or from inside
  int is_outside = (start_x_position > end_x_position);
  // Get start Z position
  int start_z_position = GetMetricAxisPosZ();
  // Calculate end Z position
  int end_z_position = start_z_position - face_length;

  // Material left on the wall for the wall pass:
  int wall_step = 0;
  // finishing step, but not more than there is to cut
  if(wall_pass)
  {
    wall_step = finish_step;
    if(wall_step > abs(start_x_position - end_x_position)) wall_step = abs(start_x_position - end_x_position);
  }
  // X position where facing passes stop
  int face_x_position = end_x_position + (is_outside ? wall_step : -wall_step);

  // Number of rough passes and rough step
  int rough_pass_cnt = 0;
  int rough_pass_distance = 0;

  // Roughing passes are needed only if cut distance is greater than finishing pass
  if(face_length > finish_step)
  {
    // Find number of rough passes
    rough_pass_cnt = ((face_length - finish_step) / rough_step + 1);
    // Find rough step to keep desired thickness of finishing pass
    rough_pass_distance = (face_length - finish_step) / rough_pass_cnt;
  }

  // Current Z axis position
  int z_position = start_z_position;

  // Generate G-code only if there is something to cut
  if(start_x_position != end_x_position)
  {
    // Save modal state to restore it at the end
    println("M70; Save modal state");

    println("G90; Absolute mode");
    println("G21; Metric mode");
    println("G94; Feed per minute mode");
    println("G40; Cutter compensation off");
    println("G50; Scaling off");
    println("G8; Radius mode");

    // Facing passes(there are none if everything is removed by the wall pass)
    if(face_x_position != start_x_position)
    {
      for(int i = 0; i < rough_pass_cnt; i++)
      {
        // Set speed for the rough passes(if any)
        if((i == 0) && (rough_speed != 0)) println("G97 M3 S", rough_speed);
        // Cutting position
        z_position -= rough_pass_distance;
        // Move tool to cut position
        println("G1 Z", printfp(z_position, 1000), " F", rough_feed);
        // Make a pass
        println("G1 X", printfp(face_x_position, 1000), " F", rough_feed);
        // Move tool away from part(1 mm clearance)
        println("G0 Z", printfp(z_position + 1000, 1000));
        // Return tool to start point
        println("G0 X", printfp(start_x_position, 1000));
      }
      // Set speed for the final pass
      if(finish_speed != 0) println("G97 M3 S", finish_speed);
      // Final pass
      z_position = end_z_position;
      // Move tool to cut position
      println("G1 Z", printfp(z_position, 1000), " F", finish_feed);
      // Make a pass
      println("G1 X", printfp(face_x_position, 1000), " F", finish_feed);
      // Move tool away from part(1 mm clearance)
      println("G0 Z", printfp(z_position + 1000, 1000));
      // Return tool to start point
      println("G0 X", printfp(start_x_position, 1000));
    }

    // Wall pass
    if(wall_pass)
    {
      // X position to come to the wall fast(1 mm clearance)
      int clear_x_position = face_x_position + (is_outside ? 1000 : -1000);
      // X and Z positions to move tool away from the corner(1 mm clearance)
      int retract_x_position = end_x_position + (is_outside ? 1000 : -1000);
      int retract_z_position = end_z_position + 1000;
      // None of them should be further than start position
      if(is_outside ? (clear_x_position > start_x_position) : (clear_x_position < start_x_position)) clear_x_position = start_x_position;
      if(is_outside ? (retract_x_position > start_x_position) : (retract_x_position < start_x_position)) retract_x_position = start_x_position;
      if(retract_z_position > start_z_position) retract_z_position = start_z_position;

      // Set speed if it wasn't set for the final facing pass
      if((finish_speed != 0) && (face_x_position == start_x_position)) println("G97 M3 S", finish_speed);
      // Return tool to start Z after facing passes
      if(face_x_position != start_x_position) println("G0 Z", printfp(start_z_position, 1000));
      // Move tool to the wall
      if(clear_x_position != start_x_position) println("G0 X", printfp(clear_x_position, 1000));
      // Dive to face diameter
      println("G1 X", printfp(end_x_position, 1000), " F", finish_feed);
      // Make a pass along the wall
      println("G1 Z", printfp(end_z_position, 1000), " F", finish_feed);
      // Move tool away from the corner diagonally
      println("G0 X", printfp(retract_x_position, 1000), " Z", printfp(retract_z_position, 1000));
      // Return tool to start position
      println("G0 Z", printfp(start_z_position, 1000));
      println("G0 X", printfp(start_x_position, 1000));
    }
    else
    {
      // Move tool to the new face
      println("G1 Z", printfp(end_z_position, 1000), " F", finish_feed);
    }

    // Restore modal state. Spindle state is restored too, so stop it after that.
    println("M72; Restore modal state");

    // Stop spindle
    println("M5");
  }
}
