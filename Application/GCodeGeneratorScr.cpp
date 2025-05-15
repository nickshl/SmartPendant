//******************************************************************************
//  @file GCodeGeneratorScr.cpp
//  @author Nicolai Shlapunov
//
//  @details CodeGeneratorScr: User CodeGeneratorScr Class, implementation
//
//  @copyright Copyright (c) 2025, Devtronic & Nicolai Shlapunov
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
#include "GCodeGeneratorScr.h"
#include "Application.h"

#include "fatfs.h"
#include <cctype> // For tolower()

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
GCodeGeneratorScr& GCodeGeneratorScr::GetInstance()
{
  static GCodeGeneratorScr gcodegeneratorscr;
  return gcodegeneratorscr;
}

// *****************************************************************************
// ***   CodeGeneratorScr Setup   **********************************************
// *****************************************************************************
Result GCodeGeneratorScr::Setup(int32_t y, int32_t height)
{
  // Tabs for screens
  tabs.SetParams(0, y, DisplayDrv::GetInstance().GetScreenW(), 40, 2u);
  // Current loaded script tab
  tabs.SetText(0u, "----", nullptr, Font_10x18::GetInstance());
  // Scripts tab
  tabs.SetText(1u, "Scripts", nullptr, Font_10x18::GetInstance());
  // Set callback
  tabs.SetCallback(AppTask::GetCurrent());

  // Fill menu_items
  for(uint32_t i = 0u; i < NumberOf(menu_items); i++)
  {
    menu_items[i].text = str[i];
    menu_items[i].n = sizeof(str[i]);
  }
  // Set callback
  menu.SetCallback(AppTask::GetCurrent(), this, reinterpret_cast<CallbackPtr>(ProcessMenuOkCallback), reinterpret_cast<CallbackPtr>(ProcessMenuCancelCallback));
  // Setup menu
  menu.Setup(menu_items, NumberOf(menu_items), 0, y + tabs.GetHeight(), display_drv.GetScreenW(), height - Font_8x12::GetInstance().GetCharH() * 2u - BORDER_W*2 - tabs.GetHeight());
  // Set number of items in menu
  menu.SetCount(0);

  // Set Message Box parameters and callback
  msg_box.Setup("", "");
  msg_box.SetCallback(AppTask::GetCurrent());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result GCodeGeneratorScr::Show()
{
  // Show free memory info
  Application::GetInstance().ShowMemoryInfo();

  // Show tabs
  tabs.Show(2000);

  // If we don't have loaded script - open "Scripts" tab
  if(p_text == nullptr) tabs.SetSelectedTab(1u);
  // Call Process callback to redraw files or parameters
  ProcessCallback(&tabs);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result GCodeGeneratorScr::Hide()
{
  // Hide free memory info
  Application::GetInstance().HideMemoryInfo();

  // Hide change box if it on the screen now
  change_box.Hide();
  // Hide menu
  menu.Hide();
  // Show tabs
  tabs.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result GCodeGeneratorScr::TimerExpired(uint32_t interval)
{
  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: ProcessMenuOkCallback function   *****************************
// *****************************************************************************
Result GCodeGeneratorScr::ProcessMenuOkCallback(GCodeGeneratorScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    GCodeGeneratorScr& ths = *obj_ptr;
    // Convert pointer to index
    uint32_t idx = (uint32_t)ptr;

    // If tab 0 is selected - we in code generator mode
    if(ths.tabs.GetSelectedTab() == 0u)
    {
      if(ths.p_text != nullptr)
      {
        // Last string - Generate
        if(idx == (uint32_t)ths.interpreter.GetGlobalVariablesCnt())
        {
          // We can generate program only in IDLE or UNKNOWN state
          if((ths.grbl_comm.GetState() == GrblComm::IDLE) || (ths.grbl_comm.GetState() == GrblComm::UNKNOWN))
          {
            // Variable to store allocated size
            uint32_t size = 0u;
            // Allocate buffer for the result
            char *txt = ProgramSender::GetInstance().AllocateDataBuffer(size);

            // Set output buffer and if successful
            if(ths.interpreter.SetOutputBuf(txt, size))
            {
              // Generate GCode for ProgramSender
              if(ths.interpreter.Execute())
              {
                // Switch to the Program Sender screen if successful
                Application::GetInstance().ChangeScreen(ProgramSender::GetInstance());
              }
              else
              {
                // If unsuccessful - display message box with an error
                ths.msg_box.Setup("Error", txt);
                // Show message box with request
                ths.msg_box.Show(10000u);
              }
            }
          }
          else
          {
            // If unsuccessful - display message box with an error
            ths.msg_box.Setup("Error", "Script can be run in IDLE or UNKNOWN state");
            // Show message box with request
            ths.msg_box.Show(10000u);
          }
        }
        else
        {
          int var_val = 0;
          int32_t min_val = 0;
          int32_t max_val = 0;
          int32_t scaler = 1u;
          uint32_t precision = 0;
          // Get variable name and value
          ths.interpreter.GetGlobalVariableValue(idx, var_val);
          ths.GetGlobalVariableDescription(idx, ths.change_box_caption_str, NumberOf(ths.change_box_caption_str));
          ths.GetGlobalVariableScaler(idx, scaler);
          ths.GetGlobalVariableUnits(idx, ths.change_box_units_str, NumberOf(ths.change_box_units_str));
          ths.GetGlobalVariableMinVal(idx, min_val);
          ths.GetGlobalVariableMaxVal(idx, max_val);
          // Convert scaler to precision
          while(scaler >= 10)
          {
            scaler /= 10;
            precision++;
          }
          // Setup object to change numerical parameters, title scale set to 1
          ths.change_box.Setup(ths.change_box_caption_str, ths.change_box_units_str, var_val, min_val, max_val, precision, 1u);
          // Set AppTask
          ths.change_box.SetCallback(AppTask::GetCurrent());
          // Save axis index as ID
          ths.change_box.SetId(idx);
          // Show change box
          ths.change_box.Show(10000u);
          // Update string on display
          ths.UpdateMenuStrings();
        }
      }
    }
    else // otherwise we in a file open mode
    {
      // Buffer for the file name, filled with 0
      char fn[32u] = {0};
      // Copy directory name
      strncpy(fn, "Scripts/", NumberOf(fn));
      // Get pointer to the string after directory name
      char* pfn = &fn[strlen(fn)];
      // Get number of remaining available characters
      uint32_t cnt = NumberOf(fn) - strlen(fn);
      // Copy filename
      for(uint32_t i = 0u; i < cnt; i++)
      {
        pfn[i] = ths.menu_items[idx].text[i];
        if(pfn[i] == '\0') break;
      }
      // Go from the end of array and replace all spaces to null-terminator until
      // we found first non-space character
      for(uint32_t i = NumberOf(fn) - 1u; i > 0u; i--)
      {
        if(fn[i] <= ' ') fn[i] = '\0';
        else break;
      }

      // Open file
      FRESULT fres = f_open(&SDFile, fn, FA_OPEN_EXISTING | FA_READ);
      // Write data to file
      if(fres == FR_OK)
      {
        // Get file size
        uint32_t fsize = f_size(&SDFile) + 1u;
        // Release buffer in program sender before allocation
        ProgramSender::GetInstance().ReleaseDataPointer();
        // Allocate memory for data and check if allocation was successful
        if(ths.AllocateDataBuffer(fsize) != nullptr)
        {
          // Read bytes
          UINT wbytes = 0u;
          // Read text
          fres = f_read(&SDFile, ths.p_text, fsize, &wbytes);
          // And null-terminator to it
          ths.p_text[wbytes] = 0x00;

          // Set program buffer
          ths.interpreter.SetPgmBuffer(ths.p_text, fsize);
          // Variable to store allocated size
          uint32_t size = 0u;
          // Allocate buffer for the result
          char *txt = ProgramSender::GetInstance().AllocateDataBuffer(size);
          // Set output buffer and if successful
          ths.interpreter.SetOutputBuf(txt, size);

          // Prescan program to find all global variables and functions
          if(ths.interpreter.Prescan()) // If prescan successful
          {
            uint32_t i = 0u;
            // Copy string
            for(i = 0u; i < NumberOf(script_caption_str); i++)
            {
              // Copy character
              ths.script_caption_str[i] = ths.menu_items[idx].text[i];
              // If end of string reached or '.' character found
              if((ths.menu_items[idx].text[i] == '\0') || (ths.menu_items[idx].text[i] == '.'))
              {
                break; // break the cycle
              }
            }
            // Null-terminate it
            ths.script_caption_str[i]= '\0';
            // Set loaded script tab caption
            ths.tabs.SetText(0u, ths.script_caption_str, nullptr, Font_10x18::GetInstance());
            // Populate menu with global variables
            ths.UpdateMenuStrings();
            ths.menu.Show(100);
            ths.tabs.SetSelectedTab(0u);
            // We don't need this data pointer - it will allocate again before execution
            ProgramSender::GetInstance().ReleaseDataPointer();
          }
          else
          {
            // If prescan failed - release allocated memory
            ths.ReleaseDataPointer();
            // Display message box with an error
            ths.msg_box.Setup("Error", txt);
            // Show message box with request
            ths.msg_box.Show(10000u);
          }
        }
        else
        {
          // Display message box with an error
          ths.msg_box.Setup("Error", "Can't allocate buffer\nto read the file.\n");
          // Show message box with request
          ths.msg_box.Show(10000u);
        }
      }
      // Close file
      fres = f_close(&SDFile);
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessMenuCancelCallback function   *************************
// *****************************************************************************
Result GCodeGeneratorScr::ProcessMenuCancelCallback(GCodeGeneratorScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    GCodeGeneratorScr& ths = *obj_ptr;
    // Convert pointer to index
    uint32_t idx = (uint32_t)ptr;

    // If tab 0 is selected - we in code generator mode
    if(ths.tabs.GetSelectedTab() == 0u)
    {
      // Last string - Generate
      if(idx < (uint32_t)ths.interpreter.GetGlobalVariablesCnt())
      {
        ths.interpreter.ResetGlobalVariableValue(idx);
        ths.UpdateMenuStrings();
      }
    }
    else
    {
      // If cancel pressed - release allocated memory
      ths.ReleaseDataPointer();
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result GCodeGeneratorScr::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Process tabs
  if(ptr == &tabs)
  {
    // If "Scripts" tab is selected
    if(tabs.GetSelectedTab() == 1u)
    {
      // Stop timer to prevent queue overflow since SD card operations can take some
      // time.
      AppTask::GetCurrent()->StopTimer();

      // Reinit SD card
      BSP_SD_Init();

      // Mount SD
      FRESULT res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
      DIR dir;

      // Open the directory
      if(res == FR_OK)
      {
        res = f_opendir(&dir, "Scripts");
      }

      uint32_t idx = 0u;

      if(res == FR_OK)
      {
        FILINFO fno;
        for(;;)
        {
          // Read a directory item
          res = f_readdir(&dir, &fno);
          // Break on error or end of dir
          if(res != FR_OK || fno.fname[0] == 0) break;
          // Check extension - we want .ms or .ls
          bool add_file = false;
          // Index variable
          uint32_t i = 0u;
          // Find end of the filename
          for(; i < NumberOf(fno.fname); i++) if(fno.fname[i] == '\0') break;
          // Check extension
          for(i -= 3u; i > 0; i--)
          {
            // Check if extension is .ms* or .ls*
            if((fno.fname[i] == '.') && (tolower(fno.fname[i+2]) == 's'))
            {
              // Allow only .ms* files for mill and .ls* files for lathe
              if(((GrblComm::GetInstance().GetModeOfOperation() == GrblComm::MODE_OF_OPERATION_MILL)  && (tolower(fno.fname[i+1]) == 'm')) ||
                 ((GrblComm::GetInstance().GetModeOfOperation() == GrblComm::MODE_OF_OPERATION_LATHE) && (tolower(fno.fname[i+1]) == 'l')))
              {
                add_file = true;
                break;
              }
//              // For test only: allow both types of files at once
//              if((tolower(fno.fname[i+1]) == 'm') || (tolower(fno.fname[i+1]) == 'l'))
//              {
//                add_file = true;
//                break;
//              }
            }
          }
          // It isn't a directory
          if(!(fno.fattrib & AM_DIR) && add_file)
          {
            menu_items[idx].str.SetString(menu_items[idx].text, menu_items[idx].n, "%-19s%12lub", fno.fname, fno.fsize);
            idx++;
            if(idx == NumberOf(menu_items)) break;
          }
        }
        f_closedir(&dir);
      }
      // Fill menu_items
      for(uint32_t i = idx; i < NumberOf(menu_items); i++)
      {
        str[i][0] = '\0';
      }

      // Restart timer
      AppTask::GetCurrent()->StartTimer();

      // Set menu items count
      menu.SetCount(idx);
      // Show menu
      menu.Show(100);
    }
    else
    {
      // If we have program text(prescan successful)
      if(p_text != nullptr)
      {
        // Populate menu with global variables
        UpdateMenuStrings();
        menu.Show(100);
        tabs.SetSelectedTab(0u);
      }
      else // otherwise hide menu
      {
        menu.Hide();
      }
    }
  }
  // Process change box callback
  else if(ptr == &change_box)
  {
    // Set variable value
    interpreter.SetGlobalVariableValue(change_box.GetId(), change_box.GetValue());
    // Update strings on display
    UpdateMenuStrings();
  }
  // Process message box with an error
  else if(ptr == &msg_box)
  {
    // Release buffer in program sender after message box with an error cleared
    ProgramSender::GetInstance().ReleaseDataPointer();
  }
  else
  {
    ; // Do nothing - MISRA rule
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: UpdateMenuStrings function   *********************************
// *****************************************************************************
void GCodeGeneratorScr::UpdateMenuStrings(void)
{
  char var_str[24u] = {0};
  char val_str[16u] = {0};
  char units_str[8u] = {0};
  int32_t scaler = 1u;

  // Get total number of global variables
  uint32_t n = interpreter.GetGlobalVariablesCnt();

  uint32_t i = 0u;
  // Cycle to fill menu with variables
  for(; (i < n) && (i < NumberOf(menu_items) - 1); i++)
  {
    int var_val = 0;
    // Get variable value and description
    interpreter.GetGlobalVariableValue(i, var_val);
    GetGlobalVariableDescription(i, var_str, NumberOf(var_str));
    GetGlobalVariableScaler(i, scaler);
    GetGlobalVariableUnits(i, units_str, NumberOf(units_str));
    // Create menus string for value
    menu.CreateString(menu_items[i], var_str, grbl_comm.ValueToStringWithScalerAndUnits(val_str, NumberOf(val_str), var_val, scaler, units_str));
  }
  // Check if we have space for Generate
  if((i == n) && (i < NumberOf(menu_items) - 1))
  {
    // Add Generate menu item
    menu.CreateString(menu_items[i], "Generate");
    // Set number of items in menu
    menu.SetCount(n + 1);
  }
}

// *****************************************************************************
// ***   Private: AllocateDataBuffer   *****************************************
// *****************************************************************************
char* GCodeGeneratorScr::AllocateDataBuffer(uint32_t size)
{
  // Always release data buffer before allocate it again
  ReleaseDataPointer();
  // Allocate memory for data
  p_text = new char[size];
  // If allocation is successful
  if(p_text != nullptr)
  {
    // Add null-terminator to the first element
    p_text[0] = '\0';
  }
  // Update free memory info
  Application::GetInstance().UpdateMemoryInfo();
  // Return result
  return p_text;
}

// *****************************************************************************
// ***   Private: ReleaseDataPointer   *****************************************
// *****************************************************************************
void GCodeGeneratorScr::ReleaseDataPointer()
{
  // If buffer was previously allocated
  if(p_text != nullptr)
  {
    // Delete previously allocated buffer
    delete [] p_text;
    // Set text to nullptr
    p_text = nullptr;
  }
  // Clear program buffer
  interpreter.SetPgmBuffer(nullptr, 0);
  // Clear loaded script tab caption
  tabs.SetText(0u, "----", nullptr, Font_10x18::GetInstance());
  // Update free memory info
  Application::GetInstance().UpdateMemoryInfo();
}

// *****************************************************************************
// ***   Private: GetGlobalVariableCommentString   *****************************
// *****************************************************************************
bool GCodeGeneratorScr::GetGlobalVariableCommentString(uint32_t variable_idx, char* ptr, uint32_t n, uint32_t pos)
{
  const char* var_comment_ptr = nullptr;

  bool result = interpreter.GetGlobalVariableCommentPtr(variable_idx, var_comment_ptr);

  // If comment not found
  if(result)
  {
    uint32_t semicolon_cnt = 0u;
    // Find units start
    while((*var_comment_ptr != '\n') && (*var_comment_ptr != '\r') && (*var_comment_ptr != '\0') && (semicolon_cnt < pos))
    {
      if((*var_comment_ptr == ';')) semicolon_cnt++;
      var_comment_ptr++; // This pointer will set to next after semicolon character
    }

    // Skip any white spaces
    while((*var_comment_ptr == ' ') || (*var_comment_ptr == '\t')) var_comment_ptr++;

    // If it isn't line end
    if((*var_comment_ptr != '\n') && (*var_comment_ptr != '\r') && (*var_comment_ptr != '\0'))
    {
      // Cycle to copy variable name, but not more than passed buffer length
      for(uint32_t i = 0u; i < n; i++)
      {
        // If delimiter found
        if((var_comment_ptr[i] == ';') || (var_comment_ptr[i] == '\r') || (var_comment_ptr[i] == '\n') || (var_comment_ptr[i] == 0))
        {
          // Null-terminate buffer, set result and break the cycle
          ptr[i] = '\0';
          break;
        }
        // Copy character to buffer
        ptr[i] = var_comment_ptr[i];
      }
    }
    else
    {
      result = false;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: GetGlobalVariableDescription   *******************************
// *****************************************************************************
bool GCodeGeneratorScr::GetGlobalVariableDescription(uint32_t variable_idx, char* ptr, uint32_t n)
{
  bool result = GetGlobalVariableCommentString(variable_idx, ptr, n, 0u);

  if(result == false)
  {
    // Use variable name
    interpreter.GetGlobalVariableName(variable_idx, ptr, n);
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: GetGlobalVariableUnits   *************************************
// *****************************************************************************
bool GCodeGeneratorScr::GetGlobalVariableUnits(uint32_t variable_idx, char* ptr, uint32_t n)
{
  bool result = GetGlobalVariableCommentString(variable_idx, ptr, n, 2u);

  if(result == false)
  {
    // No units
    ptr[0] = '\0';
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: GetGlobalVariableCommentNumber   *****************************
// *****************************************************************************
bool GCodeGeneratorScr::GetGlobalVariableCommentNumber(uint32_t variable_idx, int32_t& value, uint32_t pos)
{
  const char* var_comment_ptr = nullptr;

  bool result = interpreter.GetGlobalVariableCommentPtr(variable_idx, var_comment_ptr);

  // If comment not found
  if(result)
  {
    uint32_t semicolon_cnt = 0u;
    // Find units start
    while((*var_comment_ptr != '\n') && (*var_comment_ptr != '\r') && (*var_comment_ptr != '\0') && (semicolon_cnt < pos))
    {
      if((*var_comment_ptr == ';')) semicolon_cnt++;
      var_comment_ptr++; // This pointer will set to next after semicolon_cnt character
    }

    // If it isn't line end
    if((*var_comment_ptr != '\n') && (*var_comment_ptr != '\r') && (*var_comment_ptr != '\0'))
    {
      // Convert string to number
      value = atoi(var_comment_ptr);
    }
    else
    {
      result = false;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: GetGlobalVariableScaler   ************************************
// *****************************************************************************
bool GCodeGeneratorScr::GetGlobalVariableScaler(uint32_t variable_idx, int32_t& scaler)
{
  bool result = GetGlobalVariableCommentNumber(variable_idx, scaler, 1u);

  if(result == false)
  {
    // Set scaler to 1
    scaler = 1;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: GetGlobalVariableMinVal   ************************************
// *****************************************************************************
bool GCodeGeneratorScr::GetGlobalVariableMinVal(uint32_t variable_idx, int32_t& min)
{
  bool result = GetGlobalVariableCommentNumber(variable_idx, min, 3u);

  if(result == false)
  {
    // Set minimum value to -10000000
    min = -10000000;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: GetGlobalVariableMaxVal   ************************************
// *****************************************************************************
bool GCodeGeneratorScr::GetGlobalVariableMaxVal(uint32_t variable_idx, int32_t& max)
{
  bool result = GetGlobalVariableCommentNumber(variable_idx, max, 4u);

  if(result == false)
  {
    // Set maximum value to 10000000
    max = 10000000;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private constructor   *************************************************
// *****************************************************************************
GCodeGeneratorScr::GCodeGeneratorScr() : change_box(Application::GetInstance().GetChangeValueBox()) {};
