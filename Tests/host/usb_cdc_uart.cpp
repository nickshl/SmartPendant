// Host test of Application/UsbCdcUart.cpp: the real class over a simulated CDC
// class driver and a simulated host. Checks the IUart contract GrblComm relies
// on(nothing lost or reordered, busy instead of waiting), the flow control,
// and that a closed port, a suspend, a stalled host and a re-enumeration
// neither hang the writer nor leave stale data for the next session.
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <string>
#include <vector>
#include "UsbCdcUart.h"
#include "usbd_cdc.h"
#include "usbd_desc.h"

static int failures = 0;
#define CHECK(c) do { if(!(c)) { std::printf("%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #c); failures++; } } while(0)

// ***   Simulated device stack   **********************************************
USBD_HandleTypeDef hUsbDeviceFS;
uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ];
// Generated serial(usbd_desc.c Get_SerialNum()): word 0 + word 2 as 8 digits,
// then the 4 most significant digits of word 1
static std::string GenSerial(uint32_t w0, uint32_t w1, uint32_t w2)
{
  char buf[16];
  std::snprintf(buf, sizeof(buf), "%08X%04X", (unsigned)(w0 + w2), (unsigned)(w1 >> 16));
  return buf;
}
static uint8_t* GenSerialStrDescriptor(USBD_SpeedTypeDef, uint16_t* length) { *length = 26u; return USBD_StrDesc; }
USBD_DescriptorsTypeDef FS_Desc = {nullptr, nullptr, nullptr, nullptr, GenSerialStrDescriptor, nullptr, nullptr};
// Serial number from a descriptor, checking its format
static std::string SerialText(const uint8_t* desc, uint16_t length)
{
  CHECK(length == desc[0] && desc[1] == USB_DESC_TYPE_STRING && (length % 2u) == 0u);
  std::string text;
  for(uint16_t i = 2u; i < length; i += 2u) { CHECK(desc[i + 1u] == 0u); text += (char)desc[i]; }
  return text;
}
static PCD_HandleTypeDef hpcd;
static USBD_CDC_HandleTypeDef hcdc_mem;  // USBD_static_malloc(): never freed
static uint8_t ep_rx_buf[64];            // UserRxBufferFS
static bool out_armed = false;           // OUT endpoint can take a packet
static bool locked_ok = true;            // every stack call made with the lock
static bool in_irq = false;              // simulated USB interrupt is running
static int aborts = 0;
static int double_arm = 0;

uint8_t USBD_CDC_SetTxBuffer(USBD_HandleTypeDef* pdev, uint8_t* pbuff, uint32_t length)
{
  if(sim_critical == 0u) locked_ok = false;
  USBD_CDC_HandleTypeDef* h = (USBD_CDC_HandleTypeDef*)pdev->pClassData;
  if(h == nullptr) return USBD_FAIL;
  h->TxBuffer = pbuff; h->TxLength = length;
  return USBD_OK;
}
uint8_t USBD_CDC_TransmitPacket(USBD_HandleTypeDef* pdev)
{
  if(sim_critical == 0u) locked_ok = false;
  USBD_CDC_HandleTypeDef* h = (USBD_CDC_HandleTypeDef*)pdev->pClassData;
  if(h == nullptr) return USBD_FAIL;
  if(h->TxState != 0u) return USBD_BUSY;
  h->TxState = 1u;
  pdev->ep_in[1].total_length = h->TxLength;
  return USBD_OK;
}
uint8_t USBD_CDC_ReceivePacket(USBD_HandleTypeDef* pdev)
{
  // From the task only with the USB interrupt masked
  if(!in_irq && (sim_critical == 0u)) locked_ok = false;
  if(out_armed) double_arm++;
  out_armed = true;
  return USBD_OK;
}
uint8_t USBD_CDC_SetRxBuffer(USBD_HandleTypeDef* pdev, uint8_t* pbuff)
{
  if(!in_irq && (sim_critical == 0u)) locked_ok = false;
  ((USBD_CDC_HandleTypeDef*)pdev->pClassData)->RxBuffer = pbuff;
  return USBD_OK;
}
uint8_t USBD_CDC_RegisterInterface(USBD_HandleTypeDef* pdev, USBD_CDC_ItfTypeDef* fops)
{
  pdev->pUserData[pdev->classId] = fops;
  return USBD_OK;
}
static USBD_CDC_ItfTypeDef* Fops() { return (USBD_CDC_ItfTypeDef*)hUsbDeviceFS.pUserData[0]; }

