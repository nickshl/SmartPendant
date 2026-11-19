// Double of the generated descriptor header: the descriptor table and the
// unique ID address UsbCdcUart uses. The unique ID is a test array.
#pragma once
#include <cstdint>
#include "usb_device.h"
#define USB_DESC_TYPE_STRING  0x03U
#define USBD_MAX_STR_DESC_SIZ 512U
typedef enum { USBD_SPEED_HIGH = 0U, USBD_SPEED_FULL = 1U, USBD_SPEED_LOW = 2U } USBD_SpeedTypeDef;
typedef struct {
  uint8_t *(*GetDeviceDescriptor)(USBD_SpeedTypeDef speed, uint16_t *length);
  uint8_t *(*GetLangIDStrDescriptor)(USBD_SpeedTypeDef speed, uint16_t *length);
  uint8_t *(*GetManufacturerStrDescriptor)(USBD_SpeedTypeDef speed, uint16_t *length);
  uint8_t *(*GetProductStrDescriptor)(USBD_SpeedTypeDef speed, uint16_t *length);
  uint8_t *(*GetSerialStrDescriptor)(USBD_SpeedTypeDef speed, uint16_t *length);
  uint8_t *(*GetConfigurationStrDescriptor)(USBD_SpeedTypeDef speed, uint16_t *length);
  uint8_t *(*GetInterfaceStrDescriptor)(USBD_SpeedTypeDef speed, uint16_t *length);
} USBD_DescriptorsTypeDef;
extern USBD_DescriptorsTypeDef FS_Desc;
inline uint32_t sim_uid[3] = {0u, 0u, 0u};
#define UID_BASE ((uintptr_t)sim_uid)
