// Threading parameters: Variable name, scaler, units, min value, max value
int length = 10000;        // Thread Length; 1000; mm; -1000000; 1000000
int pitch = 500;           // Thread Pitch; 1000; mm; 0; 10000
int step = 200;            // Initial Depth; 1000; mm; 0; 1000
int depth = 1000;          // Full Depth; 1000; mm; 0; 1000
int depth_degression = 15; // Depth Degression; 10; mm; 10; 20
int speed = 200;             // Speed; 1; rpm; 1; 3000

main()
{
  // Get start Z position
  int z_position = GetAxisPosZ();

  // If allocation is successful - generate G-code
  println("G90; Absolute mode");
  println("G21; Metric mode");
  println("G8; Radius mode");

  // Set speed for the final pass
  println("M3 S", finish_speed);
  // Move tool to cut position
  print("G1 Z"); printfp(z_position, 1000); println(" F", finish_feed);
  // Make a pass
  print("G1 X"); printfp(end_x_position, 1000); println(" F", finish_feed);
  // Move tool away from part(1 mm clearance)
  print("G0 Z"); printfp(z_position + 1000, 1000); println();
  // Return tool to start point
  print("G0 X"); printfp(start_x_position, 1000); println();
  // Move tool to desired diameter
  print("G1 Z"); printfp(z_position, 1000); println(" F", finish_feed);
  // Stop spindle
  println("M5");
}
