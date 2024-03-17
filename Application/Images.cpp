//******************************************************************************
//  @file Images.cpp
//  @author Nicolai Shlapunov
//
//  @details Application: Images, data
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

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "Images.h"

// *****************************************************************************
// ***   Data   ****************************************************************
// *****************************************************************************

const ImageDesc MPG = {40, 40, 32, {.imgp = MPG_data}, PALETTE_884, (int32_t)PALETTE_884[0xC7]};
const ImageDesc RotaryTable = {40, 39, 32, {.imgp = RotaryTable_data}, PALETTE_884,  (int32_t)PALETTE_884[0xC7]};

const uint8_t MPG_data[] = {
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF, 0xA4, 0xA4, 0xA4, 0xAD, 0xAD, 0x5B, 0xAD, 0xA4, 0xA4, 
0x5B, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0x5B, 0xA4, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x5B, 0xA4, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xA4, 0xAD, 0xFF, 0xFF, 0xF6, 0x5B, 0x52, 0x52, 0x00, 0x00, 0x00, 0x00, 0x52, 0x52, 
0x5B, 0xFF, 0xFF, 0xFF, 0xA4, 0xA4, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF, 0xA4, 0xFF, 0xF6, 0xAD, 0x00, 0x00, 0x09, 
0x5B, 0xA4, 0xA4, 0x5B, 0x5B, 0xA4, 0xA4, 0xA4, 0x09, 0x00, 0x52, 0xAD, 0xF6, 0xFF, 0xA4, 0xFF, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xAD, 
0xAD, 0xFF, 0xF6, 0x09, 0x00, 0x5B, 0xAD, 0xAD, 0xFF, 0xFF, 0xFF, 0xA4, 0xAD, 0xFF, 0xFF, 0xFF, 
0xA4, 0xAD, 0x52, 0x00, 0x52, 0xF6, 0xFF, 0xAD, 0xAD, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xA4, 0xFF, 0xFF, 0x5B, 0x00, 0x52, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0x09, 0xA4, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0x00, 0xA4, 0xFF, 
0xFF, 0xA4, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xA4, 0xFF, 0xFF, 
0x5B, 0x00, 0xA4, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0xA4, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x5B, 0x00, 0x5B, 0xFF, 0xFF, 0xAD, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xF6, 0xAD, 0xFF, 0x5B, 0x00, 0xF6, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x5B, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5B, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xF6, 0x00, 
0xA4, 0xFF, 0xAD, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0x5B, 0xFF, 0x5B, 0x00, 
0xFF, 0xFF, 0xFF, 0x00, 0x09, 0x52, 0x00, 0x00, 0x52, 0x5B, 0xAD, 0xFF, 0xFF, 0xAD, 0x52, 0x52, 
0x00, 0x00, 0xA4, 0x00, 0x00, 0xFF, 0xFF, 0xF6, 0x00, 0xA4, 0xFF, 0xA4, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0x5B, 0xFF, 0xF6, 0x00, 0x5B, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x52, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
0x5B, 0x00, 0xF6, 0xFF, 0x5B, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xAD, 0xF6, 0x52, 0x52, 0xFF, 
0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xA4, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x52, 0x52, 0xFF, 0xA4, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0x5B, 0xFF, 0x5B, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6, 0x09, 0x00, 0xFF, 0xFF, 
0xFF, 0xFF, 0x00, 0xAD, 0xFF, 0x52, 0xC7, 0xC7, 0xC7, 0xC7, 0xA4, 0xFF, 0x00, 0x52, 0xFF, 0xFF, 
0xFF, 0x5B, 0x00, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xA4, 0x00, 0xA4, 0xFF, 0xFF, 0xFF, 0x5B, 0x09, 0xFF, 0x5B, 0xC7, 0xC7, 
0xC7, 0xA4, 0xFF, 0xF6, 0x00, 0xFF, 0xF6, 0xFF, 0xFF, 0x00, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0x00, 0xFF, 
0xFF, 0xFF, 0xAD, 0x00, 0xFF, 0xFF, 0xF6, 0xC7, 0xC7, 0xA4, 0xFF, 0x09, 0x09, 0xAD, 0x00, 0x00, 
0x52, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x52, 0x00, 0x00, 0xF6, 0x09, 0x5B, 0xFF, 0x5B, 0xC7, 
0xC7, 0x5B, 0xFF, 0x52, 0x5B, 0xFF, 0x52, 0x00, 0x00, 0x52, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0x00, 
0x52, 0x09, 0xFF, 0xA4, 0x52, 0xFF, 0xA4, 0xC7, 0xC7, 0xA4, 0xFF, 0x00, 0xAD, 0xFF, 0xFF, 0xFF, 
0x00, 0x5B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x5B, 0x00, 0xFF, 0xFF, 0xFF, 0xA4, 0x5B, 0xF6, 0xAD, 0xC7, 
0xC7, 0xAD, 0xF6, 0x5B, 0xAD, 0xFF, 0xFF, 0xFF, 0x00, 0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0x00, 
0xFF, 0xFF, 0xFF, 0xA4, 0x00, 0xFF, 0x09, 0xC7, 0xC7, 0xA4, 0xAD, 0x00, 0xF6, 0xFF, 0xFF, 0xAD, 
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0x00, 0xF6, 0xFF, 0xFF, 0x5B, 0xA4, 0xFF, 0xA4, 0xC7, 
0xC7, 0xAD, 0xAD, 0x09, 0xFF, 0xFF, 0xFF, 0xA4, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0x00, 
0xAD, 0xFF, 0xFF, 0xAD, 0xF6, 0xFF, 0x5B, 0xC7, 0xC7, 0xAD, 0xF6, 0x09, 0xA4, 0xFF, 0xFF, 0xFF, 
0x00, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0x00, 0xFF, 0xFF, 0xFF, 0xA4, 0x00, 0xFF, 0x52, 0xC7, 
0xC7, 0xA4, 0xFF, 0x00, 0xAD, 0xFF, 0xFF, 0xFF, 0x00, 0x5B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x5B, 0x00, 
0xFF, 0xFF, 0xFF, 0xA4, 0x52, 0xFF, 0xAD, 0xC7, 0xC7, 0x5B, 0xFF, 0x09, 0xA4, 0xFF, 0x00, 0x52, 
0x00, 0x52, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0x00, 0xF6, 0x00, 0xFF, 0xA4, 0x52, 0xFF, 0xA4, 0xC7, 
0xC7, 0xA4, 0xFF, 0x09, 0x52, 0xAD, 0x00, 0x00, 0x5B, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6, 0x00, 0xA4, 
0x00, 0x00, 0xF6, 0x09, 0x5B, 0xFF, 0x5B, 0xC7, 0xC7, 0xAD, 0xFF, 0xF6, 0x00, 0xF6, 0xFF, 0xFF, 
0xFF, 0x00, 0x52, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0x00, 0xFF, 0xFF, 0xFF, 0xF6, 0x00, 0xFF, 0xFF, 0xFF, 0xC7, 
0xC7, 0xC7, 0x5B, 0xF6, 0x52, 0x5B, 0xFF, 0xFF, 0xFF, 0x52, 0x00, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xA4, 0x00, 0x52, 0xFF, 
0xFF, 0xFF, 0x52, 0x09, 0xFF, 0x5B, 0xC7, 0xC7, 0xC7, 0xC7, 0xA4, 0xFF, 0xA4, 0x00, 0xFF, 0xFF, 
0xFF, 0xFF, 0x00, 0x00, 0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xAD, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xAD, 0xFF, 0x5B, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xA4, 0xF6, 0x52, 0x5B, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x09, 0xA4, 0xFF, 0xFF, 
0xFF, 0xAD, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xA4, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 
0xFF, 0x00, 0x5B, 0xF6, 0xA4, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xA4, 0xFF, 0xAD, 0x00, 0xA4, 
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0xFF, 0xFF, 0x52, 0x00, 0xFF, 0xFF, 0x00, 0xAD, 0xFF, 
0xF6, 0x52, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x5B, 0x00, 0xAD, 0xFF, 0xA4, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0x5B, 0xFF, 0xA4, 0x00, 0xF6, 0xFF, 0xFF, 0x00, 0x52, 0xA4, 0x00, 0x00, 
0x09, 0x52, 0x00, 0x00, 0x00, 0x00, 0x52, 0x52, 0x00, 0x00, 0x5B, 0x00, 0x00, 0xFF, 0xFF, 0xF6, 
0x00, 0xAD, 0xFF, 0x5B, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF, 0xF6, 0xFF, 0x52, 
0x00, 0xAD, 0xFF, 0x09, 0x00, 0xFF, 0xFF, 0xA4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0xA4, 0xFF, 0xFF, 0x00, 0x09, 0xFF, 0xAD, 0x00, 0x5B, 0xFF, 0xAD, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xAD, 0xFF, 0xFF, 0x5B, 0x00, 0x5B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0x5B, 0x5B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xA4, 0x00, 0x5B, 
0xFF, 0xF6, 0xF6, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xA4, 0xFF, 
0xFF, 0xA4, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xAD, 0xFF, 0xF6, 0xAD, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xAD, 0xAD, 0xFF, 0xF6, 0x5B, 0x00, 0x5B, 0xAD, 0xAD, 
0xFF, 0xFF, 0xFF, 0xA4, 0xA4, 0xFF, 0xFF, 0xFF, 0xAD, 0xAD, 0x09, 0x00, 0x52, 0xAD, 0xFF, 0xAD, 
0xAD, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xFF, 0x5B, 0xFF, 0xF6, 0xA4, 0x09, 0x00, 0x00, 0x5B, 0x5B, 0xF6, 0xA4, 0xA4, 0xF6, 0x5B, 0x5B, 
0x09, 0x00, 0x09, 0xAD, 0xFF, 0xFF, 0x5B, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xA4, 0xA4, 0xFF, 0xF6, 0xFF, 0x5B, 
0x5B, 0x09, 0x09, 0x00, 0x00, 0x5B, 0x09, 0x52, 0x5B, 0xFF, 0xFF, 0xFF, 0xA4, 0xA4, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xA4, 0x5B, 0xF6, 0xFF, 0xFF, 0xFF, 0xF6, 0xFF, 0xFF, 0xF6, 0xFF, 0xFF, 
0xFF, 0xF6, 0xA4, 0xA4, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xAD, 
0xA4, 0x5B, 0xF6, 0x52, 0x52, 0xAD, 0x52, 0xA4, 0xA4, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7};

