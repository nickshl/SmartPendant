//******************************************************************************
//  @file ProgramSender.cpp
//  @author Nicolai Shlapunov
//
//  @details ProgramSender: User ProgramSender Class, implementation
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
#include "ProgramSender.h"
#include "Application.h"

#include "fatfs.h"
#include <cctype> // For tolower()
#include <new>    // For std::nothrow

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
ProgramSender& ProgramSender::GetInstance()
{
  static ProgramSender pgmsenderscr;
  return pgmsenderscr;
}

// *****************************************************************************
// ***   ProgramSender Setup   *************************************************
// *****************************************************************************
Result ProgramSender::Setup(int32_t y, int32_t height)
{
  constexpr int32_t CTRL_HEIGHT = 40;

  // Fill menu_items
  for(uint32_t i = 0u; i < NumberOf(menu_items); i++)
  {
    menu_items[i].text = str[i];
    menu_items[i].n = sizeof(str[i]);
  }
  // Set callback
  menu.SetCallback(AppTask::GetCurrent(), this, reinterpret_cast<CallbackPtr>(ProcessMenuOkCallback), reinterpret_cast<CallbackPtr>(ProcessMenuCancelCallback));
  // Setup menu
  menu.Setup(menu_items, NumberOf(menu_items), 0, y, display_drv.GetScreenW(), height - Font_8x12::GetInstance().GetCharH() * 2u - BORDER_W * 2);
  // Setup text box
  text_box.Setup(0, y, display_drv.GetScreenW(), height - Font_8x12::GetInstance().GetCharH() * 2u - BORDER_W * 2 - CTRL_HEIGHT);

  // Feed override
  feed_dw.SetParams(BORDER_W, y + height - Font_8x12::GetInstance().GetCharH() * 2u - BORDER_W - BORDER_W - CTRL_HEIGHT, (display_drv.GetScreenW() - 2 * CTRL_HEIGHT - 3 * BORDER_W) / 2, CTRL_HEIGHT, 5u, 0u);
  feed_dw.SetBorder(BORDER_W, COLOR_DARKBLUE);
  feed_dw.SetDataFont(Font_12x16::GetInstance());
  feed_dw.SetNumber(0);
  feed_dw.SetUnits("%", DataWindow::RIGHT);
  feed_dw.SetCallback(AppTask::GetCurrent());
  feed_name.SetParams("FEED", feed_dw.GetStartX() + BORDER_W * 3 / 2, feed_dw.GetStartY() + BORDER_W * 3 / 2, COLOR_WHITE, Font_6x8::GetInstance());
  // Speed override
  speed_dw.SetParams(feed_dw.GetEndX() + BORDER_W, feed_dw.GetStartY(), feed_dw.GetWidth(), feed_dw.GetHeight(), 5u, 0u);
  speed_dw.SetBorder(BORDER_W, COLOR_DARKBLUE);
  speed_dw.SetDataFont(Font_12x16::GetInstance());
  speed_dw.SetNumber(0);
  speed_dw.SetUnits("%", DataWindow::RIGHT);
  speed_dw.SetCallback(AppTask::GetCurrent());
  speed_name.SetParams("SPEED", speed_dw.GetStartX() + BORDER_W * 3 / 2, speed_dw.GetStartY() + BORDER_W * 3 / 2, COLOR_WHITE, Font_6x8::GetInstance());
  // Buttons for control flood coolant
  flood_btn.SetParams("F", speed_dw.GetEndX() + BORDER_W, speed_dw.GetStartY(), speed_dw.GetHeight(), speed_dw.GetHeight(), true);
  flood_btn.SetFont(Font_12x16::GetInstance());
  flood_btn.SetCallback(AppTask::GetCurrent());
  flood_btn.Disable();
  // Buttons for control mist coolant
  mist_btn.SetParams("M", flood_btn.GetEndX() + BORDER_W, speed_dw.GetStartY(), speed_dw.GetHeight(), speed_dw.GetHeight(), true);
  mist_btn.SetFont(Font_12x16::GetInstance());
  mist_btn.SetCallback(AppTask::GetCurrent());
  mist_btn.Disable();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result ProgramSender::Show()
{
  // Set encoder callback handler(before menu show since menu will handle it also)
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);

  // Show free memory info
  Application::GetInstance().ShowMemoryInfo();

  // Generated program can have its own line numbers too: they are removed,
  // lines are numbered when they are sent
  StripLineNumbers(p_text);
  // Update text - in case it is generated, we have to count lines
  if(!text_box.SetText(p_text))
  {
    // If program contains lines longer than 80 characters - show message
    // instead, otherwise lines would be silently truncated during streaming
    text_box.SetText("; Program contain lines longer\n\r; than 80 characters");
  }
  // Selector for this text
  ResetSelector();
  // Show text box
  text_box.Show(100);

  // Axis data
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(3u); i++)
  {
    DataWindow& dw_real = Application::GetInstance().GetRealDataWindow(i);
    String& dw_real_name = Application::GetInstance().GetRealDataWindowNameString(i);

    // Real position
    dw_real.SetParams(BORDER_W + ((display_drv.GetScreenW() - BORDER_W * 4) / 3 + BORDER_W) * i, feed_dw.GetEndY() + BORDER_W, (display_drv.GetScreenW() - BORDER_W * 4) / 3, Font_8x12::GetInstance().GetCharH() * 2u, 8u, grbl_comm.GetReportUnitsPrecision(i));
    dw_real.SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real.SetDataFont(Font_8x12::GetInstance());
    dw_real.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT, Font_6x8::GetInstance());
    // Axis Name
    dw_real_name.SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_6x8::GetInstance());
    dw_real_name.Move(dw_real.GetStartX() + BORDER_W, dw_real.GetStartY() + BORDER_W);

    dw_real.Show(100);
    dw_real_name.Show(100);
  }

  // Reinit all three Soft Buttons
  Application::GetInstance().InitSoftButtons(true);

  // Run button
  left_btn.SetString("Run");
  left_btn.Show(102);
  // Open button
  middle_btn.SetString("Open");
  middle_btn.Show(102);
  // Stop button
  right_btn.SetString("Stop");
  right_btn.Show(102);

  // Feed objects
  feed_dw.Show(100);
  feed_name.Show(100);
  // Speed objects
  speed_dw.Show(100);
  speed_name.Show(100);
  // Coolant buttons
  flood_btn.Show(100);
  mist_btn.Show(100);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result ProgramSender::Hide()
{
  // Stop streaming. TimerExpired() isn't called for a hidden screen, so
  // streaming can't continue anyway, and Show() resets the text box selection
  // to the first line: leaving the run flag set would restart the program
  // from the beginning without a Run press when the user returns to the
  // screen(spontaneous spindle/motion start).
  run = false;
  finished = true;

  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);

  // Hide free memory info
  Application::GetInstance().HideMemoryInfo();

  // Hide menu
  menu.Hide();
  // Hide text box
  text_box.Hide();

  // We may have file open, close it and clear text box
  if(p_text == nullptr)
  {
    CloseFiles();
  }

  // Axis data
  for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
  {
    Application::GetInstance().GetRealDataWindow(i).Hide();
    Application::GetInstance().GetRealDataWindowNameString(i).Hide();
  }

  // Go button
  left_btn.Hide();
  // Open button
  middle_btn.Hide();
  // Reset button
  right_btn.Hide();

  // Feed objects
  feed_dw.Hide();
  feed_name.Hide();
  // Speed objects
  speed_dw.Hide();
  speed_name.Hide();
  // Coolant buttons
  flood_btn.Hide();
  mist_btn.Hide();

  // Reinit Soft Buttons to change their size back
  Application::GetInstance().InitSoftButtons(false);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result ProgramSender::TimerExpired(uint32_t interval)
{
  // Update left & right button text
  Application::GetInstance().UpdateLeftButtonText();
  Application::GetInstance().UpdateRightButtonText();

  // Update numbers with current overrides
  feed_dw.SetNumber(grbl_comm.GetFeedOverride());
  speed_dw.SetNumber(grbl_comm.GetSpeedOverride());
  // Set coolant state
  flood_btn.SetColor(grbl_comm.GetCoolantFlood() ? COLOR_GREEN : COLOR_WHITE);
  mist_btn.SetColor(grbl_comm.GetCoolantMist() ? COLOR_GREEN : COLOR_WHITE);

  if(run)
  {
    // Process speed & feed change
    ProcessSpeedFeed();

    // We should stream program if state is Idle, Run or Hold and we in control
    if(((grbl_comm.GetState() == GrblComm::IDLE) || (grbl_comm.GetState() == GrblComm::RUN) || (grbl_comm.GetState() == GrblComm::HOLD)) && (grbl_comm.IsInControl()))
    {
      // If we finished streaming
      if(finished)
      {
        // Wait until IDLE state
        if(grbl_comm.GetState() == GrblComm::IDLE)
        {
          // Then clear run flag
          run = false;
          // Everything is executed, including the last lines without motion:
          // controller may not report line numbers for those
          exec_line = send_line;
          // And selector shows the executed line in any mode
          text_box.SetSelectorFill(0u);
        }
      }
      else
      {
        // If ID is zero - we didn't send any commands yet
        GrblComm::status_t result = (id != 0u) ? grbl_comm.GetCmdResult(id) : GrblComm::Status_OK;
        // Only an acknowledged command permits the next line. A superseded
        // result is unknown, including commands invalidated by Stop or reset.
        if(result == GrblComm::Status_OK)
        {
          // The first numbered line is acknowledged: controller has parsed
          // it, so every line number it reports from now on belongs to this
          // program. Forget the number we have - it can be left by the
          // previous program - and use the ones that come after.
          if(!line_number_valid && (id != 0u) && (id == first_numbered_id))
          {
            grbl_comm.ClearLineNumber();
            line_number_valid = true;
          }
          // Take lines from the program until there is something to send:
          // a line with nothing but a comment isn't sent(but it is counted,
          // line numbers are positions in the program). Limited per call to
          // not to stall the task on a long block of comments.
          for(uint32_t i = 0u; !cmd_ready && !finished && (i < MAX_LINES_PER_TICK); i++)
          {
            // Buffer for line: 80 + CR + LF + \0
            char line[128] = {0};
            // Get next line, it also handles the end of program and errors
            if(GetNextLine(line, NumberOf(line)))
            {
              send_line++;
              cmd_ready = BuildCommand(line, send_line, cmd, NumberOf(cmd));
            }
          }
          // Send command. It is kept until accepted: the line is already
          // taken from the program and can't be taken again.
          if(cmd_ready && (grbl_comm.SendCmd(cmd, id) == Result::RESULT_OK))
          {
            cmd_ready = false;
            // Is it the first line that carries a number? A line with block
            // delete character doesn't count: controller may skip it.
            if((first_numbered_id == 0u) && (cmd[0] == 'N')) first_numbered_id = id;
          }
        }
        else if(result == GrblComm::Status_Cmd_Not_Executed_Yet)
        {
          ; // Wait until command will be executed
        }
        else // In case of any error - stop executing program
        {
          // Clear run flag
          run = false;
          // Set finished flag to prevent further streaming attempts
          finished = true;
          // The other stop paths in this function explain themselves, and a
          // link failure is the most confusing one to hit: the machine simply
          // stops mid-program. The operator has to know the rest was skipped,
          // and that the last line's fate is unknown - the controller may have
          // executed it and lost the acknowledgement.
          msg_box.Setup("PROGRAM STOPPED", "Command was not acknowledged\nby the controller. Remaining\nprogram was skipped.\nCheck machine position\nbefore resuming.", 1u);
          msg_box.Show(10000u);
        }
      }
      // Move selection to the line that is executed. A streamed program is
      // read from SD card for every line, so it goes a few lines per call -
      // unless program has just ended: there will be no other call. Program
      // in memory goes all the way.
      UpdateShownLine((run && (text_box.GetText() == nullptr)) ? MAX_LINES_PER_TICK : 0xFFFFu);
    }
    else
    {
      // In case of any unexpected error - stop the program
      run = false;
    }
  }
  else if(grbl_comm.GetState() == GrblComm::RUN) // If we finished program, but controller still running
  {
    // Process speed & feed change
    ProcessSpeedFeed();
  }
  else
  {
    // Safety measure: allow run program only from the beginning and only
    // if there is a program to run. Error of the previous command has to be
    // cleared first(Stop/Reset/Unlock): until then controller gets nothing.
    // TODO: add dialog box "Are you sure you want to run program from current position?" instead
    // A message in line mode isn't a program: streamed program has the file
    // for the text box open.
    if((text_box.GetSelect() == 0) && (text_box.GetNumberOfLines() > 0) && ((text_box.GetText() != nullptr) || (p_disp_file != nullptr)) && (grbl_comm.GetStatusCode() == GrblComm::Status_OK))
    {
      left_btn.Enable();
    }
    else
    {
      left_btn.Disable();
    }
    // If feed control enabled - disable it
    if(feed_dw.IsActive())
    {
      feed_dw.SetActive(false);
      feed_dw.SetBorder(BORDER_W, COLOR_DARKBLUE);
      feed_dw.SetSelected(false);
    }
    // If speed control enabled - disable it
    if(speed_dw.IsActive())
    {
      speed_dw.SetActive(false);
      speed_dw.SetBorder(BORDER_W, COLOR_DARKBLUE);
      speed_dw.SetSelected(false);
    }
    // Disable coolant control
    flood_btn.Disable();
    mist_btn.Disable();
    // Enable buttons back
    middle_btn.Enable();
    // Enable screen change if program isn't running
    Application::GetInstance().EnableScreenChange();
    // If encoder turned and we have program in memory
    if((enc_val != 0) && (p_text != nullptr))
    {
      // Select line
      text_box.Select(text_box.GetSelect() + enc_val);
      //text_box.Scroll(text_box.GetScroll() + enc_val);
      // Clear encoder value
      enc_val = 0;
    }
  }

  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: GetNextLine function   ***************************************
// *****************************************************************************
bool ProgramSender::GetNextLine(char* line, uint32_t size)
{
  bool result = false;

  // Program is streamed from SD card
  if(text_box.GetText() == nullptr)
  {
    // Read line from file
    if(f_gets(line, size, &SDFile) != nullptr)
    {
      // Null-terminate just in case
      line[size - 1u] = '\0';
      // If we read line longer than the limit + possible CR & LF characters
      if(strlen(line) > TextBox::MAX_LINE_LEN + 2u)
      {
        // Stop streaming - silently skipping the rest of the
        // program is dangerous on a CNC, operator must know.
        run = false;
        // Set finished flag to prevent further streaming attempts
        finished = true;
        // Rewind to the end of file so program can't be continued
        f_lseek(&SDFile, SDFile.obj.objsize);
        // And in the message box
        msg_box.Setup("PROGRAM STOPPED", "Line longer than 80 characters\nencountered during streaming.\nRemaining program was skipped.", 1u);
        msg_box.Show(10000u);
      }
      else
      {
        // The same line is shown without its own line number - see ShowNextLine()
        StripLineNumbers(line);
        result = true;
      }
    }
    else if(f_eof(&SDFile))
    {
      // End of program
      finished = true;
    }
    else
    {
      // Read failed but end of file isn't reached: SD error or
      // file isn't open anymore. Stop streaming.
      run = false;
      // Set finished flag to prevent further streaming attempts
      finished = true;
      // And in the message box
      msg_box.Setup("PROGRAM STOPPED", "File read error encountered\nduring streaming.\nRemaining program was skipped.", 1u);
      msg_box.Show(10000u);
    }
  }
  else // Program is in memory
  {
    // Lines are counted the way text box does it: empty lines don't exist
    while((p_send != nullptr) && ((*p_send == '\n') || (*p_send == '\r'))) p_send++;
    // Check end of program
    if((p_send == nullptr) || (*p_send == '\0'))
    {
      finished = true;
    }
    else
    {
      uint32_t i = 0u;
      // Copy line
      for(; (*p_send != '\n') && (*p_send != '\r') && (*p_send != '\0'); p_send++)
      {
        if(i < size - 1u) line[i++] = *p_send;
      }
      // Null-terminate it
      line[i] = '\0';
      result = true;
    }
  }

  return result;
}

// *****************************************************************************
// ***   Private: ShowNextLine function   **************************************
// *****************************************************************************
bool ProgramSender::ShowNextLine()
{
  int32_t select = text_box.GetSelect();
  int32_t scroll = text_box.GetScroll();

  // Program is streamed from SD card: text box holds visible lines only
  if(text_box.GetText() == nullptr)
  {
    // Buffer to read string 80 + 2 + 1
    char str[128] = {0};
    // If we did not passed half the screen or if there is nothing to add
    // and we need to go through remaining lines
    if((select < text_box.GetNumberOfVisibleLines() / 2) || disp_end)
    {
      // Go to next line
      text_box.Select(select + 1);
    }
    // Otherwise selection stays in place and lines move: read the next one.
    // File is open twice. SDFile is read for sending and runs ahead by as
    // many lines as controller holds; this position follows the line that
    // is shown. Both only go forward, no seeks are needed.
    else if((p_disp_file != nullptr) && (f_gets(str, NumberOf(str), p_disp_file) != nullptr))
    {
      // Null-terminate just in case
      str[NumberOf(str) - 1] = '\0';
      // Show it without its own line number: the one that is sent and
      // reported back by controller is the position in the program
      StripLineNumbers(str);
      // Set this line to text_box
      text_box.AddLine(str);
      // Selection stays on the same row, but it is the next line now
      select--;
    }
    else
    {
      // End of file. Read error ends here as well: it is only the picture,
      // program must not be stopped because of it.
      disp_end = true;
      // Go to next line
      text_box.Select(select + 1);
    }
  }
  else // Program is in memory
  {
    // If we half past screen
    if(select - scroll >= text_box.GetNumberOfVisibleLines() / 2)
    {
      // Scroll to to see next lines to see what will send next
      text_box.Scroll(scroll + 1);
    }
    // Go to next line
    text_box.Select(select + 1);
  }

  // Can't go further - end of program
  bool moved = (select != text_box.GetSelect());
  // Count line
  if(moved) shown_line++;

  return moved;
}

// *****************************************************************************
// ***   Private: ResetSelector function   *************************************
// *****************************************************************************
void ProgramSender::ResetSelector()
{
  // Blue for a program in memory, red for a streamed one
  text_box.SetSelectorColor((text_box.GetText() == nullptr) ? COLOR_RED : COLOR_BLUE);
  // Filled: nothing is running yet
  text_box.SetSelectorFill(0u);
}

// *****************************************************************************
// ***   Private: UpdateShownLine function   ***********************************
// *****************************************************************************
void ProgramSender::UpdateShownLine(uint32_t max_lines)
{
  // Line to show. By default - the one that is shown already.
  uint32_t target = shown_line;

  // Controller executes lines later than it receives them: it keeps tens of
  // them in its planner. Lines are sent with N word(see BuildCommand()) and
  // controller reports number of the line it is executing in status report
  // if it is enabled in $10(it is by default) - line_mode is set from it when
  // Run is pressed.
  if(line_mode == LINE_EXECUTED)
  {
    // Number is used only after the first numbered line is acknowledged, see
    // TimerExpired(). A report without number changes nothing: controller
    // may have nothing to report between motions.
    uint32_t line_number = line_number_valid ? grbl_comm.GetLineNumber() : 0u;
    // Number has to be one of ours. Never go back: lines that passed aren't
    // kept for a streamed program.
    if((line_number <= send_line) && (line_number > exec_line)) exec_line = line_number;
    // Show the line that is executed
    target = exec_line;
  }
  else // line_mode == LINE_SENT
  {
    // No line numbers from controller - show the line that will be sent next
    target = send_line + 1u;
  }

  // Go to the line, but not too many at once: for a streamed program every
  // line is read from SD card
  for(; (shown_line < target) && (max_lines != 0u); max_lines--)
  {
    if(!ShowNextLine()) break;
  }
}

// *****************************************************************************
// ***   Private: StripLineNumbers function   **********************************
// *****************************************************************************
void ProgramSender::StripLineNumbers(char* text)
{
  const char* src = text;
  char* dst = text;
  // State of the current line
  bool line_start = true;   // Nothing but spaces so far
  bool keep_line = false;   // Not a g-code - leave it as it is
  bool semicolon = false;   // Inside of ; comment
  bool parentheses = false; // Inside of ( ) comment
  bool name = false;        // Inside of < > name of parameter or subroutine

  if(text != nullptr)
  {
    while(*src != '\0')
    {
      char c = *src;
      // End of line - new line starts from scratch
      if((c == '\n') || (c == '\r'))
      {
        line_start = true;
        keep_line = semicolon = parentheses = name = false;
      }
      else
      {
        // First character of the line. System commands are '$' and '[':
        // "$N0=G54" is a startup line, not a line number.
        if(line_start && (c != ' ') && (c != '\t'))
        {
          line_start = false;
          keep_line = (c == '$') || (c == '[');
        }
        if(keep_line || semicolon)
        {
          ; // Nothing to look for till the end of line
        }
        else if(parentheses)
        {
          if(c == ')') parentheses = false;
        }
        else if(name)
        {
          if(c == '>') name = false;
        }
        else if(c == ';')
        {
          semicolon = true;
        }
        else if(c == '(')
        {
          parentheses = true;
        }
        else if(c == '<')
        {
          name = true;
        }
        else if(((c == 'N') || (c == 'n')) && (src[1] >= '0') && (src[1] <= '9'))
        {
          // Line number: skip the letter and all digits
          src++;
          while((*src >= '0') && (*src <= '9')) src++;
          // Skip the space after it too if there is nothing or a space before
          if((*src == ' ') && ((dst == text) || (dst[-1] == ' ') || (dst[-1] == '/') || (dst[-1] == '\n') || (dst[-1] == '\r'))) src++;
          // Nothing to copy
          continue;
        }
        else
        {
          ; // Do nothing - MISRA rule
        }
      }
      // Copy character
      *dst++ = c;
      src++;
    }
    // Null-terminate result
    *dst = '\0';
  }
}

// *****************************************************************************
// ***   Private: BuildCommand function   **************************************
// *****************************************************************************
bool ProgramSender::BuildCommand(const char* line, uint32_t number, char* cmd, uint32_t size)
{
  bool result = false;
  uint32_t n = 0u;

  // Skip leading spaces
  while((*line == ' ') || (*line == '\t')) line++;

  // System commands and program start/end mark aren't g-code: send as is
  if((*line == '$') || (*line == '[') || (*line == '%'))
  {
    for(; (*line != '\0') && (*line != '\n') && (*line != '\r') && (n < size - 2u); line++) cmd[n++] = *line;
    result = true;
  }
  else
  {
    // Block delete character have to stay first: controller skips the line
    // if block delete switch is on
    if(*line == '/')
    {
      cmd[n++] = '/';
      line++;
    }
    // Line number goes first: it is the only place where controller takes
    // it for every kind of line(flow control lines are parsed differently
    // after O word)
    // Controller refuses numbers above its limit: lines after that go without
    if(number <= MAX_LINE_NUMBER) n += snprintf(&cmd[n], size - n, "N%lu", (unsigned long)number);
    // Position where the line itself starts
    uint32_t start = n;
    bool parentheses = false;
    // Copy line without ';' comment: controller only cuts it off. Comments in
    // parentheses stay: controller handles them - (MSG,...) is shown to the
    // operator, plugins can act on others.
    for(; (*line != '\0') && (*line != '\n') && (*line != '\r') && (n < size - 2u); line++)
    {
      // Comment till the end of line, but ';' inside parentheses is a text
      if((*line == ';') && !parentheses) break;
      // Track comment in parentheses
      if(*line == '(')
      {
        parentheses = true;
      }
      else if(*line == ')')
      {
        parentheses = false;
      }
      else
      {
        ; // Do nothing - MISRA rule
      }
      cmd[n++] = *line;
    }
    // Remove trailing spaces
    while((n > start) && ((cmd[n - 1u] == ' ') || (cmd[n - 1u] == '\t'))) n--;
    // If nothing left - there is nothing to send
    result = (n > start);
  }
  // Since CR and LF are stripped out, we have to add it
  cmd[n++] = '\r';
  cmd[n] = '\0';

  return result;
}

// *************************************************************************
// ***   Private: ProcessSpeedFeed function   ******************************
// *************************************************************************
Result ProgramSender::ProcessSpeedFeed()
{
  Result result = Result::RESULT_OK;

  // Update feed if necessary. One step at a timer tick.
  if(feed_val > 0)
  {
    if(feed_val > 10)
    {
      result = grbl_comm.FeedCoarsePlus();
      feed_val -= 10;
    }
    else
    {
      result = grbl_comm.FeedFinePlus();
      feed_val--;
    }
  }
  else if(feed_val < 0)
  {
    if(feed_val < -10)
    {
      result = grbl_comm.FeedCoarseMinus();
      feed_val += 10;
    }
    else
    {
      result = grbl_comm.FeedFineMinus();
      feed_val++;
    }
  }
  else
  {
    ; // Do nothing - MISRA rule
  }

  // Update speed if necessary. One step at a timer tick.
  if(speed_val > 0)
  {
    if(speed_val > 10)
    {
      result = grbl_comm.SpeedCoarsePlus();
      speed_val -= 10;
    }
    else
    {
      result = grbl_comm.SpeedFinePlus();
      speed_val--;
    }
  }
  else if(speed_val < 0)
  {
    if(speed_val < -10)
    {
      result = grbl_comm.SpeedCoarseMinus();
      speed_val += 10;
    }
    else
    {
      result = grbl_comm.SpeedFineMinus();
      speed_val++;
    }
  }
  else
  {
    ; // Do nothing - MISRA rule
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   IsProgramFile function(file scope)   **********************************
// *****************************************************************************
// * Returns true if directory entry is a program file(.nc*, .gc* or .tap
// * extension, not a directory). Used by the menu fill and the open handler:
// * both must use the same filter, since the file is found by its index.
static bool IsProgramFile(const FILINFO& fno)
{
  // Check extension - we want .gc* or .nc*
  bool add_file = false;
  // Index variable
  uint32_t i = 0u;
  // Find end of the filename
  for(; i < NumberOf(fno.fname); i++) if(fno.fname[i] == '\0') break;
  // Check extension (only if the name is long enough, otherwise i -= 3u underflows)
  for(i -= ((i >= 3u) ? 3u : 0u); i > 0; i--)
  {
    // Find first '.' from the end to find extension
    if(fno.fname[i] == '.')
    {
      // Check if extension is .nc* or .gc*
      if(((tolower(fno.fname[i+1]) == 'g') || (tolower(fno.fname[i+1]) == 'n')) && (tolower(fno.fname[i+2]) == 'c'))
      {
        add_file = true;
      }
      // Check if extension is .tap
      else if((tolower(fno.fname[i+1]) == 't') && (tolower(fno.fname[i+2]) == 'a') && (tolower(fno.fname[i+3]) == 'p'))
      {
        add_file = true;
      }
      else
      {
        ; // Do nothing - MISRA rule
      }
      // There no point to check for another point since extension can be only after last one
      break;
    }
  }
  // It should be a file with the proper extension, not a directory
  return (add_file && !(fno.fattrib & AM_DIR));
}

// *****************************************************************************
// ***   Private: ProcessMenuOkCallback function   *****************************
// *****************************************************************************
Result ProgramSender::ProcessMenuOkCallback(ProgramSender* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProgramSender& ths = *obj_ptr;

    // Hide the menu
    ths.menu.Hide();

    // Selected menu index
    uint32_t sel_idx = (uint32_t)ptr;

    // Find the file by rescanning the directory instead of parsing the name
    // back out of the padded menu text: the menu shows only 19 characters of
    // the name, so a longer name either fails to open or - worse - another
    // file matching the truncated prefix could be opened and run.
    FILINFO fno;
    DIR dir;
    // Found flag
    bool found = false;

    // Open the root directory
    if(f_opendir(&dir, "/") == FR_OK)
    {
      // Index of the current program file
      uint32_t file_idx = 0u;
      for(;;)
      {
        // Read a directory item, stop on error or end of dir
        if((f_readdir(&dir, &fno) != FR_OK) || (fno.fname[0] == 0)) break;
        // Count only program files - the same filter the menu fill uses,
        // so indexes match the menu positions
        if(IsProgramFile(fno))
        {
          // Check if it is the selected one
          if(file_idx == sel_idx)
          {
            found = true;
            break;
          }
          file_idx++;
        }
      }
      f_closedir(&dir);
    }

    // Verify the found file against the displayed menu string: directory
    // content could change since the list was shown(card swap) and the
    // "-- Too many files! --" marker doesn't correspond to a file at all.
    if(found && (sel_idx < NumberOf(ths.menu_items)))
    {
      // Buffer for the check string - same size as the menu item text
      char check_str[32u + 1u];
      snprintf(check_str, NumberOf(check_str), "%-19.19s%12lub", fno.fname, fno.fsize);
      // Clear flag if it doesn't match the menu item
      if(strcmp(check_str, ths.menu_items[sel_idx].text) != 0) found = false;
    }
    else
    {
      found = false;
    }

    // Open file
    FRESULT fres = found ? f_open(&SDFile, fno.fname, FA_OPEN_EXISTING | FA_READ) : FR_NO_FILE;
    // Write data to file
    if(fres == FR_OK)
    {
      // Get file size
      uint32_t fsize = f_size(&SDFile) + 1u;
      // Allocate memory for data
      ths.AllocateDataBuffer(fsize);
      // Check if allocation was successful
      if(ths.p_text != nullptr)
      {
        // Read bytes
        UINT wbytes = 0u;
        // Read text
        fres = f_read(&SDFile, ths.p_text, fsize, &wbytes);
        // And null-terminator to it
        ths.p_text[wbytes] = 0x00;
        // Program is kept and shown without its own line numbers: lines are
        // numbered when they are sent, and it is those numbers that
        // controller reports back
        StripLineNumbers(ths.p_text);
        // Check read result: on SD error f_read() can return partial data
        // which would be displayed and runnable as a valid program with the
        // tail(possibly mid-line) missing
        if((fres != FR_OK) || (wbytes != fsize - 1u))
        {
          // Release partially read data
          ths.ReleaseDataPointer();
          // Show an error message instead of the partially read program
          ths.text_box.SetText("; File read error!");
        }
        // Set text to text box
        else if(!ths.text_box.SetText(ths.p_text))
        {
          // If program contains lines longer than 80 characters - show message
          ths.text_box.SetText("; Program contain lines longer\n\r; than 80 characters");
        }
        else
        {
          ; // Do nothing - MISRA rule
        }
        // Close file
        fres = f_close(&SDFile);
      }
      else
      {
        // Show message before the check: file is big(it doesn't fit into
        // memory) and check can take a few seconds. Display task will render
        // it while this task is busy reading the file.
        ths.msg_box.Setup("CHECKING PROGRAM", "If program contains lines\nlonger than 80 characters\nit can't be loaded", 1u);
        ths.msg_box.SetModal(true);
        ths.msg_box.Show(10000u);
        // Update Display
        ths.display_drv.UpdateDisplay();
        // Delay to let DisplayDrv to actually show message
        RtosTick::DelayMs(50u);

        // Current line number(1-based) for error reporting
        uint32_t line_n = 1u;
        // Current line length(line ending characters aren't counted)
        uint32_t line_len = 0u;
        // Number of the first line that is too long(0 - all lines are ok)
        uint32_t long_line_n = 0u;
        // Read bytes count
        UINT rb = 0u;
        // Read result: must be checked after the cycle - an error mid-scan
        // leaves the tail of the file unchecked and must not pass the check
        FRESULT check_res = FR_OK;
        // Chunk buffer: chunked f_read() is much faster than byte by byte f_gets()
        char chunk[256u];

        // Walk through the whole file to find lines longer than the line
        // buffer: discovering such line mid-run would stop the program(see
        // TimerExpired()), so it is better to refuse the file at open.
        while(((check_res = f_read(&SDFile, chunk, NumberOf(chunk), &rb)) == FR_OK) && (rb > 0u))
        {
          for(uint32_t i = 0u; i < rb; i++)
          {
            if(chunk[i] == '\n')
            {
              // End of line - count it and reset length
              line_n++;
              line_len = 0u;
            }
            else if(chunk[i] != '\r')
            {
              // Count content character and check the limit
              line_len++;
              if(line_len > TextBox::MAX_LINE_LEN)
              {
                long_line_n = line_n;
                break;
              }
            }
            else
            {
              ; // Do nothing - MISRA rule
            }
          }
          // Break outer cycle if too long line is found
          if(long_line_n != 0u) break;
        }

        // Hide message box
        ths.msg_box.Hide();

        // If read failed mid-scan: the unchecked tail can still contain a
        // long line - the exact situation this check exists to prevent, so
        // the file must be refused, not treated as checked.
        if(check_res != FR_OK)
        {
          // Close file - program can't be streamed safely
          f_close(&SDFile);
          // Clear text buffer
          ths.text_box.SetText(nullptr);
          // Show the reason
          ths.text_box.AddLine("; File read error");
        }
        // If program contains a line that is too long
        else if(long_line_n != 0u)
        {
          // Close file - program can't be streamed safely
          f_close(&SDFile);
          // Clear text buffer to switch into line mode
          ths.text_box.SetText(nullptr);
          // Show the reason with the line number. AddLine() copies the
          // string, so local buffer is ok there.
          char err_str[32u];
          snprintf(err_str, NumberOf(err_str), "; Line %lu is longer", long_line_n);
          ths.text_box.AddLine(err_str);
          ths.text_box.AddLine("; than 80 characters!");
        }
        else
        {
          // Rewind file back to the beginning after the check: lines for
          // sending are read from there
          f_lseek(&SDFile, 0u);

          // Clear text buffer to switch into line mode
          ths.text_box.SetText(nullptr);

          // Open the same file once more to have the second position in it:
          // text box shows the line that is executed, which is far behind
          // the line that is sent. Both are opened for reading only.
          ths.disp_end = false;
          // File object is allocated: see p_disp_file. Program itself isn't
          // in memory in this mode, so there is room for it.
          ths.p_disp_file = new(std::nothrow) FIL;
          if((ths.p_disp_file == nullptr) || (f_open(ths.p_disp_file, fno.fname, FA_OPEN_EXISTING | FA_READ) != FR_OK))
          {
            // Close file - we can't continue
            ths.CloseFiles();
            // Show message
            ths.text_box.SetText("; File read error");
          }
          else
          {
            // Buffer to read string
            char str[128] = {0};
            // Fill all visible lines
            for(int32_t i = 0; i < ths.text_box.GetNumberOfVisibleLines(); i++)
            {
              // Read line from file, break the cycle at the end of file
              if(f_gets(str, NumberOf(str), ths.p_disp_file) == nullptr)
              {
                // Close file - we can't continue
                ths.CloseFiles();
                // Show message
                ths.text_box.SetText("; File read error");
                // Break the cycle
                break;
              }
              // Null-terminate just in case
              str[NumberOf(str) - 1] = '\0';
              // If we read line longer than the limit + possible CR & LF
              // characters. Should never happen after the check above - kept
              // as a backstop.
              if(strlen(str) > TextBox::MAX_LINE_LEN + 2u)
              {
                // Close file - we can't continue
                ths.CloseFiles();
                // Show message
                ths.text_box.SetText("; Program contain lines longer\n\r; than 80 characters");
                // Break the cycle
                break;
              }
              else
              {
                // Program is shown without its own line numbers
                StripLineNumbers(str);
                // Set this line to text_box
                ths.text_box.AddLine(str);
              }
            }
          }
        }
      }
    }
    else
    {
      // If memory allocation operation isn't successful set text
      ths.text_box.SetText("; Error open file!");
    }

    // Selector for this text
    ths.ResetSelector();
    // And show it
    ths.text_box.Show(100);
    // Left button
    ths.left_btn.Show(102);
    // Open button
    ths.middle_btn.Show(102);
    // Right button
    ths.right_btn.Show(102);

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessMenuCancelCallback function   *************************
// *****************************************************************************
Result ProgramSender::ProcessMenuCancelCallback(ProgramSender* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProgramSender& ths = *obj_ptr;

    // Hide the menu
    ths.menu.Hide();
    // Set cancel text
    ths.text_box.SetText("; Cancel pressed in open dialog");
    // Selector for this text
    ths.ResetSelector();
    // And show textbox
    ths.text_box.Show(100);
    // Run button
    ths.left_btn.Show(102);
    // Open button
    ths.middle_btn.Show(102);
    // Stop button
    ths.right_btn.Show(102);

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result ProgramSender::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Process Run button. Since we can call this handler after press of physical
  // button, we have to check if Run button is active.
  if(ptr == &left_btn)
  {
    // We should run program only if it doesn't already run, we in control,
    // state is Idle and there is a program to run: without the line check
    // Run with nothing loaded(or after the SD file was closed by leaving
    // the screen) would stream empty commands indefinitely. Error of the
    // previous command must be cleared: the first line would fail with it.
    if(!run && grbl_comm.IsInControl() && (grbl_comm.GetState() == GrblComm::IDLE) && (text_box.GetNumberOfLines() > 0) && (text_box.GetSelect() == 0) && ((text_box.GetText() != nullptr) || (p_disp_file != nullptr)) && (grbl_comm.GetStatusCode() == GrblComm::Status_OK))
    {
      // Clear id to run program
      id = 0u;
      // Start sending from the first line. Text is taken from the text box:
      // if program can't be run, it is a message that is there.
      send_line = 0u;
      p_send = text_box.GetText();
      cmd_ready = false;
      // Nothing is known about line numbers from controller yet
      first_numbered_id = 0u;
      line_number_valid = false;
      // Follow the line controller executes if it reports line numbers,
      // otherwise the line that is sent, as before
      line_mode = grbl_comm.IsLineNumberReportEnabled() ? LINE_EXECUTED : LINE_SENT;
      // Selector is filled when it shows the line that is executed and it is
      // a frame when it shows the line that will be sent next
      text_box.SetSelectorFill((line_mode == LINE_SENT) ? 2u : 0u);
      exec_line = 0u;
      // Selected line is the first one(Run isn't enabled otherwise)
      shown_line = 1u;
      // Set run flag to start program streaming
      run = true;
      finished = false;
      // Enable Feed & Speed control
      feed_dw.SetActive(true);
      speed_dw.SetActive(true);
      feed_dw.SetBorder(BORDER_W, COLOR_RED);
      speed_dw.SetBorder(BORDER_W, COLOR_RED);
      feed_dw.SetSelected(true);
      speed_dw.SetSelected(false);
      flood_btn.Enable();
      mist_btn.Enable();
      // Disable buttons while program is running
      middle_btn.Disable();
      // Disable screen change if program is running
      Application::GetInstance().DisableScreenChange();
    }
    else
    {
      result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
    }
  }
  // Process Reset button
  else if(ptr == &right_btn)
  {
    // Clear run flag
    run = false;
    // For Application to handle it(Stop/Reset)
    result = Result::ERR_UNHANDLED_REQUEST;
  }
  // Process Reset button
  else if((ptr == &middle_btn) && (middle_btn.IsActive()))
  {
    // Stop timer to prevent queue overflow since SD card operations can take some time.
    AppTask::GetCurrent()->StopTimer();

    // Clear text box
    text_box.SetText(nullptr);
    // We may have file open - close it
    CloseFiles();
    // Clear current data to show available memory
    ReleaseDataPointer();

    // Reinit SD card
    BSP_SD_Init();

    // Mount SD
    FRESULT res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
    DIR dir;

    // Open the directory
    if(res == FR_OK)
    {
      res = f_opendir(&dir, "/");
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
        if((res != FR_OK) || (fno.fname[0] == 0)) break;
        // Add only program files. The same filter is used by the open
        // handler which finds the file by its index in the directory.
        if(IsProgramFile(fno))
        {
          menu_items[idx].str.SetString(menu_items[idx].text, menu_items[idx].n, "%-19.19s%12lub", fno.fname, fno.fsize);
          idx++;
          // If menu is full - replace last item with a marker, otherwise the
          // rest of the files would be missing without any indication
          if(idx == NumberOf(menu_items))
          {
            menu_items[idx - 1u].str.SetString(menu_items[idx - 1u].text, menu_items[idx - 1u].n, "-- Too many files! --");
            break;
          }
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

    // Hide text box before open menu
    text_box.Hide();
    // Go button
    left_btn.Hide();
    // Open button
    middle_btn.Hide();
    // Reset button
    right_btn.Hide();

    // Set menu items count
    menu.SetCount(idx);
    // Show menu
    menu.Show(100);

    // Clear current position
    idx = 0u;
  }
  else if(ptr == &feed_dw)
  {
    speed_dw.SetSelected(false);
    feed_dw.SetSelected(true);
  }
  else if(ptr == &speed_dw)
  {
    feed_dw.SetSelected(false);
    speed_dw.SetSelected(true);
  }
  else if(ptr == &flood_btn)
  {
    grbl_comm.CoolantFloodToggle();
  }
  else if(ptr == &mist_btn)
  {
    grbl_comm.CoolantMistToggle();
  }
  else
  {
    ; // Do nothing - MISRA rule
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: CloseFiles function   ****************************************
// *****************************************************************************
void ProgramSender::CloseFiles()
{
  // File that isn't open is rejected by f_close() itself
  f_close(&SDFile);
  // Second position in the same file, if there is one
  if(p_disp_file != nullptr)
  {
    f_close(p_disp_file);
    delete p_disp_file;
    p_disp_file = nullptr;
  }
}

// *****************************************************************************
// ***   Public: AllocateDataBuffer   ******************************************
// *****************************************************************************
char* ProgramSender::AllocateDataBuffer(uint32_t& size)
{
  // Always release data buffer before allocate it again
  ReleaseDataPointer();

  // If size is zero
  if(size == 0u)
  {
    // Get maximum available block size from FreeRTOS and use it. Reserve
    // heap block overhead, guarding against underflow for tiny blocks.
    HeapStats_t HeapStats;
    vPortGetHeapStats(&HeapStats);
    size = (HeapStats.xSizeOfLargestFreeBlockInBytes > 32u) ? (HeapStats.xSizeOfLargestFreeBlockInBytes - 32u) : 0u;
  }

  // Allocate memory for data. Zero size is degenerate - nothing can be
  // stored in such buffer, and writing the null-terminator below would be
  // out of bounds.
  p_text = (size != 0u) ? new(std::nothrow) char[size] : nullptr;
  // If allocation is successful
  if(p_text != nullptr)
  {
    // Add null-terminator to the first element
    p_text[0] = '\0';
  }
  else
  {
    // If allocation unsuccessful - clear size
    size = 0u;
  }
  // Set buffer(or nullptr) to textbox
  text_box.SetText(p_text);
  // Update free memory info
  Application::GetInstance().UpdateMemoryInfo();
  // Return result
  return p_text;
}

// *****************************************************************************
// ***   Public: ReleaseDataPointer   ******************************************
// *****************************************************************************
void ProgramSender::ReleaseDataPointer()
{
  // If buffer was previously allocated
  if(p_text != nullptr)
  {
    // Hide text box before delete buffer
    text_box.Hide();
    // Delete previously allocated buffer
    delete[] p_text;
    // Set text to nullptr
    p_text = nullptr;
  }
  // Set null data pointer
  text_box.SetText(nullptr);
  // Update free memory info
  Application::GetInstance().UpdateMemoryInfo();
}

// *****************************************************************************
// ***   Private: ProcessEncoderCallback function   ****************************
// *****************************************************************************
Result ProgramSender::ProcessEncoderCallback(ProgramSender* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProgramSender& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // If program isn't running - scroll text
    if(!ths.run)
    {
      ths.enc_val += enc_val;
    }
    else if(ths.feed_dw.IsSelected())
    {
      ths.feed_val += enc_val;
    }
    else if(ths.speed_dw.IsSelected())
    {
      ths.speed_val += enc_val;
    }
    else
    {
      ; // Do nothing - MISRA rule
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private constructor   *************************************************
// *****************************************************************************
ProgramSender::ProgramSender() : msg_box(Application::GetInstance().GetMsgBox()),
                                 left_btn(Application::GetInstance().GetLeftButton()),
                                 middle_btn(Application::GetInstance().GetMiddleButton()),
                                 right_btn(Application::GetInstance().GetRightButton()) {};
