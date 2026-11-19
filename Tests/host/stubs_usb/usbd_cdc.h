#pragma once
#include "usb_device.h"
#define CDC_IN_EP 0x81u
#define CDC_SET_LINE_CODING        0x20U
#define CDC_SET_CONTROL_LINE_STATE 0x22U
typedef struct _USBD_CDC_Itf {
  int8_t (* Init)(void);
  int8_t (* DeInit)(void);
  int8_t (* Control)(uint8_t cmd, uint8_t *pbuf, uint16_t length);
  int8_t (* Receive)(uint8_t *Buf, uint32_t *Len);
  int8_t (* TransmitCplt)(uint8_t *Buf, uint32_t *Len, uint8_t epnum);
} USBD_CDC_ItfTypeDef;
uint8_t USBD_CDC_RegisterInterface(USBD_HandleTypeDef* pdev, USBD_CDC_ItfTypeDef* fops);
uint8_t USBD_CDC_SetRxBuffer(USBD_HandleTypeDef* pdev, uint8_t* pbuff);
struct USBD_CDC_HandleTypeDef {
  uint8_t* RxBuffer;
  uint8_t* TxBuffer;
  uint32_t TxLength;
  volatile uint32_t TxState;
};
uint8_t USBD_CDC_SetTxBuffer(USBD_HandleTypeDef* pdev, uint8_t* pbuff, uint32_t length);
uint8_t USBD_CDC_TransmitPacket(USBD_HandleTypeDef* pdev);
uint8_t USBD_CDC_ReceivePacket(USBD_HandleTypeDef* pdev);
HAL_StatusTypeDef HAL_PCD_EP_Abort(PCD_HandleTypeDef* hpcd, uint8_t ep_addr);
