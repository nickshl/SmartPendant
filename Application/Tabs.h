//******************************************************************************
//  @file Tabs.h
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

#ifndef Tabs_h
#define Tabs_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"

// *****************************************************************************
// ***   Tabs Class   **********************************************************
// *****************************************************************************
class Tabs : public VisObject
{
  public:
    // Maximum allowed number of tabs
    static constexpr uint32_t MAX_TABS = 16u;

    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    Tabs() {};

    // *************************************************************************
    // ***   Public: SetParams   ***********************************************
    // *************************************************************************
    void SetParams(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t n);

    // *************************************************************************
    // ***   Public: SetImage   ************************************************
    // *************************************************************************
    void SetImage(uint32_t tab, const ImageDesc& img_dsc);

    // *************************************************************************
    // ***   Public: SetText   *************************************************
    // *************************************************************************
    void SetText(uint32_t tab, const char *str1, const char *str2, Font& font);

    // *************************************************************************
    // ***   Public: SetFont   *************************************************
    // *************************************************************************
    void SetFont(Font& font, uint32_t scale = 1u);

    // *************************************************************************
    // ***   Public: SetBorder   ***********************************************
    // *************************************************************************
    void SetBorder(color_t c);

    // *************************************************************************
    // ***   Public: SetSelectedTab   ******************************************
    // *************************************************************************
    void SetSelectedTab(uint32_t n);

    // *************************************************************************
    // ***   Public: GetSelectedTab   ******************************************
    // *************************************************************************
    uint32_t GetSelectedTab() {return selected_tab;}

    // *************************************************************************
    // ***   Public: Set callback function   ***********************************
    // *************************************************************************
    void SetCallback(AppTask* task, CallbackPtr func = nullptr, void* param = nullptr);

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
    inline bool IsEnabled() {return !shadowbox.IsShow();}

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

    // Border color
    color_t border_color = COLOR_WHITE;
    // Tab color
    color_t tab_color = COLOR_GREY;
    // Selected Tab color
    color_t selected_tab_color = COLOR_DARKGREY;

    // Current tab
    uint32_t selected_tab = 0u;
    // Number of tabs
    uint32_t tabs_cnt = 0u;
    // Width of tab
    int32_t tab_w = 0;

    // List that contains all menu elements
    VisList list;

    // Tabs objects
    Box box[MAX_TABS];
    // Image object for each tab
    Image img[MAX_TABS];
    // Two string objects for each tab
    String tab_cap[MAX_TABS][2];
    // Selected tab object
    Box tab;

    // Box to disable tabs
    ShadowBox shadowbox;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
};

#endif
