//******************************************************************************
//  @file TiledMap.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Tiled Map Class, header
//
//  @section LICENSE
//
//   Software License Agreement (BSD License)
//
//   Copyright (c) 2017, Devtronic & Nicolai Shlapunov
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

#ifndef TilesMap_h
#define TilesMap_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "VisObject.h"
#include "Image.h"

// *****************************************************************************
// ***   Tile Map Class   ******************************************************
// *****************************************************************************
class TiledMap : public VisObject
{
  public:
    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    TiledMap(int32_t x, int32_t y, int32_t w, int32_t h,
             uint8_t* map, uint32_t map_w, uint32_t map_h, uint8_t bitmask,
             const ImageDesc* tiles, uint32_t n, int32_t def_color);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t y = 0);
    
    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t x = 0);
    
    // *************************************************************************
    // ***   Scroll tiled map   ************************************************
    // *************************************************************************
    void ScrollView(int32_t dx, int32_t dy = 0);

    // *************************************************************************
    // ***   GetMapPosX   ******************************************************
    // *************************************************************************
    int32_t GetLvlIdxByXY(uint32_t x, uint32_t y);
    
    // *************************************************************************
    // ***   GetMapPosX   ******************************************************
    // *************************************************************************
    int8_t GetLvlDataByXY(uint32_t x, uint32_t y);

    // *************************************************************************
    // ***   GetMapPosX   ******************************************************
    // *************************************************************************
    int32_t GetMapPosX(void) {return x_pos;}
    
    // *************************************************************************
    // ***   GetMapPosY   ******************************************************
    // *************************************************************************
    int32_t GetMapPosY(void) {return y_pos;}

    // *************************************************************************
    // ***   GetPixWidth   *****************************************************
    // *************************************************************************
    int32_t GetPixWidth() {return (tile_width * map_width);}

    // *************************************************************************
    // ***   GetPixHeight   ****************************************************
    // *************************************************************************
    int32_t GetPixHeight() {return (tile_height * map_height);}

  private:
    // Pointer to the tiles map
    uint8_t* tiles_map;
    // Map width in tiles
    uint32_t map_width;
    // Map height in tiles
    uint32_t map_height;
    // Bitmask for tiles
    uint8_t tile_bitmask;
    // Image descriptions of tiles picture
    const ImageDesc* tiles_img;
    // Image descriptions of tiles picture
    uint32_t tiles_cnt;
    // Tiles width in pixels
    uint32_t tile_width;
    // Tiles height in pixels
    uint32_t tile_height;
    // Background color (-1 - transparent)
    int32_t bg_color;
    // X position of tiled map in the viewport
    int32_t x_pos = 0;
    // Y position of tiled map in the viewport
    int32_t y_pos = 0;
};

#endif
