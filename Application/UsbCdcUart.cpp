//******************************************************************************
//  @file UsbCdcUart.cpp
//  @author Nicolai Shlapunov
//
//  @details UsbCdcUart: IUart over the USB CDC virtual COM port, implementation
//
//  @copyright Copyright (c) 2016-2026, Devtronic & Nicolai Shlapunov
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
#include "UsbCdcUart.h"

#include <new> // For std::nothrow

#include "usb_device.h"
#include "usbd_cdc.h"
#include "usbd_desc.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

// Shared string descriptor buffer of the generated descriptors(usbd_desc.c):
// only one descriptor is sent at a time
extern uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ];
static_assert(USBD_MAX_STR_DESC_SIZ >= UsbCdcUart::SERIAL_DESC_SIZE, "Serial number doesn't fit the string buffer");

// The debug mirror in GrblComm writes to the same IN endpoint. GrblComm keeps
// it quiet while the USB interface is in use(GrblComm::SendToUsb()), so the
// two never share the endpoint - see there.

// *****************************************************************************
// ***   Static members   ******************************************************
// *****************************************************************************
UsbCdcUart* UsbCdcUart::instance = nullptr;

// *****************************************************************************
// ***   Local: CDC interface table   ******************************************
// *****************************************************************************
// Const - in flash. The class driver only calls through it.
static const USBD_CDC_ItfTypeDef cdc_fops =
{
  UsbCdcUart::CdcInit,
  UsbCdcUart::CdcDeInit,
  UsbCdcUart::CdcControl,
  UsbCdcUart::CdcReceive,
  nullptr // TransmitCplt: the class driver clears TxState, IsTxComplete() reads it
};

// *****************************************************************************
// ***   Local: SerialStrDescriptor   ******************************************
// *****************************************************************************
// Serial number string descriptor callback, in place of the generated one.
// Called by the USB stack in the interrupt when the host asks for it.
static uint8_t* SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
{
  (void)speed;
  *length = UsbCdcUart::BuildSerialDescriptor(*(uint32_t*)(UID_BASE), *(uint32_t*)(UID_BASE + 4u), *(uint32_t*)(UID_BASE + 8u), USBD_StrDesc);
  return USBD_StrDesc;
}

// *****************************************************************************
// ***   Public: BuildSerialDescriptor   ***************************************
// *****************************************************************************
uint16_t UsbCdcUart::BuildSerialDescriptor(uint32_t word0, uint32_t word1, uint32_t word2, uint8_t* desc)
{
  const uint32_t words[3u] = {word0, word1, word2};
  uint32_t idx = 2u;
  // Descriptor: length, type, then UTF-16LE characters
  desc[0u] = (uint8_t)SERIAL_DESC_SIZE;
  desc[1u] = USB_DESC_TYPE_STRING;
  for(uint32_t w = 0u; w < NumberOf(words); w++)
  {
    // Most significant digit first
    for(int32_t shift = 28; shift >= 0; shift -= 4)
    {
      uint8_t digit = (uint8_t)((words[w] >> shift) & 0xFu);
      desc[idx++] = (digit < 10u) ? (uint8_t)('0' + digit) : (uint8_t)('A' + digit - 10u);
      desc[idx++] = 0u;
    }
  }
  return (uint16_t)SERIAL_DESC_SIZE;
}

// *****************************************************************************
// ***   Public: Create   ******************************************************
// *****************************************************************************
UsbCdcUart* UsbCdcUart::Create()
{
  // Only one object - the interface callbacks have no way to tell two apart
  if(instance == nullptr)
  {
    // Heap: static RAM is full, and the UART user shouldn't pay for it.
    // Runs before the scheduler starts and before USB is started, so no
    // interrupt or task can see the object half made.
    instance = new(std::nothrow) UsbCdcUart();
    // USB mode: serial number from the whole unique ID, see UsbCdcUart.h.
    // The stack reads the table when the host asks, USB isn't started yet.
    if(instance != nullptr) FS_Desc.GetSerialStrDescriptor = SerialStrDescriptor;
  }
  return instance;
}

// *****************************************************************************
// ***   Public: Read   ********************************************************
// *****************************************************************************
Result UsbCdcUart::Read(uint8_t& rx_byte)
{
  Result result = Result::ERR_UART_EMPTY;

  uint32_t tail = rx_tail;
  // Check data
  if(tail != rx_head)
  {
    // Get byte. It is read before rx_tail gives its place back to the
    // interrupt: both are volatile, so the compiler keeps the order, and one
    // core without cache needs nothing more.
    rx_byte = rx_buf[tail & (RX_BUF_SIZE - 1u)];
    rx_tail = tail + 1u;
    result = Result::RESULT_OK;
  }

  // Room may have been made for a packet the host is waiting to send
  if(rx_paused) ResumeRx();

  return result;
}

