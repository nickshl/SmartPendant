// External thread cutting with G33. Start position: X at major diameter, Z in
// front of the part.
// "Infeed": Radial - straight into the part. Flank - along one flank of the
// thread. Incremental - along both flanks in turn(starts closer to the part).
// Flank and Incremental are for 60 degree threads only.
// "Cut type": Constant Depth - "Step" per pass. Constant Area - "Step" *
// sqrt(N) for pass N, but not less than 0.025 mm per pass.

// Threading parameters: Variable name, scaler, units, min value, max value
int length = 10000;        // Thread Length; 1000; mm; 0; 1000000
int pitch = 1500;          // Thread Pitch; 1000; mm; 10; 10000
int depth = 920;           // Cutting depth; 1000; mm; 0; 5000
int infeed = 0;            // Infeed; 0; Radial; Flank; Incremental
int type = 0;              // Cut type; 0; Constant Area; Constant Depth
int step = 100;            // Step; 1000; mm; 25; 1000
int speed = 200;           // Speed; 1; rpm; 1; 3000
int spring_passes = 2;     // Spring Passes; 1; cnt; 0; 5

main()
{
  // Get start Z & X position
  int start_z_position = GetMetricAxisPosZ();
  // Get current diameter. If machine in radius mode - multiply axis position by 2.
  int start_diameter = GetMetricAxisPosX() * (IsLatheDiameterMode() ? 1 : 2);
  // Minor diameter
  int minor_diameter = start_diameter - depth * 2;
  // Find full shift for Flank Infeed: depth / sqrt(3)
  int full_shift = (depth * 577) / 1000;
  // Spring passes left to do. Parameter itself must not be changed.
  int spring_passes_left = spring_passes;

  // Set parameters to proper gcode execution
  // Save modal state to restore it at the end
  println("M70; Save modal state");

  println("G90; Absolute mode");
  println("G21; Metric mode");
  println("G94; Feed per minute mode");
  println("G40; Cutter compensation off");
  println("G50; Scaling off");
  println("G7;  Diameter mode");

  // Set speed for threading(G97 - speed in rpm)
  println("G97 M3 S", speed);

  // Current diameter for passes
  int current_diameter = start_diameter;
  // Side of the thread for the current pass of Incremental Infeed
  int flank_side = 0;

  // Threading cycle
  while((current_diameter > minor_diameter) || (spring_passes_left > 0))
  {
    // If we are still in threading cycle
    if(current_diameter != minor_diameter)
    {
      // Constant depth cutting
      if(type == 1)
      {
        // Subtract step depth from current diameter(multiplied by two because of diameter mode)
        current_diameter -= step * 2;
      }
      else
      {
        // Find current height
        int current_height = (start_diameter - current_diameter) / 2;
        // Find new height to remove the same area as the first pass: new^2 = current^2 + step^2
        int new_height = sqrt(current_height * current_height + step * step);
        // Limit minimal height increase to 25 um
        if((new_height - current_height) < 25) new_height = current_height + 25;
        // Adjust diameter
        current_diameter = start_diameter - new_height * 2;
      }
      // We shouldn't cut more than minor diameter
      if(current_diameter < minor_diameter) current_diameter = minor_diameter;

      // Flank Infeed
      if(infeed == 1)
      {
        // Find shift by multiplying current depth by 0.577(for 60 degree threads)
        int shift = ((start_diameter - current_diameter) / 2 * 577) / 1000;
        // Move tool to shift position
        println("G1 Z", printfp(start_z_position + full_shift - shift, 1000), " F60");
      }
      else if(infeed == 2) // Incremental Infeed
      {
        // Find shift the same way as for Flank Infeed
        int shift = ((start_diameter - current_diameter) / 2 * 577) / 1000;
        // Move tool to shift position, every second pass - to the opposite side
        if(flank_side)
        {
          println("G1 Z", printfp(start_z_position - full_shift + shift, 1000), " F60");
        }
        else
        {
          println("G1 Z", printfp(start_z_position + full_shift - shift, 1000), " F60");
        }
        // Change side for the next pass
        flank_side = !flank_side;
      }
      else
      {
        ; // Radial Infeed: no need to move Z from base position
      }
    }
    else
    {
      spring_passes_left--;
    }
    // Move tool to workpiece
    println("G1 X", printfp(current_diameter, 1000), " F60");
    // Make a pass
    println("G33 Z", printfp(start_z_position - length, 1000), " K", printfp(pitch, 1000));
    // Move tool away from part: 0.5 mm above start diameter to be out of the thread
    println("G0 X", printfp(start_diameter + 1000, 1000));
    // Move tool to base position
    println("G0 Z", printfp(start_z_position, 1000));
  }

  // Move tool to the start diameter
  println("G0 X", printfp(start_diameter, 1000));

  // Restore modal state. Spindle state is restored too, so stop it after that.
  println("M72; Restore modal state");

  // Stop spindle
  println("M5");
}
