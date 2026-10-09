//******************************************************************************
//  @file TextBox.h
//  @author Nicolai Shlapunov
//
//  @details TextBox: User TextBox Class, header
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

#ifndef TextBox_h
#define TextBox_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCore.h"

#include "IScreen.h"
#include "InputDrv.h"

// *****************************************************************************
// ***   TextBox Class   *******************************************************
// *****************************************************************************
class TextBox : public VisList
{
  public:
    // Maximum length of a line content in characters, without line ending
    // (classic grbl line buffer limit). Public since users of the text box
    // check lines against the same limit(ProgramSender pre-check and
    // streaming backstop) - all checks must use the same number.
    static const uint32_t MAX_LINE_LEN = 80u;

    // Characters a line can show: screen is 320 pixels wide(rotation is
    // fixed), font is 10 pixels wide. Longer lines are cut.
    static const uint32_t VISIBLE_LEN = 320u / 10u;

    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    TextBox() {};

    // *************************************************************************
    // ***   Public: Setup   ***************************************************
    // *************************************************************************
    Result Setup(int32_t x, int32_t y, int32_t w, int32_t h);

    // *************************************************************************
    // ***   Public: Show   ****************************************************
    // *************************************************************************
    Result Show(uint32_t z = 0u);

    // *************************************************************************
    // ***   Public: Hide   ****************************************************
    // *************************************************************************
    Result Hide();

    // *************************************************************************
    // ***   Public: SetText   *************************************************
    // *************************************************************************
    bool SetText(const char* text);

    // *************************************************************************
    // ***   Public: AddLine   *************************************************
    // *************************************************************************
    Result AddLine(const char* text);

    // *************************************************************************
    // ***   Public: GetText   *************************************************
    // *************************************************************************
    // Text set by SetText(), nullptr if lines are added one by one
    const char* GetText() {return p_text;}

    // *************************************************************************
    // ***   Public: GetNumberOfLines   ****************************************
    // *************************************************************************
    int32_t GetNumberOfLines() {return lines_cnt;}

    // *************************************************************************
    // ***   Public: GetNumberOfVisibleLines   *********************************
    // *************************************************************************
    int32_t GetNumberOfVisibleLines() {return visible_cnt;}

    // *************************************************************************
    // ***   Public: GetSelect   ***********************************************
    // *************************************************************************
    int32_t GetSelect() {return select_pos;}

    // *************************************************************************
    // ***   Public: Select   **************************************************
    // *************************************************************************
    Result Select(int32_t n = 0);

    // *************************************************************************
    // ***   Public: GetScroll   ***********************************************
    // *************************************************************************
    int32_t GetScroll() {return scroll_pos;}

    // *************************************************************************
    // ***   Public: Scroll   **************************************************
    // *************************************************************************
    Result Scroll(int32_t n = 0);

    // *************************************************************************
    // ***   Public: SetSelectorColor   ****************************************
    // *************************************************************************
    void SetSelectorColor(color_t color);

    // *************************************************************************
    // ***   Public: SetSelectorFill   *****************************************
    // *************************************************************************
    // 0 - selector is filled, otherwise it is a frame of that width in pixels
    void SetSelectorFill(uint8_t fill);

  private:
    // Pointer to text
    const char* p_text = nullptr;
    // Pointer to current scroll position
    const char* p_scroll = nullptr;

    // Strings to show text
    String str[16];
    char str_text[16][VISIBLE_LEN + 1u] = {0}; // Visible part of a line + \0
    // Visible lines count
    int32_t visible_cnt = 0;

    // Selection box, its color and fill(see SetSelectorFill())
    Box box;
    // Current selector color
    color_t selector_color = COLOR_RED;
    // Indicate if selector filled or width of the border
    uint8_t selector_fill = 0u;
    // Current TextBox scroll position
    int32_t scroll_pos = 0;
    // Current TextBox select position
    int32_t select_pos = 0;
    // Number of lines in text
    int32_t lines_cnt = 0;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();

    // *************************************************************************
    // ***   Private: Strncpy function   ***************************************
    // *************************************************************************
    uint32_t Strncpy(char *dst, const char *src, uint32_t n);

    // *************************************************************************
    // ***   Private: UpdateSelector function   ********************************
    // *************************************************************************
    // Puts selection box on the selected line with its color and fill
    void UpdateSelector();
};

#endif