// *****************************************************************************
// ***   Public: Read   ********************************************************
// *****************************************************************************
Result UsbCdcUart::Read(uint8_t* rx_buf_ptr, uint32_t& size)
{
  Result result = Result::ERR_NULL_PTR;

  if(rx_buf_ptr != nullptr)
  {
    uint32_t rx_idx = 0u;
    // Read bytes in cycle
    while((rx_idx < size) && (Read(rx_buf_ptr[rx_idx]) == Result::RESULT_OK))
    {
      rx_idx++;
    }
    // Same as the UART: OK if anything was read, empty otherwise
    result = (rx_idx != 0u) ? Result::RESULT_OK : Result::ERR_UART_EMPTY;
    // Save read bytes count
    size = rx_idx;
  }

  return result;
}

// *****************************************************************************
// ***   Public: Write   *******************************************************
// *****************************************************************************
Result UsbCdcUart::Write(uint8_t* tx_buf_ptr, uint32_t size)
{
  Result result = Result::ERR_NULL_PTR;

  // Check input parameters
  if(tx_buf_ptr != nullptr)
  {
    if(IsTxPending())
    {
      // Previous transfer isn't finished - caller retries
      result = Result::ERR_UART_BUSY;
    }
    else if((size == 0u) || !IsLinkOpen())
    {
      // Nobody listens: discard, like a UART with nothing on the cable. The
      // status watchdog in GrblComm finds out the controller is gone.
      result = Result::RESULT_OK;
    }
    else
    {
      uint8_t usb_result = USBD_FAIL;
      // Same handle and endpoint registers as the USB interrupt: it is
      // masked meanwhile(priority 5, configMAX_SYSCALL_INTERRUPT_PRIORITY)
      Rtos::EnterCriticalSection();
      if(USBD_CDC_SetTxBuffer(&hUsbDeviceFS, tx_buf_ptr, size) == USBD_OK)
      {
        usb_result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
      }
      Rtos::ExitCriticalSection();
      // Convert operation result to Result
      if(usb_result == USBD_OK)
      {
        tx_timestamp = RtosTick::GetTimeMs();
        result = Result::RESULT_OK;
      }
      else if(usb_result == USBD_BUSY)
      {
        result = Result::ERR_UART_BUSY;
      }
      else
      {
        result = Result::ERR_UART_TRANSMIT;
      }
    }
  }

  return result;
}

// *****************************************************************************
// ***   Public: IsTxComplete   ************************************************
// *****************************************************************************
bool UsbCdcUart::IsTxComplete(void)
{
  return !IsTxPending();
}

// *****************************************************************************
// ***   Public: IsLinkOpen   **************************************************
// *****************************************************************************
bool UsbCdcUart::IsLinkOpen(void)
{
  return (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) && dtr;
}

// *****************************************************************************
// ***   Private: OnReceive   **************************************************
// *****************************************************************************
bool UsbCdcUart::OnReceive(const uint8_t* buf, uint32_t len)
{
  uint32_t head = rx_head;
  // Room in the ring. The endpoint is armed only with room for a packet, so
  // nothing is cut here; the check only keeps a broken host from overwriting.
  uint32_t room = RX_BUF_SIZE - (head - rx_tail);
  if(len > room) len = room;
  // Copy data. The task can't run in the middle of the interrupt, so it
  // sees the data and rx_head together.
  for(uint32_t i = 0u; i < len; i++)
  {
    rx_buf[(head + i) & (RX_BUF_SIZE - 1u)] = buf[i];
  }
  rx_head = head + len;
  // Arm again only if the next packet fits, otherwise the host gets NAK
  // until Read() makes room
  bool rearm = ((room - len) >= PACKET_SIZE);
  if(!rearm) rx_paused = true;
  return rearm;
}