// ***   Generated interface(usbd_cdc_if.c)   **********************************
static int8_t GenInit() { USBD_CDC_SetRxBuffer(&hUsbDeviceFS, ep_rx_buf); return 0; }
static int8_t GenDeInit() { return 0; }
static int8_t GenControl(uint8_t, uint8_t*, uint16_t) { return 0; }
static int gen_received = 0;
static int8_t GenReceive(uint8_t* buf, uint32_t* len) { gen_received += (int)*len; USBD_CDC_SetRxBuffer(&hUsbDeviceFS, buf); USBD_CDC_ReceivePacket(&hUsbDeviceFS); return 0; }
static USBD_CDC_ItfTypeDef gen_fops = {GenInit, GenDeInit, GenControl, GenReceive, nullptr};
// MX_USB_DEVICE_Init(): USBD_Init() clears the interface, then the class and
// the generated interface are registered
static void MxUsbDeviceInit()
{
  hUsbDeviceFS.pUserData[0] = nullptr;
  hUsbDeviceFS.pClass[0] = &hcdc_mem;
  hUsbDeviceFS.pUserData[0] = &gen_fops;
}
HAL_StatusTypeDef HAL_PCD_EP_Abort(PCD_HandleTypeDef*, uint8_t ep)
{
  if(sim_critical == 0u) locked_ok = false;
  CHECK(ep == CDC_IN_EP);
  aborts++;
  return HAL_OK;
}

// ***   Simulated host   ******************************************************
// Enumeration, as USBD_CDC_Init(): the interface's Init sets the receive
// buffer, the transmit state is cleared, OUT is armed into that buffer
static void Configure()
{
  hUsbDeviceFS.pClassData = &hcdc_mem;
  hUsbDeviceFS.pData = &hpcd;
  hcdc_mem.RxBuffer = nullptr;
  in_irq = true;
  Fops()->Init();
  in_irq = false;
  CHECK(hcdc_mem.RxBuffer != nullptr);
  hcdc_mem.TxState = 0u;
  hUsbDeviceFS.dev_state = USBD_STATE_CONFIGURED;
  out_armed = true;
}
// Bus reset/unplug: class deinit
static void Deconfigure()
{
  in_irq = true;
  Fops()->DeInit();
  in_irq = false;
  hUsbDeviceFS.pClassData = nullptr;
  hUsbDeviceFS.dev_state = USBD_STATE_DEFAULT;
  out_armed = false;
}
// Class request without data stage: the request itself is passed
static void ClassRequest(uint8_t request, uint16_t value)
{
  USBD_SetupReqTypedef req = {0x21u, request, value, 0u, 0u};
  in_irq = true;
  CHECK(Fops()->Control(req.bRequest, (uint8_t*)&req, 0u) == 0);
  in_irq = false;
}
// Host opens(DTR set) or closes(DTR dropped) the port
static void SetDtr(bool on) { ClassRequest(CDC_SET_CONTROL_LINE_STATE, on ? 0x0003u : 0x0000u); }
// Host reads the pending IN transfer: data taken from the caller's buffer at
// this moment, as the hardware does
static std::string HostTake()
{
  std::string s;
  if(hcdc_mem.TxState != 0u)
  {
    s.assign((const char*)hcdc_mem.TxBuffer, hcdc_mem.TxLength);
    hcdc_mem.TxState = 0u;
  }
  return s;
}
// Host sends packets until the device holds it off. Bounded: a device that
// never does fails here instead of hanging the test.
static bool HostSend(const uint8_t* data, uint32_t len);
static void HostFill()
{
  uint8_t pkt[64] = {0};
  int n = 0;
  while((n < 100) && HostSend(pkt, 64u)) n++;
  CHECK(n < 100);
}
// Host sends one packet if the device takes it(otherwise NAK, false)
static bool HostSend(const uint8_t* data, uint32_t len)
{
  if(!out_armed) return false;
  out_armed = false;
  // Lands in the buffer the endpoint was armed with
  std::memcpy(hcdc_mem.RxBuffer, data, len);
  uint32_t l = len;
  in_irq = true;
  Fops()->Receive(hcdc_mem.RxBuffer, &l);
  in_irq = false;
  return true;
}

