//******************************************************************************
//  @file DisplayDrv.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Display Driver Class, implementation
//
//  @copyright Copyright (c) 2016, Devtronic & Nicolai Shlapunov
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
#include "DisplayDrv.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
DisplayDrv& DisplayDrv::GetInstance(void)
{
   static DisplayDrv display_drv;
   return display_drv;
}

// *****************************************************************************
// ***   Display Driver Setup   ************************************************
// *****************************************************************************
Result DisplayDrv::Setup()
{
  // Task can't be initialized without display driver
  Result result = Result::ERR_NULL_PTR;

  // If pointer to display present
  if(display != nullptr)
  {
    // Init display driver
    display->Init();
    // Set inversion
    InvertDisplay(inversion);
    // Set mode - mode can be set earlier than Display initialization
    SetUpdateMode(update_mode);

    // Main list always full screen
    list.SetParams(0, 0, width, height);

    // If pointer to touchscreen present
    if(touch != nullptr)
    {
      // Init touchscreen driver
      touch->Init();
    }

    // Show string if flag is set
    if(DISPLAY_DEBUG_INFO)
    {
      // Set string parameters
      fps_str.SetParams(str, width/3, height - 12, COLOR_MAGENTA, Font_4x6::GetInstance());
      // Max Z
      fps_str.Show(0xFFFFFFFFU);
    }

    // Show string if flag is set
    if(DISPLAY_DEBUG_TOUCH)
    {
      // Set string parameters
      touch_cir.SetParams(0, 0, 3, COLOR_YELLOW, true);
      // Max Z
      touch_cir.Show(0xFFFFFFFFU);
    }

    // Set result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Display Driver Loop   *************************************************
// *****************************************************************************
Result DisplayDrv::Loop()
{
  // Variable for find FPS
  uint32_t time_ms = RtosTick::GetTimeMs();

  // If semaphore doesn't exist or taken within 50 ms - skip draw screen
  // This is need for update touchscreen state every 50 ms(20 times per second)
  // even if display is not updated
  if(screen_update.Take(50U) == Result::RESULT_OK)
  {
    // Set window for all screen and pointer to first pixel
    //if(is_dirty && (LockDisplay() == Result::RESULT_OK))
    if(LockDisplay() == Result::RESULT_OK)
    {
#if defined(MULTIPLE_UPDATE_AREAS)
      // Get current number of update areas
      uint32_t n = areas.GetItemsCnt();
      // To process touch we should not sit there forever if areas constantly adding
      for(uint32_t i = 0u; (i < n) && !areas.IsEmpty(); i++)
#endif
      {
        // Take line semaphore to copy area
        line_mutex.Lock();
        // Get update ares
#if defined(MULTIPLE_UPDATE_AREAS)
        areas.Pop(area);
#else
        // Clear flag to allow invalidate smaller area
        is_dirty = false;
#endif
        // Copy area
        uint16_t start_x = area.start_x;
        uint16_t start_y = area.start_y;
        uint16_t end_x = area.end_x;
        uint16_t end_y = area.end_y;
        // Give semaphore after changes
        line_mutex.Release();

        // Set flag if data need preparation - call virtual function once per frame
        bool is_data_need_preparation = display->IsDataNeedPreparation();
        // Set address window for all screen
        display->SetAddrWindow(start_x, start_y, end_x, end_y);
        // Find number of pixels for given area
        uint16_t pixels_cnt = end_x - start_x + 1u;
        // For each line/row
        for(int32_t i = start_y; i <= end_y; i++)
        {
          // Find current line in buffer
          scr_line_idx = i % 2;
          // Clear half of buffer
          for(uint32_t i = 0u; i < DISPLAY_MAX_BUF_LEN; i++) scr_buf[scr_line_idx][i] = bkg_color;
          // Take semaphore before draw line
          line_mutex.Lock();
          // Draw list to buf                TODO: UPDATE_LEFT_RIGHT is not works correctly if area_x isn't centered on a display
          if(update_mode == UPDATE_LEFT_RIGHT) list.DrawInBufH(scr_buf[scr_line_idx], pixels_cnt, end_y - i, start_x);
          else                                 list.DrawInBufW(scr_buf[scr_line_idx], pixels_cnt, i, start_x);
          // Give semaphore after changes
          line_mutex.Release();
          // Show display area as needed. Allow to debug unnecessary display updates.
          if(DISPLAY_DEBUG_AREA)
          {
            // Sequential colors will help to see updated area.
            static color_t colors[] = {COLOR_WHITE, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA};
            static uint32_t cidx = 0;
            // Change color only at first line
            if(i == start_y)
            {
              cidx++;
              if(cidx >= NumberOf(colors)) cidx = 0u;
            }

            if((i == start_y) || (i == end_y))
            {
              for(uint32_t i = 0; i < pixels_cnt; i++)
              {
                scr_buf[scr_line_idx][i] = colors[cidx];
              }
            }
            else
            {
              scr_buf[scr_line_idx][0] = colors[cidx];
              scr_buf[scr_line_idx][pixels_cnt - 1] = colors[cidx];
            }
          }
          // Check display bits per color
          if(is_data_need_preparation)
          {
            display->PrepareData(scr_buf[scr_line_idx], pixels_cnt);
          }
          // Wait until previous transfer complete
          while(display->IsTransferComplete() == false) taskYIELD();
          // Write stream to LCD
          display->WriteDataStream((uint8_t*)scr_buf[scr_line_idx], display->GetPixelDataCnt(pixels_cnt));
          // DO NOT TRY "OPTIMIZE" CODE !!!
          // Two "while" cycles used for generate next line when previous line
          // transfer via SPI to display.
        }
        // Wait until last transfer complete
        while(display->IsTransferComplete() == false) taskYIELD();
        // Pull up CS
        display->StopTransfer();
      }
      // Give semaphore after draw frame
      UnlockDisplay();
      // Calculate FPS if debug info is ON
      if(DISPLAY_DEBUG_INFO)
      {
        // FPS in format XX.X
        fps_x10 = (1000 * 10) / (RtosTick::GetTimeMs() - time_ms);
      }
    }
  }

  bool tmp_is_touch = false;
  int32_t tmp_tx = tx;
  int32_t tmp_ty = ty;
  // Try to take mutex. 1 ms should be enough.
  if(touchscreen_mutex.Lock(1u) == Result::RESULT_OK)
  {
    // If pointer to touchscreen present
    if(touch != nullptr)
    {
      // Get touch coordinates
      tmp_is_touch = touch->GetXY(tmp_tx, tmp_ty);
    }
    // Give semaphore for drawing frame - we can enter in this "if" statement
    // only if mutex taken
    touchscreen_mutex.Release();
  }
  // If touch state changed (move)
  if(is_touch && tmp_is_touch)
  {
    // Take semaphore before draw line
    line_mutex.Lock();
    if((tx != tmp_tx) || (ty != tmp_ty))
    {
      // Call action for parent list
      list.Action(VisObject::ACT_MOVE, tmp_tx, tmp_ty, tx, ty);
    }
    else
    {
      // Call action for parent list
      list.Action(VisObject::ACT_HOLD, tmp_tx, tmp_ty, tx, ty);
    }
    // Give semaphore after changes
    line_mutex.Release();
  }
  // If touch state changed (touch & release)
  if(is_touch != tmp_is_touch)
  {
    // Go thru VisObject list and call Active() function for active object
    // Take semaphore before draw line
    line_mutex.Lock();
    // Call action for parent list
    list.Action(tmp_is_touch ? VisObject::ACT_TOUCH : VisObject::ACT_UNTOUCH, tmp_tx, tmp_ty, tx, ty);
    // Give semaphore after changes
    line_mutex.Release();
  }
  // Debug code. Can be enabled at compilation time. Show touch position.
  if(DISPLAY_DEBUG_TOUCH)
  {
    if((tx != tmp_tx) || (ty != tmp_ty)) touch_cir.Move(tmp_tx, tmp_ty);
  }
  // Save new touch state
  is_touch = tmp_is_touch;
  tx = tmp_tx;
  ty = tmp_ty;

  // Debug code. Can be enabled at compilation time.
  if(DISPLAY_DEBUG_INFO)
  {
    if(is_touch) sprintf(str, "X: %4ld, Y: %4ld", tx, ty);
    else sprintf(str, "FPS: %2lu.%1lu, time: %lu", fps_x10/10, fps_x10%10, RtosTick::GetTimeMs()/1000UL);
    fps_str.SetString(str);
  }

  // Always run
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Set display driver(or clear if nullptr used as argument)   ************
// *****************************************************************************
Result DisplayDrv::SetDisplayDrv(IDisplay* in_display)
{
  Result result = Result::ERR_INVALID_ITEM;
  // Display driver should be set before scheduler started
  if(Rtos::IsSchedulerNotRunning())
  {
    // If pointer to display present
    if(in_display != nullptr)
    {
      // Save display driver pointer
      display = in_display;
      // Set width and height variables for screen
      width = display->GetWidth();
      height = display->GetHeight();
      // Set result
      result = Result::RESULT_OK;
    }
    else
    {
      // Task can't be initialized without display driver
      result = Result::ERR_NULL_PTR;
    }
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   Lock display   ********************************************************
// *****************************************************************************
Result DisplayDrv::LockDisplay(uint32_t wait_ms)
{
  // Take semaphore for protect draw frame
  Result result = frame_mutex.Lock(wait_ms);
  // Return result
  return result;
}

// *****************************************************************************
// ***   Unlock display   ******************************************************
// *****************************************************************************
Result DisplayDrv::UnlockDisplay(void)
{
  // Give semaphore for drawing frame
  Result result = frame_mutex.Release();
  // Return result
  return result;
}

// *****************************************************************************
// ***   Lock display line   ***************************************************
// *****************************************************************************
Result DisplayDrv::LockDisplayLine(uint32_t wait_ms)
{
  // Take semaphore for protect draw line
  Result result = line_mutex.Lock(wait_ms);
  // Return result
  return result;
}

// *****************************************************************************
// ***   Unlock display line   *************************************************
// *****************************************************************************
Result DisplayDrv::UnlockDisplayLine(void)
{
  // Give semaphore for drawing frame
  Result result = line_mutex.Release();
  // Return result
  return result;
}

// *****************************************************************************
// ***   Update display   ******************************************************
// *****************************************************************************
Result DisplayDrv::UpdateDisplay(void)
{
  // Give semaphore for update screen
  Result result = screen_update.Give();
  // Return result
  return result;
}

// *****************************************************************************
// ***   Invalidate Area   *****************************************************
// *****************************************************************************
Result DisplayDrv::InvalidateArea(int16_t start_x, int16_t start_y, int16_t end_x, int16_t end_y)
{
  Result result = Result::ERR_BAD_PARAMETER;
  // Lock display line
  line_mutex.Lock();
#if defined(COLOR_3BIT)
  // In 3 bit mode each byte contains 2 pixels, so we
  // can't start or end at any odd number of pixels.
  if(start_x % 2) start_x--;
  if(start_y % 2) start_y--;
  if(end_x % 2) end_x++;
  if(end_y % 2) end_y++;
#endif
  // Check end points - it can't be greater than screen size
  if(start_x < 0) start_x = 0;
  if(start_y < 0) start_y = 0;
  if(end_x >= width) end_x = width - 1;
  if(end_y >= height) end_y = height - 1;
  // Set area only if it is valid
  if((start_x <= end_x) && (start_y <= end_y))
  {
    if(update_mode == UPDATE_LEFT_RIGHT)
    {
      // Swap area X and Y it refreshing mode left to right
      SwapData(start_x, start_y);
      SwapData(end_x, end_y);
    }

#if defined(MULTIPLE_UPDATE_AREAS)
    // Add new area to existing one
    area.start_x = start_x;
    area.start_y = start_y;
    area.end_x = end_x;
    area.end_y = end_y;
    areas.Push(area);
#else
    // If display is "dirty"
    if(is_dirty)
    {
      // Add new area to existing one
      if(start_x < area.start_x) area.start_x = start_x;
      if(start_y < area.start_y) area.start_y = start_y;
      if(end_x > area.end_x) area.end_x = end_x;
      if(end_y > area.end_y) area.end_y = end_y;
    }
    else
    {
      // Set area
      area.start_x = start_x;
      area.end_x = end_x;
      area.start_y = start_y;
      area.end_y = end_y;
      is_dirty = true;
    }
#endif

    // All checks passed - result good
    result = Result::RESULT_OK;
  }
  // Unlock display line
  line_mutex.Release();
  // Return result
  return result;
}

// *****************************************************************************
// ***   Set Update Area   *****************************************************
// *****************************************************************************
Result DisplayDrv::SetUpdateArea(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y)
{
  Result result = Result::ERR_BAD_PARAMETER;
  // Lock display
  LockDisplay();
  // Check end points - it can't be greater than screen size
  if(end_x >= width) end_x = width - 1;
  if(end_y >= height) end_y = height - 1;
  // Set area only if it is valid
  if((start_x < end_x) && (start_y < end_y))
  {
    if(update_mode == UPDATE_TOP_BOTTOM)
    {
      area.start_x = start_x;
      area.end_x = end_x;
      area.start_y = start_y;
      area.end_y = end_y;
    }
    else // UPDATE_LEFT_RIGHT
    {
      // Swap area X and Y it refresh mode left to right
      area.start_x = start_y;
      area.end_x = end_y;
      area.start_y = start_x;
      area.end_y = end_x;
    }
    // All checks passed - result good
    result = Result::RESULT_OK;
  }
  // Unlock display
  UnlockDisplay();
  // Return result
  return result;
}

// *****************************************************************************
// ***   Update area covered by VisObject on display   *************************
// *****************************************************************************
Result DisplayDrv::UpdateObjArea(VisObject& obj)
{
  // Set update area to full screen
  SetUpdateArea(obj.GetStartX(), obj.GetStartY(), obj.GetEndX(), obj.GetEndY());
  // Give semaphore for update screen
  Result result = screen_update.Give();
  // Return result
  return result;
}

// *****************************************************************************
// ***   Update specific display area   ****************************************
// *****************************************************************************
Result DisplayDrv::UpdateArea(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y)
{
  // Set update area to full screen
  SetUpdateArea(start_x, start_y, end_x, end_y);
  // Give semaphore for update screen
  Result result = screen_update.Give();
  // Return result
  return result;
}

// *****************************************************************************
// ***   Invert Display   ******************************************************
// *****************************************************************************
void DisplayDrv::InvertDisplay(bool invert)
{
  // Lock display
  LockDisplay();
  // Wait while transfer complete before change settings
  while(display->IsTransferComplete() == false);
  // Set rotation
  display->InvertDisplay(invert);
  // Save inversion
  inversion = invert;
  // Unlock display
  UnlockDisplay();
}

// *****************************************************************************
// ***   Set Rotation   ********************************************************
// *****************************************************************************
void DisplayDrv::SetRotation(IDisplay::Rotation rot)
{
  // Lock display
  LockDisplay();
  // Wait while transfer complete before change settings
  while(display->IsTransferComplete() == false);
  // Set rotation
  display->SetRotation(rot);
  // Set width and height variables for selected screen update mode
  width = display->GetWidth();
  height = display->GetHeight();
  // Save rotation
  rotation = rot;
  // Update main list to match full screen
  list.SetParams(0, 0, width, height);
  // Unlock display
  UnlockDisplay();
  // Set update area to full screen
  InvalidateArea(0, 0, width, height);
  // If update mode is different than default - we have to prepare display for it
  if(update_mode != UPDATE_TOP_BOTTOM)
  {
      SetUpdateMode(update_mode);
  }
  // If pointer to touchscreen present
  if(touch != nullptr)
  {
    // Get touch coordinates
    touch->SetRotation((ITouchscreen::Rotation)rot);
  }
}

// *****************************************************************************
// ***   Set Update Mode   *****************************************************
// *****************************************************************************
/// TODO: this function probably isn't work as intended. Main reason for existance
/// of Left to Right update mode to draw functions(like oscilloscope buffer on grid)
/// to to it in Top to Bottom mode for each displayed line we have to run trough all
/// data array to figure out if we have to draw something. In Left to Right we can
/// grab data from array by current line index which improve speed significantly.
void DisplayDrv::SetUpdateMode(UpdateMode mode)
{
  // Lock display
  LockDisplay();
  // Wait while transfer complete before change settings
  while(display->IsTransferComplete() == false);
  // Change Update mode
  if(mode == UPDATE_LEFT_RIGHT)
  {
    display->SetRotation(((rotation - 1u) < IDisplay::ROTATION_CNT) ? (IDisplay::Rotation)(rotation - 1u) : IDisplay::ROTATION_RIGHT);
  }
  else
  {
    display->SetRotation(rotation);
  }
  // Save Update mode
  update_mode = mode;
  // Unlock display
  UnlockDisplay();  
  // Set update adea to full screen
  InvalidateArea(0, 0, width, height);
}

// *****************************************************************************
// ***   Set touchscreen driver(or clear if nullptr used as argument)   ********
// *****************************************************************************
Result DisplayDrv::SetTouchDrv(ITouchscreen* in_touch)
{
  Result result = Result::ERR_INVALID_ITEM;
  // Touchscreen driver should be set before scheduler started
  if(Rtos::IsSchedulerNotRunning())
  {
    // Store new touch pointer
    touch = in_touch;
    // Set good result
    result = Result::RESULT_OK;
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   Get Touch X and Y coordinate   ****************************************
// *****************************************************************************
bool DisplayDrv::GetTouchXY(int32_t& x, int32_t& y)
{
  // Result variable
  bool result = false;
  
  // Try to take mutex. 1 ms should be enough.
  if(touchscreen_mutex.Lock(1U) == Result::RESULT_OK)
  {
    // If display driver gets touch coordinates and touch still present
    if((touch != nullptr) && is_touch && touch->IsTouched())
    {
      // Return last values
      x = tx;
      y = ty;
      // Set result
      result = true;
    }
    else
    {
      // If no touch - clear flag for prevent return wrong coordinates if
      // display will touched without reads new coordinates
      is_touch = false;
    }
    // Give semaphore for drawing frame - we can enter in this "if" statement
    // only if mutex taken
    touchscreen_mutex.Release();
  }
  // Return result
  return result;
}

// *************************************************************************
// ***   Check touch   *****************************************************
// *************************************************************************
bool DisplayDrv::IsTouched()
{
  // Not touched by default
  bool touched = false;
  // Try to take mutex. 1 ms should be enough.
  if(touchscreen_mutex.Lock(1U) == Result::RESULT_OK)
  {
    // If pointer to touchscreen present
    if(touch != nullptr)
    {
      // Get status
      touched = touch->IsTouched();
    }
    // Give semaphore for drawing frame - we can enter in this "if" statement
    // only if mutex taken
    touchscreen_mutex.Release();
  }
  // Return status
  return touched;
}

// *****************************************************************************
// ***   Calibrate Touchscreen   ***********************************************
// *****************************************************************************
void DisplayDrv::TouchCalibrate()
{
  // If pointer to touchscreen present
  if(touch != nullptr)
  {
    // Box for calibration
    Box background(0, 0, width, height, COLOR_BLACK, true);
    Box box(0, 0, 2, 2, COLOR_WHITE, true);
    int32_t tx;
    int32_t ty;
    int32_t x1, x2;
    int32_t y1, y2;

    // Reset calibration
    touch->SetCalibrationConsts(ITouchscreen::COEF, ITouchscreen::COEF, 0, 0);

    // Show background box
    background.Show(0xFFFFFFFFU-1U);
    // Show box
    box.Show(0xFFFFFFFFU);

    // Move box to position
    box.Move(10-1, 10-1);
    // Wait press for get initial coordinates
    while(!GetTouchXY(x1, y1))
    {
      // Update Display
      UpdateDisplay();
      // Delay
      RtosTick::DelayMs(100U);
    }
    // Wait unpress and measure coordinates continuously for averaging
    while(GetTouchXY(tx, ty))
    {
      x1 = (x1 + tx) / 2;
      y1 = (y1 + ty) / 2;
      // Update Display - for update touch coordinates
      UpdateDisplay();
      // Delay
      RtosTick::DelayMs(100U);
    }

    // Move box to position
    box.Move(width - 10 - 1, height - 10 - 1);
    // Wait press for get initial coordinates
    while(!GetTouchXY(x2, y2))
    {
      // Update Display
      UpdateDisplay();
      // Delay
      RtosTick::DelayMs(100U);
    }
    // Wait unpress and measure coordinates continuously for averaging
    while(GetTouchXY(tx, ty))
    {
      x2 = (x2 + tx) / 2;
      y2 = (y2 + ty) / 2;
      // Update Display
      UpdateDisplay();
      // Delay
      RtosTick::DelayMs(100U);
    }

    // Calc coefs
    int32_t kx = ((x2 - x1) * ITouchscreen::COEF) / (width  - 2*10);
    int32_t ky = ((y2 - y1) * ITouchscreen::COEF) / (height - 2*10);
    int32_t bx = 10 - (x1 * ITouchscreen::COEF) / kx;
    int32_t by = 10 - (y1 * ITouchscreen::COEF) / ky;

    // Save calibration
    touch->SetCalibrationConsts(kx, ky, bx, by);

    // Hide box
    box.Hide();
  }
}
