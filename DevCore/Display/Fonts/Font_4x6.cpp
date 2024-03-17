//******************************************************************************
//  @file Font_4x6.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Font 4x6 implementation, implementation
//
//  @copyright Copyright (c) 2019, Devtronic & Nicolai Shlapunov
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
#include "Font_4x6.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
Font_4x6& Font_4x6::GetInstance(void)
{
   static Font_4x6 font_4x6;
   return font_4x6;
}

// *****************************************************************************
// ***   Private: Constructor   ************************************************
// *****************************************************************************
Font_4x6::Font_4x6()
{
  char_width = 4U;
  char_height = 6U;
  bytes_per_char = 6U;
  font_data_ptr = (uint8_t*)font_data;
}

// *****************************************************************************
// ***   Private: Font data   **************************************************
// *****************************************************************************
const uint8_t Font_4x6::font_data[256][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x00 - ' '
    {0x04, 0x0A, 0x0E, 0x0A, 0x04, 0x00}, // 0x01 - ''
    {0x04, 0x0E, 0x0A, 0x0E, 0x04, 0x00}, // 0x02 - ''
    {0x00, 0x0A, 0x0E, 0x0E, 0x04, 0x00}, // 0x03 - ''
    {0x00, 0x04, 0x0E, 0x0E, 0x04, 0x00}, // 0x04 - ''
    {0x04, 0x0E, 0x0E, 0x04, 0x0E, 0x00}, // 0x05 - ''
    {0x04, 0x04, 0x0E, 0x04, 0x0E, 0x00}, // 0x06 - ''
    {0x00, 0x00, 0x04, 0x00, 0x00, 0x00}, // 0x07 - ''
    {0x0F, 0x0F, 0x0B, 0x0F, 0x0F, 0x0F}, // 0x08 - ''
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x09 - ' '
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x0A - ' '
    {0x00, 0x0C, 0x08, 0x06, 0x06, 0x00}, // 0x0B - ''
    {0x04, 0x0A, 0x04, 0x0E, 0x04, 0x00}, // 0x0C - ''
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x0D - ' '
    {0x04, 0x0C, 0x0A, 0x08, 0x04, 0x00}, // 0x0E - ''
    {0x04, 0x0E, 0x0A, 0x0E, 0x04, 0x00}, // 0x0F - ''
    {0x02, 0x06, 0x0E, 0x06, 0x02, 0x00}, // 0x10 - ''
    {0x08, 0x0C, 0x0E, 0x0C, 0x08, 0x00}, // 0x11 - ''
    {0x04, 0x0E, 0x04, 0x0E, 0x04, 0x00}, // 0x12 - ''
    {0x0A, 0x0A, 0x0A, 0x00, 0x0A, 0x00}, // 0x13 - ''
    {0x0E, 0x0B, 0x0B, 0x0A, 0x0A, 0x00}, // 0x14 - ''
    {0x0C, 0x06, 0x0A, 0x0C, 0x06, 0x00}, // 0x15 - ''
    {0x00, 0x00, 0x06, 0x06, 0x00, 0x00}, // 0x16 - ''
    {0x04, 0x0E, 0x04, 0x0E, 0x04, 0x0E}, // 0x17 - ''
    {0x04, 0x0E, 0x04, 0x04, 0x04, 0x00}, // 0x18 - ''
    {0x04, 0x04, 0x04, 0x0E, 0x04, 0x00}, // 0x19 - ''
    {0x00, 0x04, 0x0F, 0x04, 0x00, 0x00}, // 0x1A - ' '
    {0x00, 0x02, 0x0F, 0x02, 0x00, 0x00}, // 0x1B - ''
    {0x00, 0x00, 0x02, 0x0E, 0x00, 0x00}, // 0x1C - ''
    {0x00, 0x0A, 0x0E, 0x0A, 0x00, 0x00}, // 0x1D - ''
    {0x00, 0x04, 0x0E, 0x0E, 0x00, 0x00}, // 0x1E - ''
    {0x00, 0x0E, 0x0E, 0x04, 0x00, 0x00}, // 0x1F - ''
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x20 - ' '
    {0x04, 0x04, 0x04, 0x00, 0x04, 0x00}, // 0x21 - '!'
    {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00}, // 0x22 - '"'
    {0x0A, 0x0E, 0x0A, 0x0E, 0x0A, 0x00}, // 0x23 - '#'
    {0x04, 0x0C, 0x06, 0x0C, 0x06, 0x04}, // 0x24 - '$'
    {0x02, 0x08, 0x04, 0x02, 0x08, 0x00}, // 0x25 - '%'
    {0x04, 0x0A, 0x0C, 0x0A, 0x0E, 0x00}, // 0x26 - '&'
    {0x06, 0x02, 0x00, 0x00, 0x00, 0x00}, // 0x27 - '''
    {0x04, 0x02, 0x02, 0x02, 0x04, 0x00}, // 0x28 - '('
    {0x02, 0x04, 0x04, 0x04, 0x02, 0x00}, // 0x29 - ')'
    {0x0A, 0x04, 0x0E, 0x04, 0x0A, 0x00}, // 0x2A - '*'
    {0x00, 0x04, 0x0E, 0x04, 0x00, 0x00}, // 0x2B - '+'
    {0x00, 0x00, 0x00, 0x00, 0x06, 0x02}, // 0x2C - ','
    {0x00, 0x00, 0x0E, 0x00, 0x00, 0x00}, // 0x2D - '-'
    {0x00, 0x00, 0x00, 0x00, 0x04, 0x00}, // 0x2E - '.'
    {0x08, 0x08, 0x04, 0x02, 0x02, 0x00}, // 0x2F - '/'
    {0x0C, 0x0A, 0x0A, 0x0A, 0x06, 0x00}, // 0x30 - '0'
    {0x04, 0x06, 0x04, 0x04, 0x0E, 0x00}, // 0x31 - '1'
    {0x06, 0x08, 0x04, 0x02, 0x0E, 0x00}, // 0x32 - '2'
    {0x06, 0x08, 0x04, 0x08, 0x06, 0x00}, // 0x33 - '3'
    {0x08, 0x0A, 0x0E, 0x08, 0x08, 0x00}, // 0x34 - '4'
    {0x0E, 0x02, 0x06, 0x08, 0x06, 0x00}, // 0x35 - '5'
    {0x04, 0x02, 0x06, 0x0A, 0x04, 0x00}, // 0x36 - '6'
    {0x0E, 0x08, 0x0C, 0x04, 0x04, 0x00}, // 0x37 - '7'
    {0x04, 0x0A, 0x04, 0x0A, 0x04, 0x00}, // 0x38 - '8'
    {0x04, 0x0A, 0x0C, 0x08, 0x04, 0x00}, // 0x39 - '9'
    {0x00, 0x00, 0x04, 0x00, 0x04, 0x00}, // 0x3A - ':'
    {0x00, 0x00, 0x04, 0x00, 0x06, 0x02}, // 0x3B - ';'
    {0x08, 0x04, 0x02, 0x04, 0x08, 0x00}, // 0x3C - '<'
    {0x00, 0x00, 0x0E, 0x00, 0x0E, 0x00}, // 0x3D - '='
    {0x02, 0x04, 0x08, 0x04, 0x02, 0x00}, // 0x3E - '>'
    {0x06, 0x08, 0x04, 0x00, 0x04, 0x00}, // 0x3F - '?'
    {0x0E, 0x0A, 0x0A, 0x02, 0x0E, 0x00}, // 0x40 - '@'
    {0x04, 0x0A, 0x0E, 0x0A, 0x0A, 0x00}, // 0x41 - 'A'
    {0x06, 0x0A, 0x06, 0x0A, 0x06, 0x00}, // 0x42 - 'B'
    {0x0C, 0x02, 0x02, 0x02, 0x0C, 0x00}, // 0x43 - 'C'
    {0x06, 0x0A, 0x0A, 0x0A, 0x06, 0x00}, // 0x44 - 'D'
    {0x0E, 0x02, 0x06, 0x02, 0x0E, 0x00}, // 0x45 - 'E'
    {0x0E, 0x02, 0x06, 0x02, 0x02, 0x00}, // 0x46 - 'F'
    {0x0C, 0x02, 0x0A, 0x0A, 0x0C, 0x00}, // 0x47 - 'G'
    {0x0A, 0x0A, 0x0E, 0x0A, 0x0A, 0x00}, // 0x48 - 'H'
    {0x0E, 0x04, 0x04, 0x04, 0x0E, 0x00}, // 0x49 - 'I'
    {0x08, 0x08, 0x08, 0x0A, 0x04, 0x00}, // 0x4A - 'J'
    {0x0A, 0x0A, 0x06, 0x0A, 0x0A, 0x00}, // 0x4B - 'K'
    {0x02, 0x02, 0x02, 0x02, 0x0E, 0x00}, // 0x4C - 'L'
    {0x0A, 0x0E, 0x0E, 0x0A, 0x0A, 0x00}, // 0x4D - 'M'
    {0x0A, 0x0E, 0x0A, 0x0A, 0x0A, 0x00}, // 0x4E - 'N'
    {0x04, 0x0A, 0x0A, 0x0A, 0x04, 0x00}, // 0x4F - 'O'
    {0x06, 0x0A, 0x06, 0x02, 0x02, 0x00}, // 0x50 - 'P'
    {0x04, 0x0A, 0x0A, 0x0E, 0x0C, 0x00}, // 0x51 - 'Q'
    {0x06, 0x0A, 0x06, 0x0A, 0x0A, 0x00}, // 0x52 - 'R'
    {0x0C, 0x02, 0x0E, 0x08, 0x06, 0x00}, // 0x53 - 'S'
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x00}, // 0x54 - 'T'
    {0x0A, 0x0A, 0x0A, 0x0A, 0x0E, 0x00}, // 0x55 - 'U'
    {0x0A, 0x0A, 0x0A, 0x0A, 0x04, 0x00}, // 0x56 - 'V'
    {0x0A, 0x0A, 0x0E, 0x0E, 0x0A, 0x00}, // 0x57 - 'W'
    {0x0A, 0x0A, 0x04, 0x0A, 0x0A, 0x00}, // 0x58 - 'X'
    {0x0A, 0x0A, 0x04, 0x04, 0x04, 0x00}, // 0x59 - 'Y'
    {0x0E, 0x08, 0x04, 0x02, 0x0E, 0x00}, // 0x5A - 'Z'
    {0x06, 0x02, 0x02, 0x02, 0x06, 0x00}, // 0x5B - '['
    {0x02, 0x02, 0x04, 0x08, 0x08, 0x00}, // 0x5C - '\'
    {0x06, 0x04, 0x04, 0x04, 0x06, 0x00}, // 0x5D - ']'
    {0x04, 0x0A, 0x00, 0x00, 0x00, 0x00}, // 0x5E - '^'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0F}, // 0x5F - '_'
    {0x06, 0x04, 0x00, 0x00, 0x00, 0x00}, // 0x60 - '`'
    {0x00, 0x00, 0x0C, 0x0A, 0x0E, 0x00}, // 0x61 - 'a'
    {0x02, 0x02, 0x06, 0x0A, 0x06, 0x00}, // 0x62 - 'b'
    {0x00, 0x00, 0x0C, 0x02, 0x0C, 0x00}, // 0x63 - 'c'
    {0x08, 0x08, 0x0C, 0x0A, 0x0C, 0x00}, // 0x64 - 'd'
    {0x00, 0x00, 0x0E, 0x06, 0x0C, 0x00}, // 0x65 - 'e'
    {0x08, 0x04, 0x0E, 0x04, 0x04, 0x00}, // 0x66 - 'f'
    {0x00, 0x00, 0x0E, 0x0A, 0x08, 0x0E}, // 0x67 - 'g'
    {0x02, 0x02, 0x06, 0x0A, 0x0A, 0x00}, // 0x68 - 'h'
    {0x04, 0x00, 0x04, 0x04, 0x04, 0x00}, // 0x69 - 'i'
    {0x04, 0x00, 0x04, 0x04, 0x04, 0x06}, // 0x6A - 'j'
    {0x02, 0x02, 0x0A, 0x06, 0x0A, 0x00}, // 0x6B - 'k'
    {0x04, 0x04, 0x04, 0x04, 0x04, 0x00}, // 0x6C - 'l'
    {0x00, 0x00, 0x0E, 0x0E, 0x0A, 0x00}, // 0x6D - 'm'
    {0x00, 0x00, 0x06, 0x0A, 0x0A, 0x00}, // 0x6E - 'n'
    {0x00, 0x00, 0x04, 0x0A, 0x04, 0x00}, // 0x6F - 'o'
    {0x00, 0x00, 0x06, 0x0A, 0x06, 0x02}, // 0x70 - 'p'
    {0x00, 0x00, 0x0C, 0x0A, 0x0C, 0x08}, // 0x71 - 'q'
    {0x00, 0x00, 0x06, 0x02, 0x02, 0x00}, // 0x72 - 'r'
    {0x00, 0x00, 0x0C, 0x04, 0x06, 0x00}, // 0x73 - 's'
    {0x00, 0x04, 0x0E, 0x04, 0x0C, 0x00}, // 0x74 - 't'
    {0x00, 0x00, 0x0A, 0x0A, 0x0E, 0x00}, // 0x75 - 'u'
    {0x00, 0x00, 0x0A, 0x0A, 0x04, 0x00}, // 0x76 - 'v'
    {0x00, 0x00, 0x0A, 0x0E, 0x0E, 0x00}, // 0x77 - 'w'
    {0x00, 0x00, 0x0A, 0x04, 0x0A, 0x00}, // 0x78 - 'x'
    {0x00, 0x00, 0x0A, 0x0A, 0x04, 0x02}, // 0x79 - 'y'
    {0x00, 0x00, 0x06, 0x04, 0x0C, 0x00}, // 0x7A - 'z'
    {0x0C, 0x04, 0x06, 0x04, 0x0C, 0x00}, // 0x7B - '{'
    {0x04, 0x04, 0x04, 0x04, 0x04, 0x00}, // 0x7C - '|'
    {0x06, 0x04, 0x0C, 0x04, 0x06, 0x00}, // 0x7D - '}'
    {0x0A, 0x05, 0x00, 0x00, 0x00, 0x00}, // 0x7E - '~'
    {0x00, 0x04, 0x0A, 0x0E, 0x00, 0x00}, // 0x7F - ''
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x80 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x81 - '�'
    {0x06, 0x02, 0x00, 0x00, 0x00, 0x00}, // 0x82 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x83 - '�'
    {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00}, // 0x84 - '�'
    {0x00, 0x00, 0x04, 0x00, 0x04, 0x00}, // 0x85 - '�'
    {0x04, 0x04, 0x0F, 0x04, 0x04, 0x04}, // 0x86 - '�'
    {0x04, 0x0F, 0x00, 0x0F, 0x04, 0x04}, // 0x87 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x88 - '�'
    {0x02, 0x08, 0x04, 0x02, 0x08, 0x00}, // 0x89 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8A - '�'
    {0x08, 0x04, 0x02, 0x04, 0x08, 0x00}, // 0x8B - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8C - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8D - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8E - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x8F - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x90 - '�'
    {0x06, 0x02, 0x00, 0x00, 0x00, 0x00}, // 0x91 - '�'
    {0x06, 0x02, 0x00, 0x00, 0x00, 0x00}, // 0x92 - '�'
    {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00}, // 0x93 - '�'
    {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00}, // 0x94 - '�'
    {0x00, 0x00, 0x04, 0x00, 0x00, 0x00}, // 0x95 - '�'
    {0x00, 0x00, 0x0E, 0x00, 0x00, 0x00}, // 0x96 - '�'
    {0x00, 0x00, 0x0E, 0x00, 0x00, 0x00}, // 0x97 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x98 - '�'
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x00}, // 0x99 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9A - '�'
    {0x02, 0x04, 0x08, 0x04, 0x02, 0x00}, // 0x9B - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9C - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9D - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9E - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0x9F - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA0 - '�'
    {0x07, 0x05, 0x07, 0x04, 0x03, 0x00}, // 0xA1 - '�'
    {0x00, 0x02, 0x05, 0x05, 0x02, 0x01}, // 0xA2 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA3 - '�'
    {0x00, 0x07, 0x05, 0x05, 0x07, 0x00}, // 0xA4 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xA5 - '�'
    {0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // 0xA6 - '�'
    {0x0C, 0x06, 0x0A, 0x0C, 0x06, 0x00}, // 0xA7 - '�'
    {0x07, 0x01, 0x03, 0x01, 0x07, 0x00}, // 0xA8 - '�'
    {0x00, 0x00, 0x0C, 0x02, 0x0C, 0x00}, // 0xA9 - '�'
    {0x06, 0x01, 0x07, 0x01, 0x06, 0x00}, // 0xAA - '�'
    {0x08, 0x04, 0x02, 0x04, 0x08, 0x00}, // 0xAB - '�'
    {0x00, 0x00, 0x07, 0x04, 0x04, 0x04}, // 0xAC - '�'
    {0x00, 0x00, 0x0E, 0x00, 0x00, 0x00}, // 0xAD - '�'
    {0x06, 0x0A, 0x06, 0x0A, 0x0A, 0x00}, // 0xAE - '�'
    {0x05, 0x02, 0x02, 0x02, 0x02, 0x00}, // 0xAF - '�'
    {0x04, 0x0A, 0x04, 0x00, 0x00, 0x00}, // 0xB0 - '�'
    {0x00, 0x04, 0x0E, 0x04, 0x00, 0x00}, // 0xB1 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB2 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB3 - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xB4 - '�'
    {0x00, 0x00, 0x05, 0x07, 0x04, 0x00}, // 0xB5 - '�'
    {0x0E, 0x0B, 0x0B, 0x0A, 0x0A, 0x00}, // 0xB6 - '�'
    {0x00, 0x00, 0x04, 0x00, 0x00, 0x00}, // 0xB7 - '�'
    {0x05, 0x00, 0x07, 0x03, 0x06, 0x00}, // 0xB8 - '�'
    {0x05, 0x07, 0x07, 0x07, 0x05, 0x00}, // 0xB9 - '�'
    {0x00, 0x00, 0x06, 0x03, 0x06, 0x00}, // 0xBA - '�'
    {0x02, 0x04, 0x08, 0x04, 0x02, 0x00}, // 0xBB - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xBC - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xBD - '�'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 0xBE - '�'
    {0x00, 0x00, 0x05, 0x02, 0x02, 0x00}, // 0xBF - '�'
    {0x02, 0x05, 0x07, 0x05, 0x05, 0x00}, // 0xC0 - '�'
    {0x07, 0x01, 0x03, 0x05, 0x03, 0x00}, // 0xC1 - '�'
    {0x03, 0x05, 0x03, 0x05, 0x03, 0x00}, // 0xC2 - '�'
    {0x07, 0x01, 0x01, 0x01, 0x01, 0x00}, // 0xC3 - '�'
    {0x02, 0x02, 0x02, 0x07, 0x05, 0x00}, // 0xC4 - '�'
    {0x07, 0x01, 0x03, 0x01, 0x07, 0x00}, // 0xC5 - '�'
    {0x05, 0x07, 0x02, 0x07, 0x05, 0x00}, // 0xC6 - '�'
    {0x07, 0x04, 0x06, 0x04, 0x07, 0x00}, // 0xC7 - '�'
    {0x05, 0x05, 0x07, 0x07, 0x05, 0x00}, // 0xC8 - '�'
    {0x07, 0x05, 0x07, 0x07, 0x05, 0x00}, // 0xC9 - '�'
    {0x05, 0x03, 0x03, 0x05, 0x05, 0x00}, // 0xCA - '�'
    {0x06, 0x05, 0x05, 0x05, 0x05, 0x00}, // 0xCB - '�'
    {0x05, 0x07, 0x05, 0x05, 0x05, 0x00}, // 0xCC - '�'
    {0x05, 0x05, 0x07, 0x05, 0x05, 0x00}, // 0xCD - '�'
    {0x02, 0x05, 0x05, 0x05, 0x02, 0x00}, // 0xCE - '�'
    {0x07, 0x05, 0x05, 0x05, 0x05, 0x00}, // 0xCF - '�'
    {0x03, 0x05, 0x03, 0x01, 0x01, 0x00}, // 0xD0 - '�'
    {0x06, 0x01, 0x01, 0x01, 0x06, 0x00}, // 0xD1 - '�'
    {0x07, 0x02, 0x02, 0x02, 0x02, 0x00}, // 0xD2 - '�'
    {0x05, 0x05, 0x07, 0x04, 0x03, 0x00}, // 0xD3 - '�'
    {0x02, 0x07, 0x05, 0x07, 0x02, 0x00}, // 0xD4 - '�'
    {0x05, 0x05, 0x02, 0x05, 0x05, 0x00}, // 0xD5 - '�'
    {0x05, 0x05, 0x05, 0x05, 0x0F, 0x08}, // 0xD6 - '�'
    {0x05, 0x05, 0x07, 0x04, 0x04, 0x00}, // 0xD7 - '�'
    {0x05, 0x05, 0x07, 0x07, 0x07, 0x00}, // 0xD8 - '�'
    {0x05, 0x05, 0x07, 0x07, 0x0F, 0x08}, // 0xD9 - '�'
    {0x03, 0x02, 0x06, 0x06, 0x06, 0x00}, // 0xDA - '�'
    {0x05, 0x05, 0x07, 0x05, 0x07, 0x00}, // 0xDB - '�'
    {0x01, 0x01, 0x03, 0x05, 0x03, 0x00}, // 0xDC - '�'
    {0x03, 0x04, 0x06, 0x04, 0x03, 0x00}, // 0xDD - '�'
    {0x05, 0x0F, 0x0B, 0x0F, 0x05, 0x00}, // 0xDE - '�'
    {0x06, 0x05, 0x06, 0x05, 0x05, 0x00}, // 0xDF - '�'
    {0x00, 0x00, 0x06, 0x05, 0x06, 0x00}, // 0xE0 - '�'
    {0x00, 0x00, 0x07, 0x03, 0x07, 0x00}, // 0xE1 - '�'
    {0x00, 0x00, 0x03, 0x07, 0x07, 0x00}, // 0xE2 - '�'
    {0x00, 0x00, 0x07, 0x01, 0x01, 0x00}, // 0xE3 - '�'
    {0x00, 0x00, 0x02, 0x07, 0x05, 0x00}, // 0xE4 - '�'
    {0x00, 0x00, 0x07, 0x03, 0x06, 0x00}, // 0xE5 - '�'
    {0x00, 0x00, 0x07, 0x02, 0x07, 0x00}, // 0xE6 - '�'
    {0x00, 0x00, 0x07, 0x06, 0x07, 0x00}, // 0xE7 - '�'
    {0x00, 0x00, 0x05, 0x07, 0x05, 0x00}, // 0xE8 - '�'
    {0x02, 0x00, 0x05, 0x07, 0x05, 0x00}, // 0xE9 - '�'
    {0x00, 0x00, 0x05, 0x03, 0x05, 0x00}, // 0xEA - '�'
    {0x00, 0x00, 0x06, 0x05, 0x05, 0x00}, // 0xEB - '�'
    {0x00, 0x00, 0x05, 0x07, 0x07, 0x00}, // 0xEC - '�'
    {0x00, 0x00, 0x05, 0x07, 0x05, 0x00}, // 0xED - '�'
    {0x00, 0x00, 0x02, 0x05, 0x02, 0x00}, // 0xEE - '�'
    {0x00, 0x00, 0x07, 0x05, 0x05, 0x00}, // 0xEF - '�'
    {0x00, 0x00, 0x03, 0x05, 0x03, 0x01}, // 0xF0 - '�'
    {0x00, 0x00, 0x06, 0x01, 0x06, 0x00}, // 0xF1 - '�'
    {0x00, 0x00, 0x07, 0x02, 0x02, 0x00}, // 0xF2 - '�'
    {0x00, 0x00, 0x05, 0x05, 0x02, 0x01}, // 0xF3 - '�'
    {0x00, 0x00, 0x07, 0x05, 0x07, 0x02}, // 0xF4 - '�'
    {0x00, 0x00, 0x05, 0x02, 0x05, 0x00}, // 0xF5 - '�'
    {0x00, 0x00, 0x05, 0x05, 0x0F, 0x08}, // 0xF6 - '�'
    {0x00, 0x00, 0x05, 0x07, 0x04, 0x00}, // 0xF7 - '�'
    {0x00, 0x00, 0x05, 0x07, 0x07, 0x00}, // 0xF8 - '�'
    {0x00, 0x00, 0x05, 0x07, 0x0F, 0x08}, // 0xF9 - '�'
    {0x00, 0x00, 0x03, 0x06, 0x06, 0x00}, // 0xFA - '�'
    {0x00, 0x00, 0x05, 0x07, 0x07, 0x00}, // 0xFB - '�'
    {0x00, 0x00, 0x01, 0x07, 0x07, 0x00}, // 0xFC - '�'
    {0x00, 0x00, 0x03, 0x06, 0x03, 0x00}, // 0xFD - '�'
    {0x00, 0x00, 0x05, 0x0F, 0x05, 0x00}, // 0xFE - '�'
    {0x00, 0x00, 0x07, 0x06, 0x05, 0x00}  // 0xFF - '�'
  };
