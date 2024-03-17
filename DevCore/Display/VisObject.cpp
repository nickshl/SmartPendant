//******************************************************************************
//  @file VisObject.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Visual Object Base Class, implementation
//
//  @copyright Copyright (c) 2016, Devtronic & Nicolai Shlapunov
//             All rights reserved.
//
//  @section SUPPORT
//
//   Devtronic invests time and resources providing this open source code,
//   please support Devtronic and open-source hardware/software by
//   donations and/or purchasing products from Devtronic.
//
//******************************************************************************

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "VisObject.h"
#include "VisList.h"

#include "DisplayDrv.h" // for AddVisObjectToList() and DelVisObjectFromList()

// *****************************************************************************
// ***   Destructor   **********************************************************
// *****************************************************************************
VisObject::~VisObject()
{
  // Remove object from object list before delete
  Hide();
}

// *****************************************************************************
// ***   Lock Visual Object  ***************************************************
// *****************************************************************************
Result VisObject::LockVisObject()
{
  // Lock line
  return DisplayDrv::GetInstance().LockDisplayLine();
};

// *****************************************************************************
// ***   Unlock Visual Object   ************************************************
// *****************************************************************************
Result VisObject::UnlockVisObject()
{
  // Unlock line
  return DisplayDrv::GetInstance().UnlockDisplayLine();
};

// *****************************************************************************
// ***   SetList   *************************************************************
// *****************************************************************************
Result VisObject::SetList(VisList& l)
{
  Result result = Result::ERR_CANNOT_EXECUTE;

  // If object doesn't have list or not in the list chain - we can add/change it
  if((list == nullptr) || (!IsInList()))
  {
    list = &l;
    result = Result::RESULT_OK;
  }

  return result;
}

// *****************************************************************************
// ***   Show Visual Object   **************************************************
// *****************************************************************************
Result VisObject::Show(uint32_t z_pos)
{
  // By default all VisObjects attached to main display list
  if(list == nullptr) list = DisplayDrv::GetInstance().GetVisList();
  // Z position is 0 by default. In this case we can use 0 here as "no pos" flag
  if(z_pos != 0)
  {
    z = z_pos;
  }
  // Invalidate area for update. We need to force invalidation there since
  // Visual Object isn't show yet
  InvalidateObjArea(true);
  // Add to VisObject List
  return list->AddVisObjectToList(this, z);//DisplayDrv::GetInstance().AddVisObjectToList(this, z);
}

// *****************************************************************************
// ***   Hide Visual Object   **************************************************
// *****************************************************************************
Result VisObject::Hide(void)
{
  // Invalidate area for update
  InvalidateObjArea();
  // Delete from VisObject List
  return list->DelVisObjectFromList(this);//DisplayDrv::GetInstance().DelVisObjectFromList(this);
}

// *****************************************************************************
// ***   Check status of Show Visual Object   **********************************
// *****************************************************************************
bool VisObject::IsShow(void)
{
  // Return false by default
  bool ret = false;

  // Check if VisObject in the list, and if it is check if list is show
  if(IsInList() && list->IsShow())
  {
    ret = true;
  }

  // Return result
  return ret;
}

// *****************************************************************************
// ***   Check status of Show Visual Object   **********************************
// *****************************************************************************
bool VisObject::IsInList(void)
{
  // Return false by default
  bool ret = false;
  // If any pointer is not null - object in list
  if( (p_next != nullptr) || (p_prev != nullptr) )
  {
    ret = true;
  }
  // Return result
  return ret;
}

// *****************************************************************************
// ***   Move Visual Object   **************************************************
// *****************************************************************************
Result VisObject::Move(int32_t x, int32_t y, bool is_delta)
{
  Result result;
  // Lock object for changes
  result = LockVisObject();
  // Check result
  if(result.IsGood())
  {
    // Check if we need to move object at all - prevent unnecessary call InvalidateObjArea() function
    if( ((is_delta == false) && ((x != x_start) || (y != y_start))) ||
        ((is_delta == true)  && ((x != 0)       || (y != 0))) )
    {
      // Invalidate area before move to redraw area object move from
      InvalidateObjArea();
      // Make changes
      if(is_delta == true)
      {
        // Move object in delta coordinates
        x_start += x;
        y_start += y;
        x_end += x;
        y_end += y;
      }
      else
      {
        // Move object in absolute coordinates
        x_start = x;
        y_start = y;
        x_end = x + width - 1;
        y_end = y + height - 1;
      }
      // Invalidate area after move to redraw area object move to
      InvalidateObjArea();
    }
    // Unlock object after changes
    result = UnlockVisObject();
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   Action   **************************************************************
// *****************************************************************************
void VisObject::Action(ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy)
{
  // Empty function. We can do active object without custom Action function
  // for cover active object with lower Z.
}

// *****************************************************************************
// ***   Invalidate Display Area   *********************************************
// *****************************************************************************
void VisObject::InvalidateObjArea(bool force)
{
  // Only if VisObject is show
  if((IsShow() || force) && (list != nullptr))
  {
    // Invalidate area
    list->InvalidateArea(x_start, y_start, x_end, y_end);
  }
}