const uint8_t RotaryTable_data[] = {
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF, 0xFF, 0xFF, 0xF6, 0xF6,
0xF6, 0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xFF, 0xAD, 0x5B, 0x5B, 0x52, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x5B, 0xA4, 0xA4, 0xF6,
0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF, 0xA4, 0x5B, 0xAD, 0xF6, 0xFF, 0xFF, 0xF6, 0xAD, 0xAD,
0xAD, 0xAD, 0xAD, 0xF6, 0xFF, 0xFF, 0xF6, 0xA4, 0x5B, 0xA4, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xF6, 0x00, 0xF6,
0xFF, 0xF6, 0xAD, 0xAD, 0xAD, 0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6, 0xAD, 0xA4, 0x5B, 0xA4, 0xF6,
0xAD, 0x09, 0xAD, 0xAD, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xF6, 0x52, 0x09, 0x09, 0xA4, 0x5B, 0xAD, 0xFF, 0xF6, 0xA4, 0xA4, 0xA4, 0xA4,
0xA4, 0xA4, 0xA4, 0xA4, 0xAD, 0xF6, 0xAD, 0x52, 0x5B, 0x00, 0xA4, 0xA4, 0x5B, 0xFF, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xF6, 0x5B, 0xFF, 0xAD, 0x09, 0x09,
0x52, 0xA4, 0x5B, 0xA4, 0xF6, 0xFF, 0xFF, 0xF6, 0xF6, 0xFF, 0xFF, 0xFF, 0xAD, 0x09, 0x5B, 0x5B,
0xA4, 0xA4, 0xA4, 0xFF, 0xF6, 0x5B, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xFF, 0x52, 0xFF, 0xFF, 0xA4, 0xFF, 0xAD, 0x52, 0x09, 0x52, 0xA4, 0x5B, 0x5B, 0x00, 0x52,
0xA4, 0x5B, 0x09, 0x52, 0xA4, 0x5B, 0x5B, 0x5B, 0xF6, 0xFF, 0xAD, 0xA4, 0xFF, 0xAD, 0xA4, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF, 0x5B, 0xF6, 0xFF, 0xAD, 0xFF, 0xFF, 0x5B,
0xAD, 0xAD, 0x52, 0x09, 0x52, 0x5B, 0xF6, 0x52, 0x09, 0xF6, 0x09, 0x5B, 0x5B, 0x09, 0xFF, 0xFF,
0xA4, 0xF6, 0xFF, 0x5B, 0xF6, 0xFF, 0x52, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xFF, 0x52, 0xFF, 0xFF, 0xAD, 0xFF, 0xF6, 0xA4, 0xFF, 0xA4, 0xF6, 0xA4, 0x09, 0x00, 0xA4, 0xAD,
0xA4, 0x00, 0x00, 0x09, 0xF6, 0xF6, 0xA4, 0xFF, 0xFF, 0x5B, 0xFF, 0xF6, 0xA4, 0xFF, 0xAD, 0xAD,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xF6, 0x52, 0xFF, 0xF6, 0xFF, 0xFF, 0xA4, 0xF6,
0xFF, 0xA4, 0xFF, 0xFF, 0x5B, 0x5B, 0x00, 0x09, 0x09, 0x00, 0x00, 0xAD, 0xAD, 0xFF, 0x5B, 0xFF,
0xFF, 0x5B, 0xFF, 0xFF, 0xA4, 0xFF, 0xF6, 0x5B, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xF6, 0x52, 0xFF, 0xF6, 0xF6, 0xFF, 0xA4, 0xFF, 0xFF, 0xAD, 0xFF, 0xFF, 0x52, 0xF6, 0x09, 0x00,
0x00, 0x00, 0xA4, 0x5B, 0xF6, 0xFF, 0x5B, 0xFF, 0xFF, 0x5B, 0xFF, 0xF6, 0xA4, 0xFF, 0xAD, 0x5B,
0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xF6, 0x09, 0xFF, 0xFF, 0xAD, 0xFF, 0xF6, 0xF6,
0xFF, 0xA4, 0xAD, 0xA4, 0x52, 0x52, 0xAD, 0xF6, 0xAD, 0xA4, 0x09, 0x52, 0xA4, 0x5B, 0xF6, 0xFF,
0xA4, 0xF6, 0xFF, 0xA4, 0xF6, 0xFF, 0xA4, 0x52, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xF6, 0x52, 0xAD, 0xFF, 0xF6, 0xF6, 0xFF, 0xA4, 0xAD, 0xA4, 0x5B, 0x52, 0x5B, 0xF6, 0xF6, 0xAD,
0xAD, 0xF6, 0xAD, 0x09, 0x52, 0x52, 0xAD, 0xA4, 0xA4, 0xFF, 0xAD, 0xA4, 0xFF, 0xF6, 0x5B, 0x52,
0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xF6, 0x5B, 0xA4, 0xF6, 0xFF, 0xF6, 0xAD, 0xAD,
0x5B, 0x5B, 0x5B, 0xAD, 0xAD, 0xAD, 0xAD, 0xA4, 0xA4, 0xA4, 0xA4, 0xAD, 0xAD, 0x5B, 0x52, 0x5B,
0xAD, 0xAD, 0x5B, 0xFF, 0xFF, 0x52, 0xF6, 0x00, 0x5B, 0xF6, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xF6, 0x52, 0xFF, 0x5B, 0xAD, 0xF6, 0xA4, 0x5B, 0x09, 0xA4, 0xA4, 0xAD, 0xAD, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xF6, 0xA4, 0x5B, 0xA4, 0xAD, 0x5B, 0x52, 0x52, 0xAD, 0xF6, 0x52, 0xF6, 0xF6, 0xAD,
0xFF, 0x5B, 0x5B, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xF6, 0x00, 0xFF, 0xFF, 0x09, 0x52, 0x09, 0x5B,
0xAD, 0xAD, 0xF6, 0xFF, 0xF6, 0xAD, 0xAD, 0xA4, 0xA4, 0xA4, 0xAD, 0xF6, 0xFF, 0xF6, 0xAD, 0xA4,
0xA4, 0x52, 0x52, 0x09, 0xA4, 0xFF, 0x5B, 0xFF, 0xFF, 0xFF, 0xF6, 0x5B, 0xA4, 0xFF, 0xC7, 0xC7,
0xF6, 0x5B, 0x5B, 0xFF, 0x00, 0xAD, 0x00, 0xA4, 0xFF, 0xFF, 0xF6, 0xAD, 0xF6, 0xAD, 0xAD, 0xAD,
0xAD, 0xAD, 0xAD, 0xAD, 0xAD, 0xF6, 0xFF, 0xFF, 0xAD, 0x00, 0xA4, 0x5B, 0xA4, 0xAD, 0xA4, 0xFF,
0xFF, 0xFF, 0xFF, 0xAD, 0x00, 0xFF, 0xC7, 0xC7, 0xF6, 0xA4, 0xAD, 0xA4, 0xF6, 0xAD, 0xA4, 0xAD,
0xA4, 0xA4, 0xA4, 0xF6, 0xFF, 0xFF, 0xFF, 0x52, 0x00, 0xFF, 0xFF, 0xFF, 0xF6, 0xAD, 0x5B, 0x52,
0xA4, 0x5B, 0xA4, 0xA4, 0xAD, 0x52, 0xFF, 0xFF, 0xFF, 0xF6, 0xAD, 0xF6, 0x5B, 0xFF, 0xC7, 0xC7,
0xF6, 0xA4, 0xFF, 0xAD, 0xA4, 0xF6, 0xAD, 0xF6, 0xFF, 0xFF, 0xF6, 0xAD, 0xA4, 0xA4, 0xA4, 0x5B,
0xA4, 0xA4, 0xAD, 0xA4, 0xAD, 0xF6, 0xFF, 0xFF, 0xFF, 0xA4, 0xAD, 0xAD, 0x52, 0xFF, 0xFF, 0xFF,
0xAD, 0xAD, 0xAD, 0xFF, 0x5B, 0xFF, 0xC7, 0xC7, 0xF6, 0x5B, 0xFF, 0xFF, 0xF6, 0x52, 0xAD, 0xF6,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xAD, 0x52, 0xA4, 0xFF, 0xFF, 0xAD, 0xAD, 0xFF, 0xA4, 0xA4, 0xFF, 0xA4, 0xFF, 0xC7, 0xC7,
0xF6, 0x09, 0xFF, 0xFF, 0xFF, 0x52, 0x5B, 0x5B, 0x5B, 0xF6, 0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6, 0xAD, 0x5B, 0x5B, 0xF6, 0xFF, 0xF6, 0xAD, 0xF6, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xA4, 0xFF, 0xC7, 0xC7, 0xF6, 0x00, 0xF6, 0xFF, 0xA4, 0x52, 0x09, 0xFF,
0xF6, 0xAD, 0xA4, 0x5B, 0xA4, 0xF6, 0xAD, 0xF6, 0xF6, 0xAD, 0xF6, 0xAD, 0xA4, 0xA4, 0xA4, 0xAD,
0xFF, 0xFF, 0xFF, 0xAD, 0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xA4, 0xFF, 0xC7, 0xC7,
0xF6, 0x5B, 0x52, 0xF6, 0xF6, 0x5B, 0xA4, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6, 0xAD, 0xA4, 0x00,
0x5B, 0xA4, 0xAD, 0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xF6, 0x52, 0xFF, 0x52, 0xF6, 0xC7, 0xC7, 0xF6, 0x52, 0xAD, 0x5B, 0xF6, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6, 0x52, 0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6,
0xA4, 0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6, 0xA4, 0xAD, 0xA4, 0x52, 0xF6, 0xC7,
0xFF, 0x52, 0xF6, 0xA4, 0x09, 0x5B, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6, 0xAD, 0xF6, 0x52, 0xA4, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xF6, 0xA4, 0x5B, 0xF6, 0xF6, 0xA4, 0x5B, 0xFF, 0xC7, 0xF6, 0x52, 0xFF, 0x52, 0x00, 0x09, 0xAD,
0xF6, 0x52, 0x00, 0x00, 0x00, 0x09, 0x52, 0x52, 0xAD, 0xFF, 0xFF, 0xFF, 0xAD, 0xAD, 0xFF, 0x52,
0xAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0xAD, 0xF6, 0x5B, 0xA4, 0xF6, 0xFF, 0x5B, 0xFF,
0xC7, 0xC7, 0xAD, 0x5B, 0xF6, 0xA4, 0xAD, 0xA4, 0x00, 0xF6, 0xAD, 0x5B, 0x5B, 0xA4, 0xA4, 0xA4,
0x09, 0x5B, 0xAD, 0x5B, 0xF6, 0xFF, 0xFF, 0xF6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6, 0xAD, 0xFF,
0xFF, 0xAD, 0xAD, 0xFF, 0xFF, 0xAD, 0x5B, 0xFF, 0xC7, 0xC7, 0xFF, 0xAD, 0x52, 0xF6, 0xFF, 0xF6,
0xA4, 0xA4, 0xA4, 0xFF, 0xAD, 0x5B, 0x52, 0x00, 0xAD, 0x09, 0xF6, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xF6, 0xAD, 0xF6, 0xFF, 0xF6, 0xAD, 0xF6, 0xFF, 0xF6, 0x52, 0xA4, 0xFF, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xF6, 0x52, 0x5B, 0xAD, 0xAD, 0xA4, 0xF6, 0xA4, 0x5B, 0xA4, 0x09, 0x00,
0x52, 0xAD, 0x52, 0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0xAD, 0xFF, 0xAD, 0xAD, 0xFF, 0xFF, 0xAD, 0xF6,
0xFF, 0xFF, 0x5B, 0x52, 0xF6, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF, 0xF6, 0x52,
0xA4, 0xA4, 0xAD, 0x5B, 0x52, 0x09, 0x00, 0x09, 0x00, 0xAD, 0x52, 0xFF, 0xFF, 0xFF, 0xFF, 0x5B,
0xAD, 0x5B, 0xF6, 0xFF, 0xF6, 0xAD, 0xFF, 0xFF, 0xAD, 0x09, 0xAD, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF, 0x5B, 0xAD, 0xA4, 0x00, 0x52, 0x09, 0x52, 0x52,
0x00, 0xA4, 0x52, 0xFF, 0xFF, 0xFF, 0xF6, 0xAD, 0xF6, 0x5B, 0x5B, 0xAD, 0xF6, 0xFF, 0xF6, 0x5B,
0xA4, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF,
0xA4, 0xAD, 0x5B, 0x5B, 0x00, 0x52, 0x5B, 0xAD, 0x00, 0x52, 0x09, 0xF6, 0xFF, 0xAD, 0x5B, 0xF6,
0xFF, 0xAD, 0xF6, 0xFF, 0xFF, 0xA4, 0x5B, 0xF6, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xF6, 0x5B, 0xAD, 0xA4, 0xAD, 0xA4, 0xF6, 0xA4,
0x09, 0x00, 0xA4, 0x52, 0x52, 0xF6, 0xFF, 0xAD, 0x5B, 0xFF, 0xFF, 0xAD, 0x52, 0xAD, 0xFF, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xA4, 0xA4, 0xAD, 0x5B, 0xAD, 0x5B, 0xA4, 0x00, 0x09, 0xF6, 0xF6, 0xAD, 0x5B, 0xAD, 0xFF,
0xAD, 0xF6, 0x52, 0xA4, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF, 0x5B, 0xA4, 0xA4, 0x09, 0x52, 0xAD,
0x00, 0xAD, 0xC7, 0xC7, 0xC7, 0xFF, 0xAD, 0x5B, 0x09, 0x52, 0xF6, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xFF, 0xAD, 0x52, 0x52, 0xAD, 0xFF, 0xF6, 0x00, 0xA4, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF,
0xF6, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xF6, 0x00, 0x00, 0xAD, 0xFF, 0xF6, 0x52, 0xF6,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xFF, 0x00, 0x00, 0x09, 0xA4, 0x5B, 0xFF, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xFF, 0xA4, 0x00, 0x00, 0xAD, 0xFF, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7,
0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7};
