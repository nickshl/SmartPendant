// Facing parameters: Variable name, scaler, units, min value, max value
int face_length = 10000;   // Turn Length; 1000; mm; -1000000; 1000000
int face_diameter = 10000; // Turn Diameter; 1000; mm; 0; 1000000
int rough_step = 250;      // Rough Step; 1000; mm; 0; 1000
int rough_feed = 120;      // Rough Feed; 1; mm/min; 1; 1000
int rough_speed = 0;       // Rough Speed; 1; rpm; 1; 3000
int finish_step = 100;     // Finish Step; 1000; mm; 0; 1000
int finish_feed = 60;      // Finish Feed; 1; mm/min; 1; 1000
int finish_speed = 0;      // Finish Speed; 1; rpm; 1; 3000

main()
{
  // Get current diameter. If lathe in Diameter mode - divide by two to get radius.
  int start_x_position = GetAxisPosX() / (IsLatheDiameterMode() ? 2 : 1);
  // Calculate end X position
  int end_x_position = face_diameter / 2;
  // Find number of rough passes
  int rough_pass_cnt = ((face_length - finish_step) / rough_step + 1);
  // Find rough step to keep desired thickness of finishing pass
  int rough_pass_distance = (face_length - finish_step) / rough_pass_cnt;

  // If we need to cut distance less than finishing pass - there shouldn't be any rouging passes
  if(face_length <= finish_step)
  {
    rough_pass_cnt = 0;
  }

  // Get start Z position
  int z_position = GetAxisPosZ();

  // If allocation is successful - generate G-code
  println("G90; Absolute mode");
  println("G21; Metric mode");
  println("G8; Radius mode");

  for(int i = 0; i < rough_pass_cnt; i++)
  {
    // Set speed for the rough passes(if any)
    if((i == 0) && (rough_speed != 0)) println("M3 S", rough_speed);
    // Cutting radius
    z_position -= rough_pass_distance;
    // Move tool to cut position
    println("G1 Z", printfp(z_position, 1000), " F", rough_feed);
    // Make a pass
    println("G1 X", printfp(end_x_position, 1000), " F", rough_feed);
    // Move tool away from part(1 mm clearance)
    println("G0 Z", printfp(z_position + 1000, 1000));
    // Return tool to start point
    println("G0 X", printfp(start_x_position, 1000));
  }
  // Set speed for the final pass
  if(finish_speed != 0) println("M3 S", finish_speed);
  // Final pass
  z_position = GetAxisPosZ() - face_length;
  // Move tool to cut position
  println("G1 Z", printfp(z_position, 1000), " F", finish_feed);
  // Make a pass
  println("G1 X", printfp(end_x_position, 1000), " F", finish_feed);
  // Move tool away from part(1 mm clearance)
  println("G0 Z", printfp(z_position + 1000, 1000));
  // Return tool to start point
  println("G0 X", printfp(start_x_position, 1000));
  // Move tool to desired diameter
  println("G1 Z", printfp(z_position, 1000), " F", finish_feed);
  // Stop spindle
  println("M5");
}