int main()
{
  // UART interface: no object, the generated callbacks stay
  MxUsbDeviceInit();
  UsbCdcUart_RegisterInterface();
  CHECK(Fops() == &gen_fops);

  // UART interface: the generated serial number stays
  CHECK(FS_Desc.GetSerialStrDescriptor == GenSerialStrDescriptor);

  UsbCdcUart* uart = UsbCdcUart::Create();
  CHECK(uart != nullptr);
  // USB interface: all 96 bits of the unique ID, word 0 first. The example
  // is the one ST staff discussed(bytes AF 00 40 00 14 50 36 42 48 37 30 20).
  {
    CHECK(FS_Desc.GetSerialStrDescriptor != GenSerialStrDescriptor);
    sim_uid[0] = 0x004000AFu; sim_uid[1] = 0x42365014u; sim_uid[2] = 0x20303748u;
    uint16_t length = 0u;
    uint8_t* desc = FS_Desc.GetSerialStrDescriptor(USBD_SPEED_FULL, &length);
    CHECK(desc == USBD_StrDesc && length == UsbCdcUart::SERIAL_DESC_SIZE && length == 50u);
    CHECK(SerialText(desc, length) == "004000AF4236501420303748");
    // Hex digits are upper case, every word starts at its most significant digit
    uint8_t buf[UsbCdcUart::SERIAL_DESC_SIZE + 2u] = {0};
    buf[UsbCdcUart::SERIAL_DESC_SIZE] = 0xA5u;
    length = UsbCdcUart::BuildSerialDescriptor(0x01234567u, 0x89ABCDEFu, 0xFEDCBA98u, buf);
    CHECK(SerialText(buf, length) == "0123456789ABCDEFFEDCBA98");
    CHECK(buf[UsbCdcUart::SERIAL_DESC_SIZE] == 0xA5u);          // nothing written past the end
    // Same lot and position, another wafer: the generated serial is the same,
    // this one isn't
    uint8_t other[UsbCdcUart::SERIAL_DESC_SIZE];
    length = UsbCdcUart::BuildSerialDescriptor(0x004000AFu, 0x42365015u, 0x20303748u, other);
    CHECK(GenSerial(0x004000AFu, 0x42365014u, 0x20303748u) == GenSerial(0x004000AFu, 0x42365015u, 0x20303748u));
    CHECK(SerialText(other, length) != SerialText(desc, UsbCdcUart::SERIAL_DESC_SIZE));
  }
  CHECK(UsbCdcUart::Create() == uart);
  CHECK(uart->Init() == Result::RESULT_OK);
  CHECK(uart->SetBaudRate(115200u) == Result::RESULT_OK);

  // USB interface: registered once, right after MX_USB_DEVICE_Init() has
  // registered the generated ones(usb_device.c, USER CODE). Read() doesn't
  // touch the registration.
  {
    MxUsbDeviceInit();
    UsbCdcUart_RegisterInterface();
    CHECK(Fops() != &gen_fops && Fops() != nullptr && Fops()->TransmitCplt == nullptr);
    USBD_CDC_ItfTypeDef* ours = Fops();
    hUsbDeviceFS.pUserData[0] = &gen_fops;
    uint8_t c;
    CHECK(uart->Read(c) == Result::ERR_UART_EMPTY);
    CHECK(Fops() == &gen_fops);
    // The host enumerated and sent a packet before the registration: the
    // generated code takes it. After it, the packet in the generated buffer
    // is taken and the next one goes to ours.
    Configure();
    uint8_t first[3] = {'a', 'b', 'c'};
    CHECK(HostSend(first, 3u) && gen_received == 3);
    UsbCdcUart_RegisterInterface();
    CHECK(Fops() == ours);
    uint8_t second[2] = {'d', 'e'};
    uint8_t* gen_buf = hcdc_mem.RxBuffer;
    CHECK(HostSend(second, 2u));
    CHECK(uart->Read(c) == Result::RESULT_OK && c == 'd' && uart->Read(c) == Result::RESULT_OK && c == 'e');
    CHECK(HostSend(second, 2u) && hcdc_mem.RxBuffer != gen_buf);
    CHECK(uart->Read(c) == Result::RESULT_OK && uart->Read(c) == Result::RESULT_OK && uart->Read(c) == Result::ERR_UART_EMPTY);
    // Unplugged
    Deconfigure();
  }

  // Not enumerated: writes discarded, never busy, nothing to read
  {
    uint8_t msg[] = "G0X1\n";
    CHECK(!uart->IsLinkOpen());
    CHECK(uart->IsTxComplete());
    CHECK(uart->Write(msg, 5u) == Result::RESULT_OK);
    CHECK(uart->IsTxComplete());
    uint8_t c;
    CHECK(uart->Read(c) == Result::ERR_UART_EMPTY);
    uint8_t buf[4]; uint32_t n = 4u;
    CHECK(uart->Read(buf, n) == Result::ERR_UART_EMPTY);
    CHECK(n == 0u);
    CHECK(uart->Write(nullptr, 1u) == Result::ERR_NULL_PTR);
  }

  // Configured, but the port not opened: still discarded. Line coding
  // changes nothing.
  Configure();
  {
    uint8_t msg[] = "?";
    ClassRequest(CDC_SET_LINE_CODING, 0u);
    CHECK(!uart->IsLinkOpen());
    CHECK(uart->Write(msg, 1u) == Result::RESULT_OK);
    CHECK(hcdc_mem.TxState == 0u);
  }

  // Open: one transfer at a time, busy until the host takes it, sent from
  // the caller's buffer
  SetDtr(true);
  {
    CHECK(uart->IsLinkOpen());
    uint8_t msg[] = "$J=G91X1F100\n";
    CHECK(uart->Write(msg, 13u) == Result::RESULT_OK);
    CHECK(!uart->IsTxComplete());
    uint8_t rt = 0x85u;
    CHECK(uart->Write(&rt, 1u) == Result::ERR_UART_BUSY);
    RtosTick::now += 1u;
    CHECK(!uart->IsTxComplete());
    CHECK(HostTake() == "$J=G91X1F100\n");
    CHECK(uart->IsTxComplete());
    CHECK(uart->Write(&rt, 1u) == Result::RESULT_OK);
    CHECK(HostTake() == std::string(1, (char)0x85));
    CHECK(aborts == 0);
  }

  // Receive: random packets and random reads - nothing lost, nothing
  // reordered, the host is held off(NAK) rather than overrun
  {
    std::vector<uint8_t> sent, got;
    srand(12345u);
    uint32_t naks = 0u;
    for(int step = 0; step < 200000; step++)
    {
      if(rand() % 3 != 0)
      {
        uint8_t pkt[64];
        uint32_t len = 1u + (uint32_t)(rand() % 64);
        for(uint32_t i = 0u; i < len; i++) pkt[i] = (uint8_t)rand();
        if(HostSend(pkt, len)) sent.insert(sent.end(), pkt, pkt + len);
        else naks++;
      }
      else
      {
        uint8_t buf[40]; uint32_t n = 1u + (uint32_t)(rand() % 40);
        Result r = uart->Read(buf, n);
        CHECK((r == Result::RESULT_OK) == (n != 0u));
        got.insert(got.end(), buf, buf + n);
      }
    }
    // Drain
    for(int i = 0; i < 1000; i++)
    {
      uint8_t c;
      if(uart->Read(c) != Result::RESULT_OK) break;
      got.push_back(c);
    }
    CHECK(naks > 0u);          // flow control was exercised
    CHECK(sent == got);
    CHECK(out_armed);          // re-armed once there was room
    CHECK(double_arm == 0);
    std::printf("USB CDC receive: %zu bytes, %u packets held off\n", got.size(), naks);
  }

  // Port closed with a transfer pending: aborted at once, so the next
  // session doesn't get it; writes discarded while closed, never busy
  {
    uint8_t msg[] = "$J=G91X10F1000\n";
    CHECK(uart->Write(msg, 15u) == Result::RESULT_OK);
    CHECK(!uart->IsTxComplete());
    SetDtr(false);
    CHECK(uart->IsTxComplete());
    CHECK(aborts == 1);
    CHECK(hcdc_mem.TxState == 0u);
    CHECK(hUsbDeviceFS.ep_in[1].total_length == 0u);
    for(int i = 0; i < 100; i++) CHECK(uart->Write(msg, 15u) == Result::RESULT_OK);
    CHECK(hcdc_mem.TxState == 0u);
    SetDtr(true);
    CHECK(HostTake() == "");
    CHECK(uart->Write(msg, 15u) == Result::RESULT_OK);
    CHECK(HostTake() == "$J=G91X10F1000\n");
  }

  // Host keeps DTR but doesn't read(closed with stty -hupcl, or stalled):
  // busy until the timeout, then aborted
  {
    uint8_t msg[] = "G4P1\n";
    CHECK(uart->Write(msg, 5u) == Result::RESULT_OK);
    RtosTick::now += 249u;
    CHECK(!uart->IsTxComplete());
    CHECK(uart->Write(msg, 5u) == Result::ERR_UART_BUSY);
    RtosTick::now += 1u;
    CHECK(uart->IsTxComplete());
    CHECK(aborts == 2);
    CHECK(uart->Write(msg, 5u) == Result::RESULT_OK);
    CHECK(HostTake() == "G4P1\n");
  }
  // Timeout across the 32 bit millisecond wrap
  {
    RtosTick::now = 0xFFFFFFF0u;
    uint8_t msg[] = "?";
    CHECK(uart->Write(msg, 1u) == Result::RESULT_OK);
    RtosTick::now += 100u;
    CHECK(!uart->IsTxComplete());
    RtosTick::now += 150u;
    CHECK(uart->IsTxComplete());
    CHECK(aborts == 3);
  }

  // Suspend(cable pulled with a self powered device): link closed at once,
  // pending transfer aborted, writes discarded; resume restores the link
  {
    uint8_t msg[] = "?";
    CHECK(uart->Write(msg, 1u) == Result::RESULT_OK);
    hUsbDeviceFS.dev_state = USBD_STATE_SUSPENDED;
    CHECK(!uart->IsLinkOpen());
    CHECK(uart->IsTxComplete());
    CHECK(aborts == 4);
    CHECK(uart->Write(msg, 1u) == Result::RESULT_OK && hcdc_mem.TxState == 0u);
    hUsbDeviceFS.dev_state = USBD_STATE_CONFIGURED;
    CHECK(uart->IsLinkOpen());
  }

  // Re-enumeration while receive is held off: the new configuration arms the
  // endpoint and clears the pause; the reader doesn't arm it a second time;
  // the port is closed until the host opens it again
  {
    HostFill();
    CHECK(!out_armed);
    // Suspended: room is made, but the endpoint can't be armed - stays paused
    hUsbDeviceFS.dev_state = USBD_STATE_SUSPENDED;
    uint8_t c;
    for(int i = 0; i < 64; i++) CHECK(uart->Read(c) == Result::RESULT_OK);
    CHECK(!out_armed);
    hUsbDeviceFS.dev_state = USBD_STATE_CONFIGURED;
    CHECK(uart->Read(c) == Result::RESULT_OK);
    CHECK(out_armed);
    HostFill();
    Deconfigure();
    CHECK(!uart->IsLinkOpen());
    uint8_t msg[] = "?";
    CHECK(uart->Write(msg, 1u) == Result::RESULT_OK);
    Configure();
    CHECK(!uart->IsLinkOpen());
    for(int i = 0; (i < 1000) && (uart->Read(c) == Result::RESULT_OK); i++) {}
    CHECK(double_arm == 0);
    SetDtr(true);
    CHECK(uart->Write(msg, 1u) == Result::RESULT_OK);
    CHECK(HostTake() == "?");
    // A new configuration alone(the stack deinitializes first, but Init
    // doesn't rely on it) closes the port too
    Configure();
    CHECK(!uart->IsLinkOpen());
  }

  CHECK(locked_ok);
  CHECK(sim_critical == 0u);   // every enter has its exit
  if(failures == 0) std::printf("USB CDC interface tests passed\n");
  return failures == 0 ? 0 : 1;
}
