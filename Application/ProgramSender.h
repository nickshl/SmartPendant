//******************************************************************************
//  @file ProgramSender.h
//  @author Nicolai Shlapunov
//
//  @details ProgramSender: User ProgramSender Class, header
//
//  @copyright Copyright (c) 2023, Devtronic & Nicolai Shlapunov
//             All rights reserved.
//
//  @section SUPPORT
//
//   Devtronic invests time and resources providing this open source code,
//   please support Devtronic and open-source hardware/software by
//   donations and/or purchasing products from Devtronic.
//
//******************************************************************************

#ifndef ProgramSender_h
#define ProgramSender_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCore.h"

#include "IScreen.h"
#include "DataWindow.h"
#include "GrblComm.h"
#include "InputDrv.h"
#include "Menu.h"
#include "MsgBox.h"
#include "TextBox.h"

#include "fatfs.h"

// *****************************************************************************
// ***   Local const variables   ***********************************************
// *****************************************************************************

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************
#define BG_Z (100)

// *****************************************************************************
// ***   ProgramSender Class   *************************************************
// *****************************************************************************
class ProgramSender : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static ProgramSender& GetInstance();

    // *************************************************************************
    // ***   Setup function   **************************************************
    // *************************************************************************
    virtual Result Setup(int32_t y, int32_t height);

    // *************************************************************************
    // ***   Public: Show   ****************************************************
    // *************************************************************************
    virtual Result Show();

    // *************************************************************************
    // ***   Public: Hide   ****************************************************
    // *************************************************************************
    virtual Result Hide();

    // *************************************************************************
    // ***   Public: TimerExpired   ********************************************
    // *************************************************************************
    virtual Result TimerExpired(uint32_t interval);

    // *************************************************************************
    // ***   Public: ProcessCallback   *****************************************
    // *************************************************************************
    virtual Result ProcessCallback(const void* ptr);

    // *************************************************************************
    // ***   Public: AllocateDataBuffer   **************************************
    // *************************************************************************
    char* AllocateDataBuffer(uint32_t& size);

    // *************************************************************************
    // ***   Public: GetDataBufferPtr   ****************************************
    // *************************************************************************
    char* GetDataBufferPtr() {return p_text;}

    // *************************************************************************
    // ***   Public: GetDataBufferLength   *************************************
    // *************************************************************************
    uint32_t GetDataBufferLength() {return (p_text != nullptr) ? strlen(p_text) : 0u;}

    // *************************************************************************
    // ***   Public: ReleaseDataPointer   **************************************
    // *************************************************************************
    void ReleaseDataPointer();

  private:
    static const uint8_t BORDER_W = 4u;

    // Run flag
    bool run = false;
    bool finished = false;
    // Current position
    uint32_t idx = 0u;
    // Current cmd ID
    uint32_t id = 0u;

    // Pointer to text buffer used if program loaded completely
    char* p_text = nullptr;

    // *************************************************************************
    // *** Sending and showing position   **************************************
    // *************************************************************************
    // Controller executes a line long after it was sent: selection in the
    // text box can't be the place lines are taken from. Sending has its own
    // position and the selection follows the line number controller reports.

    // How many lines at most are taken from the program for sending, or
    // read for the text box of a streamed program, in one TimerExpired() call
    static const uint32_t MAX_LINES_PER_TICK = 8u;
    // The greatest line number grblHAL accepts is 10000000
    static const uint32_t MAX_LINE_NUMBER = 9999999u;
    // Number(from 1) of the last line taken from the program for sending
    uint32_t send_line = 0u;
    // Next line to send if program is in memory
    const char* p_send = nullptr;
    // Command made of that line. Kept until GrblComm accepts it. The longest
    // one is '/', 'N' with seven digits, the line itself and CR.
    char cmd[TextBox::MAX_LINE_LEN + 16u] = {0};
    bool cmd_ready = false;
    // ID of the first command sent with a line number, zero if none yet
    uint32_t first_numbered_id = 0u;
    // Set when that command is acknowledged: line numbers controller reports
    // after that belong to this program
    bool line_number_valid = false;
    // What selection in the text box follows
    enum LineMode : uint8_t
    {
      LINE_EXECUTED, // Line that controller executes: it reports line numbers
      LINE_SENT      // Line that will be sent next: it doesn't
    };
    LineMode line_mode = LINE_SENT;
    // Number of the greatest line controller reported as executed
    uint32_t exec_line = 0u;
    // Number of the line selected in the text box
    uint32_t shown_line = 1u;
    // Streamed program is open twice: SDFile is read for sending, this one
    // for the text box. Allocated while such program is open: file object
    // holds a sector buffer and there is no RAM to keep it all the time.
    FIL* p_disp_file = nullptr;
    // Nothing more to read for the text box
    bool disp_end = false;

    // Strings
    char str[32u][32u + 1u] = {0};
    // menu items
    Menu::MenuItem menu_items[32u];
    // Menu object
    Menu menu;

    // Text box for program
    TextBox text_box;

    // Message box to display errors
    MsgBox& msg_box;

    // Soft Buttons
    UiButton& left_btn;
    UiButton& middle_btn;
    UiButton& right_btn;

    // *************************************************************************
    // *** Feeds & Speeds override   *******************************************
    // *************************************************************************

    // String for caption
    String feed_name;
    // Data windows to show current value
    DataWindow feed_dw;
    // Feed value
    int32_t feed_val = 0;

    // String for caption
    String speed_name;
    // Data windows to show current value
    DataWindow speed_dw;
    // Feed value
    int32_t speed_val = 0;

    // Buttons for control flood coolant
    UiButton flood_btn;
    // Buttons for control mist coolant
    UiButton mist_btn;

    // *************************************************************************
    // *************************************************************************
    // *************************************************************************

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // Encoder value
    int32_t enc_val = 0u;

    // Encoder callback entry
    InputDrv::CallbackListEntry enc_cble;

    // *************************************************************************
    // ***   Private: GetNextLine function   ***********************************
    // *************************************************************************
    // Takes the next line of the program for sending. Returns false if there
    // is none: program has ended(finished is set) or can't be read(streaming
    // is stopped with a message).
    bool GetNextLine(char* line, uint32_t size);

    // *************************************************************************
    // ***   Private: ShowNextLine function   **********************************
    // *************************************************************************
    // Moves selection in the text box one line down. Returns false at the
    // end of program.
    bool ShowNextLine();

    // *************************************************************************
    // ***   Private: ResetSelector function   *********************************
    // *************************************************************************
    // Sets selector of the text box for a new text in it
    void ResetSelector();

    // *************************************************************************
    // ***   Private: UpdateShownLine function   *******************************
    // *************************************************************************
    // Moves selection toward the line that is executed, max_lines at most
    void UpdateShownLine(uint32_t max_lines);

    // *************************************************************************
    // ***   Private: CloseFiles function   ************************************
    // *************************************************************************
    // Closes both positions in the streamed program. A file that isn't open
    // is fine.
    void CloseFiles();

    // *************************************************************************
    // ***   Private: StripLineNumbers function   ******************************
    // *************************************************************************
    // Removes N words from the text(one line or a whole program) in place.
    // Text of comments, system commands('$') and names in < > isn't touched.
    static void StripLineNumbers(char* text);

    // *************************************************************************
    // ***   Private: BuildCommand function   **********************************
    // *************************************************************************
    // Makes a command from the program line: comments are removed and line
    // number is added. Returns false if there is nothing to send.
    static bool BuildCommand(const char* line, uint32_t number, char* cmd, uint32_t size);

    // *************************************************************************
    // ***   Private: ProcessSpeedFeed function   ******************************
    // *************************************************************************
    Result ProcessSpeedFeed();

    // *************************************************************************
    // ***   Private: ProcessMenuOkCallback function   *************************
    // *************************************************************************
    static Result ProcessMenuOkCallback(ProgramSender* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessMenuCancelCallback function   *********************
    // *************************************************************************
    static Result ProcessMenuCancelCallback(ProgramSender* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessEncoderCallback function   ************************
    // *************************************************************************
    static Result ProcessEncoderCallback(ProgramSender* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    ProgramSender();
};

#endif
