// Double of the ST USB device stack: the members and functions UsbCdcUart
// uses, with the behaviour of the class driver(usbd_cdc.c) they depend on.
#pragma once
#include <cstdint>
#define USBD_OK   0u
#define USBD_BUSY 1u
#define USBD_FAIL 3u
#define USBD_STATE_DEFAULT    1u
#define USBD_STATE_CONFIGURED 3u
#define USBD_STATE_SUSPENDED  4u
struct USBD_EndpointTypeDef { uint32_t total_length; };
struct USBD_HandleTypeDef {
  volatile uint8_t dev_state = USBD_STATE_DEFAULT;
  uint8_t classId = 0u;
  void* pClass[1] = {nullptr};
  void* pUserData[1] = {nullptr};
  void* pClassData = nullptr;
  void* pData = nullptr;
  USBD_EndpointTypeDef ep_in[16] = {};
};
struct PCD_HandleTypeDef { int dummy; };
struct USBD_SetupReqTypedef { uint8_t bmRequest; uint8_t bRequest; uint16_t wValue; uint16_t wIndex; uint16_t wLength; };
enum HAL_StatusTypeDef { HAL_OK = 0, HAL_ERROR = 1 };
