//******************************************************************************
//  @file VisObject.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Visual Object Base Class, header
//
//  @section LICENSE
//
//   Software License Agreement (BSD License)
//
//   Copyright (c) 2016, Devtronic & Nicolai Shlapunov
//   All rights reserved.
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//   3. Neither the name of the Devtronic nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
//   THIS SOFTWARE IS PROVIDED BY DEVTRONIC ''AS IS'' AND ANY EXPRESS OR IMPLIED
//   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL DEVTRONIC BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
//   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//******************************************************************************

#ifndef VisObject_h
#define VisObject_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"

// *****************************************************************************
// ***   Forward declaration   *************************************************
// *****************************************************************************
class VisList;

// *****************************************************************************
// * VisObject class. This class implements base Visual Objects properties. ****
// *****************************************************************************
class VisObject
{
  public:
    // *************************************************************************
    // ***   Action   **********************************************************
    // ************************************************************************* 
    typedef enum
    {
      ACT_TOUCH,   // When object touched
      ACT_UNTOUCH, // When object untouched
      ACT_MOVE,    // When touch position moved inside object
      ACT_HOLD,    // When touch position isn't moved inside object
      ACT_MOVEIN,  // When object moved in to object
      ACT_MOVEOUT, // When object moved out of object
      ACT_MAX      // Total possible actions
    } ActionType;

    // *************************************************************************
    // ***   VisObject   *******************************************************
    // ************************************************************************* 
    VisObject() {};

    // *************************************************************************
    // ***   ~VisObject   ******************************************************
    // *************************************************************************
    // * Destructor. Call DelVisObjectFromList() from DisplayDrv class for
    // * remove from list before delete and delete semaphore.
    virtual ~VisObject();

    // *************************************************************************
    // ***   LockVisObject   ***************************************************
    // *************************************************************************
    Result LockVisObject();

    // *************************************************************************
    // ***   UnlockVisObject   *************************************************
    // *************************************************************************
    Result UnlockVisObject();

    // *************************************************************************
    // ***   SetList   *********************************************************
    // *************************************************************************
    Result SetList(VisList& l);

    // *************************************************************************
    // ***   Show   ************************************************************
    // *************************************************************************
    // * Show VisObject on screen. This function call AddVisObjectToList() from
    // * DisplayDrv class. When this function calls first time, user must
    // * provide Z level. In future user can call this function without
    // * parameters - previously set Z will be used.
    virtual Result Show(uint32_t z_pos = 0);

    // *************************************************************************
    // ***   Hide   ************************************************************
    // *************************************************************************
    // * Hide VisObject from screen. This function call DelVisObjectFromList()
    // * from DisplayDrv class.
    virtual Result Hide(void);

    // *************************************************************************
    // ***   IsShow   **********************************************************
    // *************************************************************************
    virtual bool IsShow(void);

    // *************************************************************************
    // ***   IsInList   ********************************************************
    // *************************************************************************
    virtual bool IsInList(void);

    // *************************************************************************
    // ***   Move   ************************************************************
    // *************************************************************************
    // * Move object on screen. Set new x and y coordinates. If flag is set - 
    // * move is relative, not absolute.
    virtual Result Move(int32_t x, int32_t y, bool is_delta = false);

    // *************************************************************************
    // ***   DrawInBufH   ******************************************************
    // *************************************************************************
    // * Draw one horizontal line of object in specified buffer.
    // * Each derived class must implement this function.
    virtual void DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y = 0) = 0;

    // *************************************************************************
    // ***   DrawInBufW   ******************************************************
    // *************************************************************************
    // * Draw one vertical line of object in specified buffer.
    // * Each derived class must implement this function.
    virtual void DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x = 0) = 0;

    // *************************************************************************
    // ***   Action   **********************************************************
    // *************************************************************************
    virtual void Action(ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy);

    // *************************************************************************
    // ***   Return Start X coordinate   ***************************************
    // *************************************************************************
    virtual int32_t GetStartX(void) {return x_start;}

    // *************************************************************************
    // ***   Return Start Y coordinate   ***************************************
    // *************************************************************************
    virtual int32_t GetStartY(void) {return y_start;}

    // *************************************************************************
    // ***   Return End X coordinate   *****************************************
    // *************************************************************************
    virtual int32_t GetEndX(void) {return x_end;}

    // *************************************************************************
    // ***   Return End Y coordinate   *****************************************
    // *************************************************************************
    virtual int32_t GetEndY(void) {return y_end;}

    // *************************************************************************
    // ***   Return Width of object   ******************************************
    // *************************************************************************
    virtual int32_t GetWidth(void) {return width;}

    // *************************************************************************
    // ***   Return Height of object   *****************************************
    // *************************************************************************
    virtual int32_t GetHeight(void) {return height;}

    // *************************************************************************
    // ***   IsActive   ********************************************************
    // *************************************************************************
    virtual bool IsActive() {return active;}

    // *************************************************************************
    // ***   SetActive   *******************************************************
    // *************************************************************************
    virtual void SetActive(bool is_active) {active = is_active;}

    // *************************************************************************
    // ***   Invalidate Object Area   ******************************************
    // *************************************************************************
    virtual void InvalidateObjArea(bool force = false);

  protected:
    // *************************************************************************
    // ***   Object parameters   ***********************************************
    // *************************************************************************

    // Current list for object
    VisList* list = nullptr;

    // X and Y start coordinates of object
    int16_t x_start = 0, y_start = 0;
    // X and Y end coordinates of object
    int16_t x_end = 0, y_end = 0;
    // Width and Height of object
    int16_t width = 0, height = 0;
    // Rotation of object
    int8_t rotation = 0;
    // Object active
    bool active = false;

  private:
    // *************************************************************************
    // ***   Object parameters   ***********************************************
    // *************************************************************************
    // * Only base class and DisplayDrv have access to this parameters

    // Z position of object
    uint16_t z = 0;
    // Pointer to next object. This pointer need to maker object list. Object
    // can be added only to one list.
    VisObject* p_next = nullptr;
    // Pointer to next object. This pointer need to maker object list. Object
    // can be added only to one list.
    VisObject* p_prev = nullptr;

    // VisList is friend for access to pointers and Z
    friend class VisList;
};

#endif
