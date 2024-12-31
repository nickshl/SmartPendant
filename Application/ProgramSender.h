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
    char* AllocateDataBuffer(uint32_t size);

    // *************************************************************************
    // ***   Public: GetDataBufferPtr   ****************************************
    // *************************************************************************
    char* GetDataBufferPtr() {return text;}

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

    // Strings
    char str[32u][32u + 1u] = {0};
    // menu items
    Menu::MenuItem menu_items[32u];
    // Menu object
    Menu menu;

    char* text = nullptr;
    TextBox text_box;

    // Data windows to show real position
    DataWindow dw_real[GrblComm::AXIS_CNT];
    // String for caption
    String dw_real_name[NumberOf(dw_real)];

    // Memory info
    String mem_info;
    char mem_info_buf[32u];

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
    // ***   Private: UpdateMemoryInfo function   ******************************
    // *************************************************************************
    void UpdateMemoryInfo();

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
