// Threading parameters: Variable name, scaler, units, min value, max value
int length = 10000;        // Thread Length; 1000; mm; 0; 1000000
int pitch = 1500;          // Thread Pitch; 1000; mm; 0; 10000
int depth = 920;           // Cutting depth; 1000; mm; 0; 5000
int cut = 0;               // Cut feed; 0; Radial Infeed; Flank Infeed
int type = 0;              // Cut type; 0; Constant Area; Constant Depth
int step = 100;            // Step; 1000; mm; 0; 1000
int speed = 200;           // Speed; 1; rpm; 1; 3000
int spring_passes = 2;     // Spring Passes; 1; cnt; 0; 5

main()
{
  // Get start Z & X position
  int start_z_position = GetAxisPosZ();
  // Get current diameter. If machine in radius mode - multiply axis position by 2.
  int start_diameter = GetAxisPosX() * (IsLatheDiameterMode() ? 1 : 2);
  // Minor diameter
  int minor_diameter = start_diameter - depth * 2;
  // Find full shift for Flank Infeed: depth / sqrt(3)
  int full_shift = (depth * 577) / 1000;
  // Find area of first pass triangle(for 60 degree threads), sq.mm * 1000^2
  int area = (step * step * 577) / 1000;
  // Shadow variable to prevent changing settings
  int spring_passes = spring_passes;

  // Set parameters to proper gcode execution
  println("G90; Absolute mode");
  println("G21; Metric mode");
  println("G7;  Diameter mode");

  // Set speed for threading
  println("M3 S", speed);

  // Current diameter for passes
  int current_diameter = start_diameter;

  // Threading cycle
  while((current_diameter > minor_diameter) || (spring_passes > 0))
  {
    // If we still in threading cycle
    if(current_diameter != minor_diameter)
    {
      // Constant depth cutting
      if(type == 1)
      {
        // Substract step depth from current diameter(multiplied by two because of diameter mode)
        current_diameter -= step * 2; 
      }
      else
      {
        // Find current height
        int current_height = (start_diameter - current_diameter) / 2;
        // Fins area of first pass triangle(for 60 degree threads)
        int new_height = sqrt(current_height * current_height + step * step);
        // Limit minimal height increase to 25 um
        if((new_height - current_height) < 25) new_height = current_height + 25;
        // Adjust diameter
        current_diameter = start_diameter - new_height * 2;
      }
      // We shouldn't cut more than minor diameter
      if(current_diameter < minor_diameter) current_diameter = minor_diameter;

      // Flank Infeed
      if(cut == 1)
      {
        // Find shift by multiplying current depth by 0.577(for 60 degree threads)
        int shift = ((start_diameter - current_diameter) / 2 * 577) / 1000;
        // Move tool to shift position
        println("G1 Z", printfp(start_z_position + full_shift - shift, 1000), " F60");
      }
      else if(cut == 2) // Incremental Infeed
      {
        ; // Not implemented yet, would be Radial Infeed
      }
      else
      {
        ; // Radial Infeed: no need to move Z from base position
      }
    }
    else
    {
      spring_passes--;
    }
    // Move tool to workpiece
    println("G1 X", printfp(current_diameter, 1000), " F60");
    // Make a pass
    println("G33 Z", printfp(start_z_position - length, 1000), " K", printfp(pitch, 1000));
    // Move tool away from part(1mm per diameter, 0.5 mm clearance)
    println("G0 X", printfp(current_diameter + 1000, 1000));
    // Move tool to base position
    println("G0 Z", printfp(start_z_position, 1000));
  }

  // Move tool to the start diameter
  println("G0 X", printfp(start_diameter, 1000));

  // Stop spindle
  println("M5");
}
