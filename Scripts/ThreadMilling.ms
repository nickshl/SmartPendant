// Internal thread milling from the center of the hole, start Z is the top of
// the thread. Right thread is cut upward, left thread - downward. Only full
// turns are made: helix is extended above start Z if needed.
// "Feed" is feed of the cutting edge: feed of tool center is reduced.
// Speed 0 - spindle isn't started by the script.

// Threading parameters: Variable name, scaler, units, min value, max value
// For enum parameters: Variable name, 0 to indicate enum, first value, ..., last value
int endmill_diameter = 2400; // Endmill diameter; 1000; mm; 0; 25400
int thread_diameter = 3000;  // Thread major diameter; 1000; mm; 0; 1000000
int thread_pitch = 500;      // Thread pitch; 1000; mm; 10; 10000
int thread_depth = 5000;     // Thread depth; 1000; mm; 0; 100000
int thread_direction = 0;    // Thread direction; 0; Right; Left
int feed = 30;               // Feed; 1; mm/min; 1; 1000
int speed = 0;               // Speed; 1; rpm; 0; 5000
int coolant = 0;             // Coolant; 0; Flood; Mist; None

main()
{
  if(endmill_diameter < thread_diameter)
  {
    // Helix diameter - tool center follows smaller diameter than the cutting edge
    int helix_diameter = thread_diameter - endmill_diameter;
    // Find full thread depth: to simplify calculations threading is done only in
    // full turns. For the right thread it is needed to return tool to initial
    // position, for the left one - for initial lift to avoid cutting deeper than set.
    int thread_full_depth = (thread_depth / thread_pitch) * thread_pitch + ((thread_depth % thread_pitch) ? thread_pitch : 0);

    // Feed for the tool center, mm/min * 1000: it moves along smaller circle than
    // the cutting edge and should move slower. Formula in two forms: precision
    // for small diameters, no overflow for large ones.
    int center_feed = (helix_diameter < 2000) ? (feed * 1000 * helix_diameter) / thread_diameter : feed * ((helix_diameter * 1000) / thread_diameter);
    // Feed can't be zero
    if(center_feed < 1) center_feed = 1;

    // Switch to relative and metric mode
    // Save modal state to restore it at the end
    println("M70; Save modal state");

    println("G91; Relative mode");
    println("G21; Metric mode");
    println("G94; Feed per minute mode");
    println("G40; Cutter compensation off");
    println("G50; Scaling off");
    println("G17; XY plane");

    // Set speed(if any)
    if(speed != 0) println("M3 S", speed);
    // Set coolant
    if(coolant == 0) println("M8"); // Flood
    if(coolant == 1) println("M7"); // Mist

    // Dive to desired depth for Right thread
    if(thread_direction == 0) println("G1 Z", printfp(-thread_depth, 1000), " F", feed);
    // Lift the head for Left thread to avoid cutting deeper than requested
    if(thread_direction == 1)  println("G1 Z", printfp(thread_full_depth - thread_depth, 1000), " F", feed);

    // Approach
    println("G03 X", printfp(helix_diameter / 2, 1000), " Y0 I", printfp(helix_diameter / 4, 1000)," J0 F", printfp(center_feed, 1000));

    // Generate cycle
    for(int threading_progress = 0; threading_progress < thread_depth; threading_progress += thread_pitch)
    {
      // Thread Interpolation
      println("G3 X0Y0 Z", printfp(thread_direction ? -thread_pitch : thread_pitch, 1000), " I-", printfp(helix_diameter / 2, 1000), " J0 F", printfp(center_feed, 1000));
    }

    // Move back to the center
    println("G03 X-", printfp(helix_diameter / 2, 1000)," I-", printfp(helix_diameter / 4, 1000)," J0 F", printfp(center_feed, 1000));

    // Dive to initial depth for Right thread
    if(thread_direction == 0) println("G1 Z", printfp(thread_depth - thread_full_depth, 1000), " F", feed);
    // Retract to initial position for Left thread
    if(thread_direction == 1) println("G1 Z", printfp(thread_depth, 1000), " F", feed);

    // Restore modal state. Spindle state is restored too, so stop it after that.
    println("M72; Restore modal state");

    // Stop coolant
    println("M9");
    // Stop spindle
    println("M5");
  }
}
