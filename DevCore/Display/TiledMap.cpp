//******************************************************************************
//  @file TiledMap.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Tiled Map Class, implementation
//
//  @copyright Copyright (c) 2017, Devtronic & Nicolai Shlapunov
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
#include "TiledMap.h"

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
TiledMap::TiledMap(int32_t x, int32_t y, int32_t w, int32_t h,
                   uint8_t* map, uint32_t map_w, uint32_t map_h, uint8_t bitmask,
                   const ImageDesc* tiles, uint32_t n, int32_t def_color)
{
  x_start = x;
  y_start = y;
  width = w;
  height = h;
  x_end = x_start + width - 1;
  y_end = y_start + height - 1;
  tiles_map = map;
  map_width = map_w;
  map_height = map_h;
  tile_bitmask = bitmask;
  tiles_img = tiles;
  tiles_cnt = n;
  tile_width = tiles->width;
  tile_height = tiles->height;
  bg_color = def_color;
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void TiledMap::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  // Draw only if needed
  if((line >= y_start) && (line <= y_end))
  {
    // Find start x position
    int32_t start = x_start - start_x;
    // Prevent write in memory before buffer
    if(start < 0) start = 0;
    // Find end x position
    int32_t end = x_end - start_x;
    // Prevent buffer overflow
    if(end > n) end = n;

    // Find start tile index and offsets
    int32_t x_tile_idx = (x_pos + start_x) / tile_width;
    int32_t y_tile_idx = (y_pos + line - y_start) / tile_height;
    int32_t tile_idx = y_tile_idx * map_width + x_tile_idx;
    int32_t x_tile_offset = (x_pos  + start_x) % tile_width;
    int32_t y_tile_offset = ((y_pos + line - y_start) % tile_height) * tile_width;

    // If default color is 0 or greater
    if(bg_color >= 0)
    {
      // Fill buffer by default color
      for(int32_t i = start; i < end; i++)
      {
        buf[i] = bg_color;
      }
    }
    // Prepare variables for first cycle
    int32_t pix_idx = start;
    int32_t tile_pix_idx = x_tile_offset;
    // Draw line with tiles
    while(pix_idx < n)
    {
      // Get tile value
      uint8_t tile_val = tiles_map[tile_idx] & tile_bitmask;
      // Skip empty tiles
      if(tile_val >= tiles_cnt)
      {
        pix_idx += tile_width - tile_pix_idx;
        tile_idx++;
        tile_pix_idx = 0;
        tile_val = tiles_map[tile_idx] & tile_bitmask;
        continue;
      }
      // Get pointer to the current tile image
      const uint8_t* tile_ptr = &tiles_img[tile_val].imgp[y_tile_offset];
      // Get pointer to the current tile palette
      const color_t* palette_ptr = tiles_img[tile_val].palette;
      // Get transparent color
      const int32_t transparent_color = tiles_img[tile_val].transparent_color;
      // Draw tile
      for(;(tile_pix_idx < (int32_t)tile_width) && (pix_idx < end); tile_pix_idx++)
      {
        // Get pixel data
        color_t data = palette_ptr[tile_ptr[tile_pix_idx]];
        // If not transparent - output to buffer
        if((int32_t)data != transparent_color) buf[pix_idx] = data;
        pix_idx++; 
      }
      // Increase tile index
      tile_idx++;
      // Clear tile pixel counter
      tile_pix_idx = 0;
    }
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void TiledMap::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
   // Not implemented yet
}

// *****************************************************************************
// ***   Scroll tiled map   ****************************************************
// *****************************************************************************
void TiledMap::ScrollView(int32_t dx, int32_t dy)
{
  // Lock object for changes
  LockVisObject();
  // Do changes
  x_pos += dx;
  if(x_pos < 0) x_pos = 0;
  y_pos += dy;
  if(y_pos < 0) y_pos = 0;
  // Invalidate area after move to redraw area object move to
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   GetLvlIdxByXY   *******************************************************
// *****************************************************************************
int32_t TiledMap::GetLvlIdxByXY(uint32_t x, uint32_t y)
{
  uint32_t result = -1;
  // Calculate tiles indexes
  uint32_t x_tile = x/tile_width;
  uint32_t y_tile = y/tile_height;
  // Set result only if x & y valid values
  if(x_tile < map_width && y_tile < map_height)
  {
    result = y_tile*map_width + x_tile;
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   GetLvlDataByXY   ******************************************************
// *****************************************************************************
int8_t TiledMap::GetLvlDataByXY(uint32_t x, uint32_t y)
{
  uint32_t result = 0u;
  // Calculate tiles indexes
  uint32_t x_tile = x/tile_width;
  uint32_t y_tile = y/tile_height;
  // Set result only if x & y valid values
  if(x_tile < map_width && y_tile < map_height)
  {
    result = tiles_map[y_tile*map_width + x_tile];
  }
  // Return result
  return result;
}
