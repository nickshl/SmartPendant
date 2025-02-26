//******************************************************************************
//  @file Application.h
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

#ifndef Application_h
#define Application_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "AppTask.h"
#include "DisplayDrv.h"
#include "UiEngine.h"
#include "Hysteresis.h"
#include "StHalUart.h"

#include "DataWindow.h"
#include "Header.h"
#include "Tabs.h"

#include "InputDrv.h"
#include "GrblComm.h"
#include "NVM.h"

#include "Images.h"
#include "DirectControlScr.h"
#include "OverrideCtrlScr.h"
#include "DelayControlScr.h"
#include "RotaryTableScr.h"
#include "ProgramSender.h"
#include "ProbeScr.h"
#include "MillOpsScr.h"
#include "LatheOpsScr.h"
#include "SettingsScr.h"

// *****************************************************************************
// ***   Local const variables   ***********************************************
// *****************************************************************************

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************
#define BG_Z (100)

// *****************************************************************************
// ***   Application Class   ***************************************************
// *****************************************************************************
class Application : public AppTask
{
  public:
    // *************************************************************************
    // ***   Public: Get Instance   ********************************************
    // *************************************************************************
    static Application& GetInstance(void);

    // *************************************************************************
    // ***   Public: Setup function   ******************************************
    // *************************************************************************
    virtual Result Setup();

    // *************************************************************************
    // ***   Public: TimerExpired function   ***********************************
    // *************************************************************************
    virtual Result TimerExpired();

    // *************************************************************************
    // ***   Public: ProcessMessage function   *********************************
    // *************************************************************************
    virtual Result ProcessMessage();

    // *************************************************************************
    // ***   Public: ProcessCallback function   ********************************
    // *************************************************************************
    virtual Result ProcessCallback(const void* ptr);

    // *************************************************************************
    // ***   Public: InitSoftButtons function   ********************************
    // *************************************************************************
    void InitSoftButtons(bool three_buttons = false);

    // *************************************************************************
    // ***   Public: GetScreenStartY function   *********************************
    // *************************************************************************
    inline int32_t GetScreenStartY() {return 40;}

    // *************************************************************************
    // ***   Public: GetScreenEndY function   **********************************
    // *************************************************************************
    inline int32_t GetScreenEndY() {return status_box.GetStartY() - 1;}

    // *************************************************************************
    // ***   Public: GetRightButton function   *********************************
    // *************************************************************************
    UiButton& GetRightButton() {return right_btn;}

    // *************************************************************************
    // ***   Public: GetLeftButton function   **********************************
    // *************************************************************************
    UiButton& GetLeftButton() {return left_btn;}

    // *************************************************************************
    // ***   Public: GetMiddleButton function   ********************************
    // *************************************************************************
    UiButton& GetMiddleButton() {return middle_btn;}

    // *************************************************************************
    // ***   Public: GetRealDataWindow function   ******************************
    // *************************************************************************
    inline DataWindow& GetRealDataWindow(uint32_t axis) {return dw_real[axis];}

    // *************************************************************************
    // ***   Public: GetRealDataWindowNameString function   ********************
    // *************************************************************************
    inline String& GetRealDataWindowNameString(uint32_t axis) {return dw_real_name[axis];}

    // *************************************************************************
    // ***   Public: GetChangeValueBox function   ******************************
    // *************************************************************************
    ChangeValueBox& GetChangeValueBox() {return change_value_box;}

    // *************************************************************************
    // ***   Public: UpdateLeftButtonText function   ***************************
    // *************************************************************************
    void UpdateLeftButtonText();

    // *************************************************************************
    // ***   Public: UpdateLeftButtonIdleText function   ***********************
    // *************************************************************************
    void UpdateLeftButtonIdleText(const char* str = nullptr);

    // *************************************************************************
    // ***   Public: UpdateRightButtonText function   **************************
    // *************************************************************************
    void UpdateRightButtonText();

    // *************************************************************************
    // ***   Public: UpdateRightButtonIdleText function   **********************
    // *************************************************************************
    void UpdateRightButtonIdleText(const char* str = nullptr);

    // *************************************************************************
    // ***   Public: ChangeScreen function   ***********************************
    // *************************************************************************
    void ChangeScreen(IScreen& screen);

    // *************************************************************************
    // ***   Public: EnableScreenChange function   *****************************
    // *************************************************************************
    void EnableScreenChange() {header.Enable(); mpg_btn.Enable();}

    // *************************************************************************
    // ***   Public: DisableScreenChange function   ****************************
    // *************************************************************************
    void DisableScreenChange() {header.Disable(); mpg_btn.Disable();}

  private:
    // Timer period
    static const uint32_t TASK_TIMER_PERIOD_MS = 20U;
    // Border width
    static constexpr uint8_t BORDER_W = 4u;

    // Pointers to screens
    IScreen* scr[16u] = {0};
    // Screen counter
    uint32_t scr_cnt = 0u;
    // Current screen index
    uint32_t scr_idx = 0u;

    // Pages
    Header header;

    // MPG button
    UiButton mpg_btn;

    // Status objects
    Box status_box;
    String state_str;
    String status_str;
    String pins_str;

    // Data windows to show real position
    DataWindow dw_real[GrblComm::AXIS_CNT];
    // String for caption
    String dw_real_name[NumberOf(dw_real)];

    // Soft Buttons
    UiButton left_btn;
    UiButton middle_btn;
    UiButton right_btn;

    // Object to change numerical parameters
    ChangeValueBox change_value_box;

    // Display rotation
    uint8_t rotation = IDisplay::ROTATION_RIGHT;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // Input driver instance
    InputDrv& input_drv = InputDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();
    // NVM instance
    NVM& nvm = NVM::GetInstance();

    // Task queue message types
    enum TaskQueueMsgType
    {
       TASK_CALLBACK_MSG
    };

    // Task queue message struct
    struct TaskQueueMsg
    {
      TaskQueueMsgType type;
      union
      {
          void* obj_ptr;
      };
    };

    // Buffer for received task message
    TaskQueueMsg rcv_msg;

    // Button callback entry
    InputDrv::CallbackListEntry btn_cble;

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(Application* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ChangeScreen function   **********************************
    // *************************************************************************
    void ChangeScreen(uint8_t scrn);

    // *************************************************************************
    // ***   Private: InitHeader function   ************************************
    // *************************************************************************
    void InitHeader();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    Application() : AppTask(APPLICATION_TASK_STACK_SIZE, APPLICATION_TASK_PRIORITY,
                            "Application", 32u, sizeof(TaskQueueMsg), &rcv_msg,
                            TASK_TIMER_PERIOD_MS) {};
};

#endif
