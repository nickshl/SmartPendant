//******************************************************************************
//  @file VisList.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Visual Object Base Class, header
//
//  @section LICENSE
//
//   Software License Agreement (BSD License)
//
//   Copyright (c) 2023, Devtronic & Nicolai Shlapunov
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

#ifndef VisList_h
#define VisList_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "VisObject.h"
#include "AppTask.h"

// *****************************************************************************
// ***   VisList class   *******************************************************
// *****************************************************************************
class VisList : public VisObject
{
  public:
    // *************************************************************************
    // ***   VisList   *********************************************************
    // ************************************************************************* 
    VisList() {};

    // *************************************************************************
    // ***   SetParams   *******************************************************
    // *************************************************************************
    void SetParams(int32_t x, int32_t y, int32_t w, int32_t h);

    // *************************************************************************
    // ***   Add Visual Object to object list   ********************************
    // *************************************************************************
    Result AddVisObjectToList(VisObject* obj, uint32_t z);

    // *************************************************************************
    // ***   Delete Visual Object from object list   ***************************
    // *************************************************************************
    Result DelVisObjectFromList(VisObject* obj);

    // *************************************************************************
    // ***   DrawInBufH   ******************************************************
    // *************************************************************************
    // * Draw one horizontal line of object in specified buffer.
    // * Each derived class must implement this function.
    virtual void DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y =  0);

    // *************************************************************************
    // ***   DrawInBufW   ******************************************************
    // *************************************************************************
    // * Draw one vertical line of object in specified buffer.
    // * Each derived class must implement this function.
    virtual void DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x =  0);

    // *************************************************************************
    // ***   Action   **********************************************************
    // *************************************************************************
    virtual void Action(ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy);

    // *************************************************************************
    // ***   Invalidate Area   *************************************************
    // *************************************************************************
    virtual void InvalidateArea(int16_t start_x, int16_t start_y, int16_t end_x, int16_t end_y);

    // *************************************************************************
    // ***   IsShow   **********************************************************
    // *************************************************************************
    virtual bool IsShow(void);

  private:
    // *************************************************************************
    // ***   Object parameters   ***********************************************
    // *************************************************************************    // Pointer to first object in list
    VisObject* object_list = nullptr;
    // Pointer to last object in list
    VisObject* object_list_last = nullptr;
};

#endif
