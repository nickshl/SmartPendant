// Host double of GCodeGeneratorScr: only what the menu comment parser needs.
// The parser functions themselves are extracted from GCodeGeneratorScr.cpp
// unchanged by script_checks.py and compiled against this declaration.
#pragma once
#include <cstdint>
#include "GrblComm.h"
#include "Little-C.h"
struct GCodeGeneratorScr {
  LittleC interpreter;
  bool GetGlobalVariableCommentString(uint32_t variable_idx, char* ptr, uint32_t n, uint32_t pos);
  bool GetGlobalVariableCommentNumber(uint32_t variable_idx, int32_t& value, uint32_t pos);
  bool GetGlobalVariableDescription(uint32_t variable_idx, char* ptr, uint32_t n);
  bool GetGlobalVariableScaler(uint32_t variable_idx, int32_t& scaler);
  bool GetGlobalVariableUnits(uint32_t variable_idx, char* ptr, uint32_t n);
  bool GetGlobalVariableEnumValue(uint32_t variable_idx, char* ptr, uint32_t n, uint32_t idx);
  int32_t GetGlobalVariableEnumCount(uint32_t variable_idx);
  bool IsGlobalVariableEnum(uint32_t variable_idx);
  bool GetGlobalVariableMinVal(uint32_t variable_idx, int32_t& min);
  bool GetGlobalVariableMaxVal(uint32_t variable_idx, int32_t& max);
};
