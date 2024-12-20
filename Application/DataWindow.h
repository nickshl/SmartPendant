//******************************************************************************
//  @file DataWindow.h
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

#ifndef DataWindow_h
#define DataWindow_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"

// *****************************************************************************
// ***   DataWindow Class   ****************************************************
// *****************************************************************************
class DataWindow : public VisObject
{
  public:
    enum Position
    {
      NONE,
      TOP,
      LEFT,
      RIGHT,
      BOTTOM,
      BOTTOM_RIGHT
    };

    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    DataWindow() {};

    // *************************************************************************
    // ***   SetParams   *******************************************************
    // *************************************************************************
    void SetParams(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t len, uint32_t ths);

    // *************************************************************************
    // ***   SetDataFont   *****************************************************
    // *************************************************************************
    void SetDataFont(Font& font, uint32_t scale = 1u);

    // *************************************************************************
    // ***   SetUnits   ********************************************************
    // *************************************************************************
    void SetUnits(const char * str, Position pos = BOTTOM, Font& font = Font_8x12::GetInstance());

    // *************************************************************************
    // ***   SetBorder   *******************************************************
    // *************************************************************************
    void SetBorder(uint16_t w, color_t c);

    // *************************************************************************
    // ***   SetBorderWidth   **************************************************
    // *************************************************************************
    void SetBorderWidth(uint16_t w);

    // *************************************************************************
    // ***   SetBorderColor   **************************************************
    // *************************************************************************
    void SetBorderColor(color_t cu, color_t cs = COLOR_GREEN);

    // *************************************************************************
    // ***   SetLimits   *******************************************************
    // *************************************************************************
    void SetLimits(int32_t min, int32_t max);

    // *************************************************************************
    // ***   SetNumber   *******************************************************
    // *************************************************************************
    bool SetNumber(int32_t n);

    // *************************************************************************
    // ***   GetNumber   *******************************************************
    // *************************************************************************
    int32_t GetNumber() {return data;}

    // *************************************************************************
    // ***   SetSelected   *****************************************************
    // *************************************************************************
    void SetSelected(bool is_selected);

    // *************************************************************************
    // ***   IsSelected   ******************************************************
    // *************************************************************************
    bool IsSelected() {return selected;}

    // *************************************************************************
    // ***   Set callback function   *******************************************
    // *************************************************************************
    void SetCallback(AppTask* task, CallbackPtr func = nullptr, void* param = nullptr);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t y = 0);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t x = 0);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void Action(VisObject::ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy);

  private:
    // Callback function pointer
    AppTask* callback_task = nullptr;
    CallbackPtr callback_func = nullptr;
    void* callback_param = nullptr;

    // Selected flag
    bool selected = false;

    // Border colors
    color_t border_color_unselected = COLOR_BLACK;
    color_t border_color_selected = COLOR_GREEN;

    // Decimal point
    int32_t before_decimal = 1u;
    int32_t after_decimal = 0u;
    int32_t decimal_multiplier = 1u;

    // Min & Max limits
    int32_t min_limit = INT32_MIN;
    int32_t max_limit = INT32_MAX;

    // Data
    int32_t data = -1;

    // Objects
    Box box;
    String units_str;
    String data_str;
    char data_str_buf[24] = {0};
    char format_str_buf[16] = {0};
    Position units_str_pos = NONE;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();

    // *************************************************************************
    // ***   Private: UpdateStringPositions   **********************************
    // *************************************************************************
    void UpdateStringPositions(void);
};

#endif
