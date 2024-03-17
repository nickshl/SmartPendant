//******************************************************************************
//  @file ProgrammSender.h
//  @author Nicolai Shlapunov
//
//  @details ProgrammSender: User ProgrammSender Class, header
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

#ifndef ProgrammSender_h
#define ProgrammSender_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "UiEngine.h"

#include "IScreen.h"
#include "DataWindow.h"
#include "GrblComm.h"
#include "InputDrv.h"
#include "Menu.h"
#include "TextBox.h"

// *****************************************************************************
// ***   Local const variables   ***********************************************
// *****************************************************************************

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************
#define BG_Z (100)

// *****************************************************************************
// ***   ProgrammSender Class   **********************************************
// *****************************************************************************
class ProgrammSender : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static ProgrammSender& GetInstance();

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

  private:
    static const uint8_t BORDER_W = 4u;

    // Run flag
    bool run = false;
    // Current position
    uint32_t idx = 0u;
    // Current cmd ID
    uint32_t id = 0u;

    // Strings
    char str[32u][32u + 1u] = {0};
    // menu items
    Menu::MenuItem menu_items[32u];
    // Menu object
    Menu menu;

    char text[2048u];
    TextBox text_box;

    // Soft Buttons
    UiButton left_btn;
    UiButton right_btn;
    // Buttons to open file
    UiButton open_btn;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // Encoder value
    int32_t enc_val = 0u;

    // Encoder callback entry
    InputDrv::CallbackListEntry enc_cble;
    // Button callback entry
    InputDrv::CallbackListEntry btn_cble;

    // *************************************************************************
    // ***   Private: ProcessMenuOkCallback function   *************************
    // *************************************************************************
    static Result ProcessMenuOkCallback(ProgrammSender* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessMenuCancelCallback function   *********************
    // *************************************************************************
    static Result ProcessMenuCancelCallback(ProgrammSender* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessEncoderCallback function   ************************
    // *************************************************************************
    static Result ProcessEncoderCallback(ProgrammSender* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(ProgrammSender* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    ProgrammSender() {};
};

#endif
