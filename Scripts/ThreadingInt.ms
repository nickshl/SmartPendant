// Enlarge parameters:  Variable name, scaler, units, min value, max value
// For enum parameters: Variable name, 0 to indicate enum, first value, ..., last value
int endmill_diameter = 2400; // Endmill diameter; 1000; mm; 0; 25400
int thread_diameter = 3000;  // Thread major diameter; 1000; mm; 0; 1000000
int thread_pitch = 500;      // Thread pitch; 1000; mm; 0; 10000
int thread_depth = 5000;     // Thread depth; 1000; mm; 0; 100000
int thread_direction = 0;    // Thread direction; 0; Right; Left
int feed = 30;               // Feed; 1; mm/min; 1; 1000
int speed = 0;               // Speed; 1; rpm; 1; 5000
int coolant = 0;             // Coolant; 0; Flood; Mist; None

main()
{
  if(endmill_diameter < thread_diameter)
  {
    // Helix diameter - we need to follow smaller diameter to endmill follow desired one
    int helix_diameter = thread_diameter - endmill_diameter;
    // Find full thread depth. To simplify calcualtions we do threading only in full
    // thread pitch helix segments. For right thread we need know full thread to
    // to calcualate movement at the end to move tool to initial position. For
    // the left thread we need it to calculate initial lift to about cutting depper
    // than set.
    int thread_full_depth = (thread_depth / thread_pitch) * thread_pitch + ((thread_depth % thread_pitch) ? thread_pitch : 0);

    // Switch to relative and metric mode
    println("G91; Relative mode");
    println("G21; Metric mode");

    // Set speed for the rough passes(if any)
    if(speed != 0) println("M3 S", speed);
    // Set coolant
    if(coolant == 0) println("M8"); // Flood
    if(coolant == 1) println("M7"); // Mist

    // Dive to desired depth for Right thread
    if(thread_direction == 0) println("G1 Z", printfp(-thread_depth, 1000), " F", feed);
    // Lift the head for Left thread to avoid cutting deper than requested
    if(thread_direction == 1)  println("G1 Z", printfp(thread_full_depth - thread_depth, 1000), " F", feed);

    // Approach
    println("G03 X", printfp(helix_diameter / 2, 1000), " Y0 I", printfp(helix_diameter / 4, 1000)," J0 F", feed);

    // Generate cycle
    for(int threading_progress = 0; threading_progress < thread_depth; threading_progress += thread_pitch;)
    {
      // Thread Interpolation
      println("G3 X0Y0 Z", printfp(thread_direction ? -thread_pitch : thread_pitch, 1000), " I-", printfp(helix_diameter / 2, 1000), " J0 F", feed);
    }

    // Move back to the center
    println("G03 X-", printfp(helix_diameter / 2, 1000)," I-", printfp(helix_diameter / 4, 1000)," J0 F", feed);

    // Dive to initial depth for Right thread
    if(thread_direction == 0) println("G1 Z", printfp(thread_depth - thread_full_depth, 1000), " F", feed);
    // Retract to initial position for Left thread
    if(thread_direction == 1) println("G1 Z", printfp(thread_depth, 1000), " F", feed);

    // Stop coolant
    println("M9");
    // Stop spindle
    println("M5");
  }
}
