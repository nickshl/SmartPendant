//******************************************************************************
//  @file Header.h
//  @author Nicolai Shlapunov
//
//  @details Application: User Application Class, header
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

#ifndef Header_h
#define Header_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"

#include "UiEngine.h"
#include "InputDrv.h"

// *****************************************************************************
// ***   Header Class   ********************************************************
// *****************************************************************************
class Header : public VisObject
{
  public:
    // Maximum allowed number of pages
    static constexpr uint32_t MAX_PAGES = 16u;

    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    Header() {};

    // *************************************************************************
    // ***   Public: SetParams   ***********************************************
    // *************************************************************************
    void SetParams(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t n);

    // *************************************************************************
    // ***   Public: SetImage   ************************************************
    // *************************************************************************
    void SetImage(uint32_t page_idx, const ImageDesc& img_dsc);

    // *************************************************************************
    // ***   Public: SetText   *************************************************
    // *************************************************************************
    void SetText(uint32_t page_idx, const char *str_in, Font& font);

    // *************************************************************************
    // ***   Public: SetFont   *************************************************
    // *************************************************************************
    void SetFont(Font& font, uint32_t scale = 1u);

    // *************************************************************************
    // ***   Public: SetBorder   ***********************************************
    // *************************************************************************
    void SetBorder(color_t c);

    // *************************************************************************
    // ***   Public: SetSelectedPage   *****************************************
    // *************************************************************************
    void SetSelectedPage(uint32_t n);

    // *************************************************************************
    // ***   Public: GetSelectedPage   *****************************************
    // *************************************************************************
    uint32_t GetSelectedPage() {return selected_page;}

    // *************************************************************************
    // ***   Public: Set callback function   ***********************************
    // *************************************************************************
    void SetCallback(AppTask* task, CallbackPtr func = nullptr, void* param = nullptr);

    // *************************************************************************
    // ***   Public: ResizeButtons   *******************************************
    // *************************************************************************
    void ResizeButtons();

    // *************************************************************************
    // ***   Public: Show   ****************************************************
    // *************************************************************************
    Result Show(uint32_t z = 0u);

    // *************************************************************************
    // ***   Public: Hide   ****************************************************
    // *************************************************************************
    Result Hide();

    // *************************************************************************
    // ***   Public: Enable   **************************************************
    // *************************************************************************
    inline Result Enable() {return shadowbox.Hide();}

    // *************************************************************************
    // ***   Public: Disable   *************************************************
    // *************************************************************************
    inline Result Disable() {return shadowbox.Show(UINT32_MAX);}

    // *************************************************************************
    // ***   Public: IsEnabled   ***********************************************
    // *************************************************************************
    inline bool IsEnabled() {return shadowbox.IsShow();}

    // *************************************************************************
    // ***   Public: Put line in buffer   **************************************
    // *************************************************************************
    virtual void DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t y = 0);

    // *************************************************************************
    // ***   Public: Put line in buffer   **************************************
    // *************************************************************************
    virtual void DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t x = 0);

    // *************************************************************************
    // ***   Public: Put line in buffer   **************************************
    // *************************************************************************
    virtual void Action(VisObject::ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy);

  private:

    // Callback function pointer
    AppTask* callback_task = nullptr;
    CallbackPtr callback_func = nullptr;
    void* callback_param = nullptr;

    // Selected page
    uint32_t selected_page = 0u;
    // Number of pages
    uint32_t pages_cnt = 0u;

    // Image object for each page
    Image img[MAX_PAGES];
    // String objects for each page
    String str[MAX_PAGES];
    // Button objects for each page
    UiButton button[MAX_PAGES];

    // Line
    Line line_bottom;
    // Buttons
    UiButton btn_left;
    UiButton btn_right;
    // Box for cover screen
    Box box;

    // Box to disable header
    ShadowBox shadowbox;

    // Button callback entry
    InputDrv::CallbackListEntry btn_cble;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();

    // *************************************************************************
    // ***   Private: ProcessUiCallback function   *****************************
    // *************************************************************************
    static Result ProcessUiCallback(Header* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(Header* obj_ptr, void* ptr);
};

#endif
