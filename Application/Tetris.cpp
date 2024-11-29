//******************************************************************************
//  @file Tetris.cpp
//  @author Nicolai Shlapunov
//
//  @details Application: Tetris Application Class, header
//
//  @copyright Copyright (c) 2017, Devtronic & Nicolai implementation
//             All rights reserved.
//
//  @section SUPPORT
//
//   Devtronic invests time and resources providing this open source code,
//   please support Devtronic and open-source hardware/software by
//   donations and/or purchasing products from Devtronic.
//
//******************************************************************************

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "Tetris.h"

// *****************************************************************************
// ***   Constants   ***********************************************************
// *****************************************************************************

const uint16_t music_data_table[]= {
0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x0F71, 0x14A1,
0x1881, 0x0A51, 0x1061, 0x14A1, 0x1B81, 0x0A51, 0x1061, 0x14A1, 0x1B81, 0x0A51, 0x1061, 0x14A1, 0x1B81, 0x0A51, 0x1061,
0x14A1, 0x1B81, 0x0A51, 0x0DC1, 0x1261, 0x1721, 0x0A51, 0x0DC1, 0x1261, 0x1721, 0x0A51, 0x0DC1, 0x1261, 0x1721, 0x0A51,
0x0DC1, 0x1261, 0x1721, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x0F71, 0x14A1, 0x1881,
0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0931, 0x1061, 0x1721, 0x1B81, 0x0931, 0x1061, 0x1721, 0x1B81, 0x0931, 0x1061, 0x1721,
0x1B81, 0x0931, 0x1061, 0x1721, 0x1B81, 0x0C41, 0x0F71, 0x1261, 0x1EE1, 0x0C41, 0x0F71, 0x1261, 0x1EE1, 0x0C41, 0x0F71,
0x1261, 0x1EE1, 0x0C41, 0x0F71, 0x1261, 0x1EE1, 0x0831, 0x0C41, 0x14A1, 0x1EE1, 0x0831, 0x0C41, 0x14A1, 0x1EE1, 0x0831,
0x0B91, 0x14A1, 0x1B81, 0x0831, 0x0B91, 0x14A1, 0x1B81, 0x07B1, 0x0C41, 0x14A1, 0x1EE1, 0x07B1, 0x0C41, 0x14A1, 0x1EE1,
0x07B1, 0x0DC1, 0x1061, 0x1371, 0x07B1, 0x0DC1, 0x0F71, 0x1371, 0x0A51, 0x0C41, 0x0F71, 0x14A1, 0x0A51, 0x0C41, 0x0F71,
0x14A1, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0931, 0x1261, 0x1881, 0x1B81, 0x0931, 0x1261,
0x1881, 0x1B81, 0x0931, 0x1061, 0x1721, 0x1B81, 0x0931, 0x1061, 0x1721, 0x1B81, 0x0C41, 0x0F71, 0x1261, 0x1881, 0x0C41,
0x0F71, 0x1261, 0x1881, 0x0C41, 0x1261, 0x1881, 0x1EE1, 0x0C41, 0x1261, 0x1881, 0x1EE1, 0x0B91, 0x1721, 0x1EE1, 0x22A1,
0x0B91, 0x1721, 0x1EE1, 0x22A1, 0x0B91, 0x14A1, 0x1D21, 0x22A1, 0x0B91, 0x14A1, 0x1D21, 0x22A1, 0x0F71, 0x1261, 0x1721,
0x24B1, 0x0F71, 0x1261, 0x1721, 0x24B1, 0x0DC1, 0x1EE1, 0x24B1, 0x2E41, 0x0DC1, 0x1EE1, 0x24B1, 0x2E41, 0x0D01, 0x1EE1,
0x24B1, 0x3DC1, 0x0D01, 0x1EE1, 0x24B1, 0x3DC1, 0x0C41, 0x1721, 0x1EE1, 0x24B1, 0x0C41, 0x15E1, 0x1EE1, 0x24B1, 0x0B91,
0x1721, 0x1EE1, 0x22A1, 0x0B91, 0x1721, 0x1EE1, 0x22A1, 0x0B91, 0x1EE1, 0x22A1, 0x2E41, 0x0B91, 0x1EE1, 0x22A1, 0x2E41,
0x0B91, 0x1721, 0x1D21, 0x22A1, 0x0B91, 0x1721, 0x1D21, 0x22A1, 0x0B91, 0x1D21, 0x22A1, 0x2E41, 0x0B91, 0x1D21, 0x22A1,
0x2E41, 0x0F71, 0x3DC1, 0x2E41, 0x24B1, 0x1EE1, 0x2E41, 0x24B1, 0x1EE1, 0x1721, 0x3DC1, 0x1EE1, 0x1721, 0x1261, 0x1061,
0x0F71, 0x0DC1, 0x0C41, 0x3DC1, 0x2E41, 0x24B1, 0x1EE1, 0x2E41, 0x24B1, 0x1EE1, 0x1721, 0x24B1, 0x1EE1, 0x1721, 0x1261,
0x1151, 0x0F71, 0x0C41, 0x0B91, 0x2E41, 0x22A1, 0x1EE1, 0x1721, 0x22A1, 0x1EE1, 0x1721, 0x14A1, 0x1EE1, 0x1D21, 0x1721,
0x1151, 0x0F71, 0x0E91, 0x0B91, 0x0931, 0x1261, 0x1721, 0x1261, 0x0F71, 0x1EE1, 0x1721, 0x0F71, 0x09C1, 0x1EE1, 0x1721,
0x1371, 0x0F71, 0x1B81, 0x1721, 0x0F71, 0x14A1, 0x5261, 0x3DC1, 0x3101, 0x2931, 0x3DC1, 0x3101, 0x2931, 0x1EE1, 0x3101,
0x2931, 0x1EE1, 0x1881, 0x1721, 0x14A1, 0x1261, 0x1061, 0x5261, 0x3DC1, 0x3101, 0x2931, 0x3DC1, 0x3101, 0x2931, 0x1EE1,
0x3101, 0x2931, 0x1EE1, 0x1881, 0x1721, 0x14A1, 0x1061, 0x0F71, 0x3DC1, 0x2E41, 0x2931, 0x1EE1, 0x2E41, 0x2931, 0x1EE1,
0x1B81, 0x2931, 0x26E1, 0x1EE1, 0x1721, 0x14A1, 0x1371, 0x0F71, 0x07B1, 0x0F71, 0x1371, 0x1721, 0x1EE1, 0x26E1, 0x2E41,
0x3DC1, 0x3101, 0x2931, 0x1EE1, 0x1881, 0x1721, 0x1371, 0x0F71, 0x09C1, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x0F71,
0x14A1, 0x1881, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x1061, 0x14A1, 0x1B81, 0x0A51,
0x1061, 0x14A1, 0x1B81, 0x0A51, 0x1061, 0x14A1, 0x1B81, 0x0A51, 0x1061, 0x14A1, 0x1B81, 0x0A51, 0x0DC1, 0x1261, 0x1721,
0x0A51, 0x0DC1, 0x1261, 0x1721, 0x0A51, 0x0DC1, 0x1261, 0x1721, 0x0A51, 0x0DC1, 0x1261, 0x1721, 0x0A51, 0x0F71, 0x14A1,
0x1881, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0A51, 0x0F71, 0x14A1, 0x1881, 0x0931, 0x1061,
0x1721, 0x1B81, 0x0931, 0x1061, 0x1721, 0x1B81, 0x0931, 0x1061, 0x1721, 0x1B81, 0x0931, 0x1061, 0x1721, 0x1B81, 0x0C41,
0x0F71, 0x1261, 0x1EE1, 0x0C41, 0x0F71, 0x1261, 0x1EE1, 0x0C41, 0x0F71, 0x1261, 0x1EE1, 0x0C41, 0x0F71, 0x1261, 0x1EE1,
0x0831, 0x0C41, 0x14A1, 0x1EE1, 0x0831, 0x0C41, 0x14A1, 0x1EE1, 0x0831, 0x0B91, 0x14A1, 0x1B81, 0x0831, 0x0B91, 0x14A1,
0x1B81, 0x0831, 0x0F71, 0x14A1, 0x1881, 0x0831, 0x0F71, 0x14A1, 0x1881, 0x0831, 0x0DC1, 0x14A1, 0x1721, 0x0831, 0x0DC1,
0x14A1, 0x1721, 0x0831, 0x0C41, 0x14A1, 0x1EE1, 0x0831, 0x0C41, 0x14A1, 0x1EE1, 0x0831, 0x0B91, 0x14A1, 0x1B81, 0x0831,
0x0B91, 0x14A1, 0x1B81, 0x07B1, 0x0F71, 0x14A1, 0x1881, 0x07B1, 0x0F71, 0x14A1, 0x1881, 0x07B1, 0x0DC1, 0x1371, 0x1721,
0x07B1, 0x0DC1, 0x1371, 0x1721, 0x0A51, 0x5261, 0x3DC1, 0x3101, 0x2931, 0x1EE1, 0x1881, 0x14A1, 0x0F71, 0x3DC1, 0x3101,
0x2931, 0x1EE1, 0x1881, 0x14A1, 0x0F71, 0x0C41, 0x3101, 0x2931, 0x3101, 0x2E41, 0x3DC1, 0x4DC1, 0x5C81, 0xA4D1, 0x3DC1,
0x3101, 0x2931, 0x1EE1, 0x1881, 0x14A1, 0x0F71, 0x0A5F, 0x000F};