// *****************************************************************************
// ***   Private: IsTxPending   ************************************************
// *****************************************************************************
bool UsbCdcUart::IsTxPending(void)
{
  bool pending = false;

  USBD_CDC_HandleTypeDef* hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  // Memory comes from USBD_static_malloc(), so the pointer stays valid even
  // if the interrupt deinitializes the class meanwhile
  if((hcdc != nullptr) && (hcdc->TxState != 0u))
  {
    // Nobody will take it: the port closed, the device was suspended or
    // unplugged, or the host stopped reading. Abort, so it isn't delivered
    // to whoever opens the port next - a stale command executed by a new
    // session.
    if(!IsLinkOpen() || RtosTick::CheckTimeDifferenceMs(tx_timestamp, TX_TIMEOUT_MS))
    {
      // Same handle and endpoint registers as the USB interrupt
      Rtos::EnterCriticalSection();
      // Check again with the USB interrupt masked - it may have completed
      // meanwhile
      if(hcdc->TxState != 0u)
      {
        // Disable the endpoint. The EPDISD interrupt that follows flushes its
        // FIFO(HAL_PCD_IRQHandler()), the next transfer enables it again.
        (void)HAL_PCD_EP_Abort((PCD_HandleTypeDef*)hUsbDeviceFS.pData, CDC_IN_EP);
        // No DataIn will come for it, so the class state is cleared here
        hUsbDeviceFS.ep_in[CDC_IN_EP & 0xFU].total_length = 0u;
        hcdc->TxState = 0u;
      }
      Rtos::ExitCriticalSection();
    }
    else
    {
      pending = true;
    }
  }

  return pending;
}

// *****************************************************************************
// ***   Private: ResumeRx   ***************************************************
// *****************************************************************************
void UsbCdcUart::ResumeRx(void)
{
  // Room for a whole packet?
  if((RX_BUF_SIZE - (rx_head - rx_tail)) >= PACKET_SIZE)
  {
    // Same handle and endpoint registers as the USB interrupt
    Rtos::EnterCriticalSection();
    // Still paused(a new configuration arms the endpoint itself) and the
    // endpoint can be armed. If it can't now(suspended) it stays paused and
    // the next Read() tries again.
    if(rx_paused && (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED))
    {
      rx_paused = false;
      (void)USBD_CDC_SetRxBuffer(&hUsbDeviceFS, rx_packet);
      (void)USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    }
    Rtos::ExitCriticalSection();
  }
}

// *****************************************************************************
// ***   Private: CdcInit   ****************************************************
// *****************************************************************************
int8_t UsbCdcUart::CdcInit(void)
{
  // Configured: the class driver arms the OUT endpoint with this buffer when
  // this returns and clears the transmit state itself
  (void)USBD_CDC_SetRxBuffer(&hUsbDeviceFS, instance->rx_packet);
  instance->rx_paused = false;
  // Port is closed until the host opens it with DTR
  instance->dtr = false;
  return (int8_t)USBD_OK;
}

// *****************************************************************************
// ***   Private: CdcDeInit   **************************************************
// *****************************************************************************
int8_t UsbCdcUart::CdcDeInit(void)
{
  // Reset, unplug or new configuration: port closed. Nothing to release,
  // the ring keeps what was received, the transmit state is the class
  // driver's.
  instance->dtr = false;
  return (int8_t)USBD_OK;
}

// *****************************************************************************
// ***   Private: CdcControl   *************************************************
// *****************************************************************************
int8_t UsbCdcUart::CdcControl(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  // Only DTR matters: the host has the port open. Line coding has no meaning
  // for a virtual port; GET_LINE_CODING isn't answered, as in the generated
  // code. SET_CONTROL_LINE_STATE has no data stage: pbuf is the setup request.
  (void)length;
  if((cmd == CDC_SET_CONTROL_LINE_STATE) && (pbuf != nullptr))
  {
    instance->dtr = ((((USBD_SetupReqTypedef*)pbuf)->wValue & 0x0001u) != 0u);
  }
  return (int8_t)USBD_OK;
}

// *****************************************************************************
// ***   Private: CdcReceive   *************************************************
// *****************************************************************************
int8_t UsbCdcUart::CdcReceive(uint8_t* buf, uint32_t* len)
{
  // Arm again only if the next packet fits, otherwise the host gets NAK
  // until Read() makes room. buf may be the generated buffer if the host
  // enumerated before UsbCdcUart_RegisterInterface(): the next packet goes
  // to ours.
  if(instance->OnReceive(buf, *len))
  {
    (void)USBD_CDC_SetRxBuffer(&hUsbDeviceFS, instance->rx_packet);
    (void)USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  }
  return (int8_t)USBD_OK;
}

// *****************************************************************************
// ***   Global: UsbCdcUart_RegisterInterface   ********************************
// *****************************************************************************
extern "C" void UsbCdcUart_RegisterInterface(void)
{
  // UART interface: the generated callbacks stay
  if(UsbCdcUart::GetInstance() != nullptr)
  {
    // The class driver only calls through the table, it is never written.
    // No interrupt lock: the registration is one pointer store, and the
    // USB interrupt may already run(USBD_Start() is done) - a request it
    // handles before this goes to the generated callbacks, which are fine.
    (void)USBD_CDC_RegisterInterface(&hUsbDeviceFS, const_cast<USBD_CDC_ItfTypeDef*>(&cdc_fops));
  }
}