// Array contains all possible shapes
const bool TetrisShape::shapesArray[7][4*4] = 
{
  {0,1,0,0, 0,1,1,0, 0,1,0,0, 0,0,0,0}, // T
  {0,1,0,0, 0,1,0,0, 0,1,1,0, 0,0,0,0}, // L
  {0,0,1,0, 0,0,1,0, 0,1,1,0, 0,0,0,0}, // J
  {0,1,0,0, 0,1,0,0, 0,1,0,0, 0,1,0,0}, // I
  {0,0,0,0, 0,1,1,0, 0,1,1,0, 0,0,0,0}, // O
  {0,0,1,0, 0,1,1,0, 0,1,0,0, 0,0,0,0}, // S
  {0,1,0,0, 0,1,1,0, 0,0,1,0, 0,0,0,0}, // Z
};

// Shape rotating matrix 
const int8_t TetrisShape::shapeRtArray[4][4*4] = 
{
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {3,7,11,15,2,6,10,14,1,5,9,13,0,4,8,12},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {12,8,4,0,13,9,5,1,14,10,6,2,15,11,7,3}
};
  
// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
Tetris& Tetris::GetInstance(void)
{
   static Tetris tetris;
   return tetris;
}

// *****************************************************************************
// ***   Tetris Setup   ********************************************************
// *****************************************************************************
Result Tetris::Setup()
{
  // Set display rotation
  display_drv.SetRotation(IDisplay::ROTATION_RIGHT);
  // Set display background color
  display_drv.SetBackgroundColor(COLOR_BLACK);

  // Return result
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Application task   ****************************************************
// *****************************************************************************
Result Tetris::Loop()
{
  // Bucket object
  TetrisBucket bucket;
  // Shape object
  TetrisShape shape;
  // Next shape object
  TetrisShape next_shape;

  // String to show pause
  String pause_str("PAUSE", (display_drv.GetScreenW() - strlen("PAUSE")*12)/2,(display_drv.GetScreenH() - 16) / 2, COLOR_WHITE, Font_12x16::GetInstance());

  // Play Sound (Demo)
  sound_drv.PlaySound(music_data_table, NumberOf(music_data_table), 120U, true);

  // Initialize random seed
  srand(RtosTick::GetTickCount());

  // Show bucket, shape, next shape and score string on screen
  bucket.Show(1);
  shape.Show(2);
  next_shape.Show(3);
  char scr_str[32] = {" "};
  String score_str(scr_str, (WIDTH + 1u) * CUBE_SIZE, 16, COLOR_WHITE, Font_8x12::GetInstance());
  score_str.Show(3);

  // Init ticks variable
  uint32_t last_wake_ticks = RtosTick::GetTickCount();

  // Delay between frames
  int32_t delay;
  // Delays per shape moving down
  int32_t loops;
  // Pause flag
  bool pause = false;

  // Value for encoder task
  int32_t last_enc_val = 0;

  while (game_over == false)
  {
    // Delay between frames
    delay = 100;
    // Delays per shape moving down
    loops = 10 - bucket.GetScore()/1000;
    // Loops can't be less than 1
    if(loops < 1) loops = 1;

    // Copy next shape to current one
    shape.PopulateShapeArray(next_shape);
    // Create next shape
    next_shape.PopulateShapeArray(rand()%7, 1+(rand()%5));
    next_shape.MoveShape(WIDTH + 2u, 5);

    if(bucket.CheckShapeCollisionIntoBucket(shape))
    {
      char str[16] = {"GAME OVER"};
      String gameover_str(str,(display_drv.GetScreenW() - strlen(str)*12)/2,(display_drv.GetScreenH() - 16) / 2, COLOR_WHITE, Font_12x16::GetInstance());
      gameover_str.Show(10);
      // Update Display
      display_drv.UpdateDisplay();
      // Pause until next tick
      RtosTick::DelayMs(3000U);
      // Exit from cycle
      break;
    }

    round = true;
    while(round == true)
    {
      // Lock Display
      display_drv.LockDisplay();

      // Get buttons state
      bool left_btn_state = input_drv.GetButtonState(InputDrv::BTN_LEFT) && input_drv.GetButtonCurrentState(InputDrv::BTN_LEFT);
      bool right_btn_state = input_drv.GetButtonState(InputDrv::BTN_RIGHT) && input_drv.GetButtonCurrentState(InputDrv::BTN_RIGHT);
      // Create value from buttons state
      int32_t btn_cnt = (left_btn_state ? -1 : 0) + (right_btn_state ? 1 : 0);
      // Get encoder count since last call
      int32_t en_cnt = input_drv.GetEncoderState(last_enc_val);

      if((en_cnt != 0) && (pause == false))
      {
        // No need use more than one move per 100 ms 
        int32_t dir = en_cnt > 0 ? 1 : -1;
        for(int32_t i = en_cnt; i != 0; i -= dir)
        {
          // Move shape
          shape.MoveShape(dir, 0, true);
          // If shape have collision
          if (bucket.CheckShapeCollisionIntoBucket(shape))
          {
            // Return shape on previous position
            shape.MoveShape(-dir, 0, true);
            // Exit from loop
            break;
          }
        }
      }

      if((btn_cnt != 0) && (pause == false))
      {
        // No need use more than one rotation per 100 ms 
        int32_t rot = btn_cnt > 0 ? 1 : -1;
        for(int32_t i = btn_cnt; i != 0; i -= rot)
        {
          // Rotate shape
          shape.RotateShape(rot);
          // If we cannot rotate shape
          if (bucket.CheckShapeCollisionIntoBucket(shape))
          {
            // Rotate back
            shape.RotateShape(-rot);
            // Exit from loop
            break;
          }
        }
      }
      // If any encoder button pressed - pull shape down 
      if(   (input_drv.GetButtonState(InputDrv::BTN_LEFT_DOWN) && input_drv.GetButtonCurrentState(InputDrv::BTN_LEFT_DOWN))
         || (input_drv.GetButtonState(InputDrv::BTN_RIGHT_DOWN) && input_drv.GetButtonCurrentState(InputDrv::BTN_RIGHT_DOWN)) )
      {
        if(pause == false)
        {
          delay = 1;
          loops = 0;
        }
      }

      // If any encoder button pressed - pull shape down
      if(   (input_drv.GetButtonState(InputDrv::BTN_LEFT_UP) && input_drv.GetButtonCurrentState(InputDrv::BTN_LEFT_UP))
         || (input_drv.GetButtonState(InputDrv::BTN_RIGHT_UP) && input_drv.GetButtonCurrentState(InputDrv::BTN_RIGHT_UP)) )
      {
        // Invert pause flag
        pause = !pause;
        // Show or hide "PAUSE" string
        if(pause) pause_str.Show(10);
        else      pause_str.Hide();
      }
      if(pause == false)
      {
        if(loops == 0)
        {
          // Fall down
          shape.MoveShape(0, 1, true);
          // If shape have collision
          if (bucket.CheckShapeCollisionIntoBucket(shape))
          {
            // Return shape on up position
            shape.MoveShape(0, -1, true);
            // Store shape in bucket - now it is static
            bucket.PutShapeIntoBucket(shape);
            // This round finished
            round = false;
          }
          // Delays per shape moving down
          loops = 10 - bucket.GetScore()/1000;
          // Loops can't be less than 1
          if(loops < 1) loops = 1;
        }
        else
        {
          loops--;
        }
      }
      // Create score string
      score_str.SetString(scr_str, NumberOf(scr_str), "Score: %lu", bucket.GetScore());
      // Unlock Display
      display_drv.UnlockDisplay();
      // Update Display
      display_drv.UpdateDisplay();
      // Pause until next tick
      RtosTick::DelayUntilMs(last_wake_ticks, delay);
    }
    bucket.RemoveFullLines();
  }

  // Always run
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TetrisShape   *********************************************************
// *****************************************************************************
TetrisShape::TetrisShape()
{
  // Init by default
  PopulateShapeArray(0, 1);
}

// *****************************************************************************
// ***   RotateShape   *********************************************************
// *****************************************************************************
void TetrisShape::RotateShape(int8_t dir)
{
  // Apply rotation
  shapeRotate += dir;
  // Check underflow/overflow
  if (shapeRotate < 0) shapeRotate = 3;
  if (shapeRotate > 3) shapeRotate = 0;

  // Fill array with new shape
  for (int32_t n = 0; n < 4 * 4; n++)
  {
    shapeArray[shapeRtArray[shapeRotate][n]] = shapesArray[shapeNum][n];
  }

  // Invalidate area, during rotation position isn't changed
  InvalidateObjArea();
}

// *****************************************************************************
// ***   MoveShape   ***********************************************************
// *****************************************************************************
void TetrisShape::MoveShape(int8_t x, int8_t y, bool is_delta)
{
  // Invalidate area before move to redraw area object move from
  InvalidateObjArea();
  // Make changes
  if(is_delta == true)
  {
    // Move shape in delta coordinates
    shapeTopLeftX += x;
    shapeTopLeftY += y;
  }
  else
  {
    // Move shape in absolute coordinates
    shapeTopLeftX = x;
    shapeTopLeftY = y;
  }
  // Invalidate area after move to redraw area object move to
  InvalidateObjArea();
}

// *****************************************************************************
// ***   DrawShapeIntoBuffer   *************************************************
// *****************************************************************************
// This function copy shape to bucket
void TetrisShape::DrawShapeIntoBuffer(uint8_t * buf, int32_t width, int32_t height)
{
  for (int32_t y = 0; y < 4; y++)
  {
    for (int32_t x = 0; x < 4; x++)
    {
      if (shapeArray[y*4 + x] == true)
      {
        buf[(shapeTopLeftY + y)*width + shapeTopLeftX + x] = shapeColorIdx;
      }
    }
  }
}

// *****************************************************************************
// ***   CheckShapeCollisionIntoBuffer   ***************************************
// *****************************************************************************
// This function check collision between shape and bucket
bool TetrisShape::CheckShapeCollisionIntoBuffer(uint8_t * buf, int32_t width, int32_t height)
{
  for(int32_t y = 0; y < 4; y++)
  {
    for(int32_t x = 0; x < 4; x++)
    {
      if(    (buf[(shapeTopLeftY + y)*width + shapeTopLeftX + x] != 0x00)
          && (shapeArray[y*4 + x] == true) )
      {
        return true;
      }
    }
  }
  return false;
}

// *****************************************************************************
// ***   PopulateShapeArray   **************************************************
// *****************************************************************************
void TetrisShape::PopulateShapeArray(uint8_t shapeType, uint8_t color_idx)
{
  shapeTopLeftX = 6;
  shapeTopLeftY = 0;
  shapeNum = shapeType;
  shapeRotate = 0;
  shapeColorIdx = color_idx;

  for(int32_t n = 0; n < 4*4; n++)
  {
    shapeArray[n] = shapesArray[shapeNum][n];
  }
}

// *****************************************************************************
// ***   PopulateShapeArray   **************************************************
// *****************************************************************************
void TetrisShape::PopulateShapeArray(TetrisShape& shape)
{
  shapeTopLeftX = 6;
  shapeTopLeftY = 0;
  shapeNum = shape.shapeNum;
  shapeRotate = shape.shapeRotate;
  shapeColorIdx = shape.shapeColorIdx;

  for(int32_t n = 0; n < 4*4; n++)
  {
    shapeArray[n] = shape.shapeArray[n];
  }
}

// ***************************************************************************
// ***   Put line in buffer   ************************************************
// ***************************************************************************
void TetrisShape::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  // Draw only if needed
  if((line >= shapeTopLeftY*CUBE_SIZE) && (line < (shapeTopLeftY+4)*CUBE_SIZE))
  {
    int32_t shape_line = line - shapeTopLeftY*CUBE_SIZE;
    int32_t start = -1;
    int32_t end = 0;
    for(int32_t i = 0; i < 4; i++)
    {
      if(shapeArray[(shape_line/CUBE_SIZE)*4 + i] == true)
      {
        if(start == -1) start = ((shapeTopLeftX+i)*CUBE_SIZE) - start_x;
        end = ((shapeTopLeftX+i)*CUBE_SIZE) + CUBE_SIZE - 1 - start_x;
      }
    }
    // Prevent write in memory before buffer
    if(start < 0) start = 0;
    // Prevent buffer overflow
    if(end > n) end = n;
    // Have sense draw only if end pointer in buffer
    if(end > 0)
    {
      for(int32_t i = start; i <= end; i++) buf[i] = colors[shapeColorIdx];
    }
  }
}

// *****************************************************************************
// ***   Invalidate Display Area   *********************************************
// *****************************************************************************
void TetrisShape::InvalidateObjArea(bool force)
{
  // Calculate position in pixels
  x_start = shapeTopLeftX * CUBE_SIZE;
  x_end = (shapeTopLeftX + 4) * CUBE_SIZE;
  y_start = shapeTopLeftY * CUBE_SIZE;
  y_end = (shapeTopLeftY + 4) * CUBE_SIZE;
  // And invalidate object area
  VisObject::InvalidateObjArea();
}

// *****************************************************************************
// ***   TetrisBucket   ********************************************************
// *****************************************************************************
TetrisBucket::TetrisBucket()
{
  InitBucket();
}

// *****************************************************************************
// ***   CheckShapeCollisionIntoBucket   ***************************************
// *****************************************************************************
// * Check shape collision with bucket
bool TetrisBucket::CheckShapeCollisionIntoBucket(TetrisShape& ts)
{
  return ts.CheckShapeCollisionIntoBuffer(bucket, WIDTH, HEIGHT);
}

// *****************************************************************************
// ***   PutShapeIntoBucket   **************************************************
// *****************************************************************************
// * Store shape into bucket
void TetrisBucket::PutShapeIntoBucket(TetrisShape& ts)
{
  ts.DrawShapeIntoBuffer(bucket, WIDTH, HEIGHT);
}

// *****************************************************************************
// ***   RemoveFullLines   *****************************************************
// *****************************************************************************
void TetrisBucket::RemoveFullLines()
{
  int32_t y;
  int32_t cnt = 0;
  // Don't check last line with border
  for (y = 0; y < HEIGHT - 1; y++)
  {
    int32_t x;
    bool f = true;
    for (x = 1; x < WIDTH-1; x++)
    {
      if(bucket[y*WIDTH + x] == 0x00) f = false;
    }
    if (f == true)
    {
      memcpyback(&bucket[WIDTH], bucket, (y - 1)*WIDTH + x);
      cnt++;
    }
  }
  if (cnt > 0)
  {
    // Update score
    score += 100 + 200 * (cnt - 1);
    // Invalidate bucket to update
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   InitBucket   **********************************************************
// *****************************************************************************
// * Init bucket with default values
void TetrisBucket::InitBucket()
{
  int32_t y;
  // Fill bucket with zero color index(black)
  memset(bucket, 0x00, WIDTH*HEIGHT);
  // Fill bucket left & right borders with 7 color index(gray)
  for (y = 0; y < HEIGHT-1; y++)
  {
    bucket[y*WIDTH] = 0x07;
    bucket[y*WIDTH + WIDTH -1] = 0x07;
  }
  // Fill bucket down borders with 7 color index(gray)
  for (int32_t x = 0; x < WIDTH; x++)
  {
    bucket[y*WIDTH + x] = 0x07;
  }
  // Calculate position in pixels for InvalidateObjArea()
  x_start = 0;
  x_end = WIDTH * CUBE_SIZE;
  y_start = 0;
  y_end = HEIGHT * CUBE_SIZE;
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void TetrisBucket::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  // Draw only if needed
  if(line < HEIGHT*CUBE_SIZE)
  {
    // Convert line number to bucket line idx
    int32_t bucket_line = line/CUBE_SIZE;
    for(int32_t i=0; i < WIDTH; i++)
    {
      uint8_t color = bucket[bucket_line*WIDTH + i];
      if(color != 0x00)
      {
        int32_t start = (i*CUBE_SIZE) - start_x;
        int32_t end = start + CUBE_SIZE - 1;
        // Have sense draw only if end pointer in buffer
        if(end > 0)
        {
          for(int32_t i = start; i <= end; i++) buf[i] = colors[color];
        }
      }
    }
  }
  else
  {
    volatile int32_t bucket_line = line/CUBE_SIZE;
    bucket_line--;
  }
}

// *****************************************************************************
// ***   memcpyback   **********************************************************
// *****************************************************************************
// * Copy from end to start(for overlapped blocks)
void TetrisBucket::memcpyback(uint8_t * dst, uint8_t * src, int32_t num)
{
  // Copy byte-by-byte in back order
  for (int32_t i = num-1; i >= 0; i--)
  {
    dst[i] = src[i];
  }
}
