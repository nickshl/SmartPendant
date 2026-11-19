//******************************************************************************
//  @file UsbCdcUart.h
//  @author Nicolai Shlapunov
//
//  @details UsbCdcUart: IUart over the USB CDC virtual COM port, header
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
//
// Lets GrblComm talk to a host program on a PC that speaks the grblHAL protocol
// over the USB virtual COM port instead of the UART. GrblComm sees the same
// IUart contract as with StHalUart:
//
// - Read() returns bytes from a ring buffer filled by the CDC receive
//   callback, ERR_UART_EMPTY when there are none.
// - Write() transmits straight from the caller's buffer, like the UART DMA
//   does, so the buffer must stay untouched until IsTxComplete() is true.
//   It never waits: while a transfer is pending it returns ERR_UART_BUSY and
//   GrblComm re-queues the message.
//
// Flow control instead of loss on receive: the OUT endpoint is armed again
// only while the ring has room for a whole packet. Otherwise the host gets
// NAK until Read() has made room and re-armed it.
//
// The link counts as open while the device is configured and the host holds
// DTR. Linux(cdc-acm) sets DTR when a program opens the tty and drops it when
// the port is closed - no code on the PC side. A closed link behaves like a
// UART with nothing on the other end of the cable: writes are accepted and
// discarded, and a transfer still pending is aborted at once. Otherwise it
// would be delivered when the port is opened again - a command the previous
// session never received, executed by the next one. A transfer the host
// doesn't take within TX_TIMEOUT_MS is aborted too: a host that keeps DTR
// after closing(stty -hupcl) or stopped reading.
//
// The CDC class driver delivers received data and DTR only through the
// interface callbacks(USBD_CDC_ItfTypeDef); there is nothing to poll. This class has
// its own and installs them with USBD_CDC_RegisterInterface() in place of
// the generated ones(usbd_cdc_if.c): UsbCdcUart_RegisterInterface() is
// called once from the USB_DEVICE_Init_PostTreatment USER CODE section of
// usb_device.c, after MX_USB_DEVICE_Init() has registered the generated
// ones. Without an object(UART interface) it does nothing: the generated
// ones stay, as does the debug mirror that uses them.
//
// The serial number string(USB mode only) is all 96 bits of the unique
// device ID, 24 hex digits: ST guarantees only all of them together to be
// unique. The generated one(usbd_desc.c) folds them into 12 digits and
// drops the wafer number - chips from the same position on different wafers
// of a lot get the same serial, and host software matching the pendant by
// serial would mix them up. Create() installs it in the descriptor table
// FS_Desc, before USB is started. Manufacturer and product strings are set
// in CubeMX.
//
// Objects of this class are created only when the USB interface is selected,
// on the heap: static RAM is full, and a UART user shouldn't pay for it.
//
//******************************************************************************

#ifndef UsbCdcUart_h
#define UsbCdcUart_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCore.h"

// *****************************************************************************
// ***   Called from usb_device.c(USER CODE) after MX_USB_DEVICE_Init()   ******
// *****************************************************************************
extern "C" void UsbCdcUart_RegisterInterface(void);

// *****************************************************************************
// ***   UsbCdcUart class   ****************************************************
// *****************************************************************************
class UsbCdcUart : public IUart
{
  public:
    // *************************************************************************
    // ***   Public: Create   **************************************************
    // *************************************************************************
    // * Creates the only object on the heap, nullptr if there is no memory.
    // * Must be called before the scheduler starts, and only once: the
    // * default task calls UsbCdcUart_RegisterInterface() when it starts USB.
    static UsbCdcUart* Create();

    // *************************************************************************
    // ***   Public: BuildSerialDescriptor   ***********************************
    // *************************************************************************
    // * Serial number string descriptor from the three words of the unique ID:
    // * 24 hex digits, word 0 first, each word most significant digit first.
    // * desc must hold SERIAL_DESC_SIZE bytes. Returns its length.
    static constexpr uint32_t SERIAL_DESC_SIZE = 2u + 24u * 2u;
    static uint16_t BuildSerialDescriptor(uint32_t word0, uint32_t word1, uint32_t word2, uint8_t* desc);

    // *************************************************************************
    // ***   Public: GetInstance   *********************************************
    // *************************************************************************
    // * The object, nullptr if the USB interface isn't in use.
    static inline UsbCdcUart* GetInstance() {return instance;}

    // *************************************************************************
    // ***   Public: Init   ****************************************************
    // *************************************************************************
    // * The USB device is started by MX_USB_DEVICE_Init(), nothing to do here.
    virtual inline Result Init() {return Result::RESULT_OK;}

    // *************************************************************************
    // ***   Public: SetBaudRate   *********************************************
    // *************************************************************************
    // * A virtual COM port has no baud rate, the host's line coding is ignored.
    virtual inline Result SetBaudRate(uint32_t baud_rate) {return Result::RESULT_OK;}

    // *************************************************************************
    // ***   Public: Read   ****************************************************
    // *************************************************************************
    virtual Result Read(uint8_t& rx_byte);

    // *************************************************************************
    // ***   Public: Read   ****************************************************
    // *************************************************************************
    virtual Result Read(uint8_t* rx_buf_ptr, uint32_t& size);

    // *************************************************************************
    // ***   Public: Write   ***************************************************
    // *************************************************************************
    virtual inline Result Write(uint8_t tx_byte) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Write   ***************************************************
    // *************************************************************************
    // * Starts a transfer from tx_buf_ptr, which must stay valid until
    // * IsTxComplete() returns true. ERR_UART_BUSY while one is pending.
    virtual Result Write(uint8_t* tx_buf_ptr, uint32_t size);

    // *************************************************************************
    // ***   Public: IsTxComplete   ********************************************
    // *************************************************************************
    // * False while a transfer is pending.
    virtual bool IsTxComplete(void);

    // *************************************************************************
    // ***   Public: IsLinkOpen   **********************************************
    // *************************************************************************
    // * Device configured and port opened by the host(DTR set).
    bool IsLinkOpen(void);

    // *************************************************************************
    // ***   Public: CDC interface callbacks, in the USB interrupt   ***********
    // *************************************************************************
    // Called only by the class driver, through the table installed by
    // UsbCdcUart_RegisterInterface().
    // Never installed without the object.
    static int8_t CdcInit(void);
    static int8_t CdcDeInit(void);
    static int8_t CdcControl(uint8_t cmd, uint8_t* pbuf, uint16_t length);
    static int8_t CdcReceive(uint8_t* buf, uint32_t* len);

  private:
    // Size of the receive ring buffer, power of two. Status reports are about
    // 100 bytes, and GrblComm empties the ring every millisecond.
    static constexpr uint32_t RX_BUF_SIZE = 256u;
    // Full speed bulk packet size(CDC_DATA_FS_MAX_PACKET_SIZE). The OUT
    // endpoint is armed only while the ring has room for one.
    static constexpr uint32_t PACKET_SIZE = 64u;
    // A pending transfer the host doesn't take within this time is aborted.
    // An open port reads a packet within a millisecond; this is a host that
    // keeps DTR after closing(stty -hupcl) or stopped reading.
    static constexpr uint32_t TX_TIMEOUT_MS = 250u;

    static_assert((RX_BUF_SIZE & (RX_BUF_SIZE - 1u)) == 0u, "RX_BUF_SIZE must be a power of two");
    static_assert(RX_BUF_SIZE >= 2u * PACKET_SIZE, "RX_BUF_SIZE must fit at least two packets");

    // Receive ring buffer. Volatile, like the indexes: the interrupt writes
    // it, and the compiler must not move the read of a byte in Read() after
    // the rx_tail update that gives its place back to the interrupt.
    volatile uint8_t rx_buf[RX_BUF_SIZE];
    // Buffer the OUT endpoint receives a packet into
    uint8_t rx_packet[PACKET_SIZE];
    // Free running indexes: head is written only by the interrupt, tail only
    // by the reader, so the ring needs no lock
    volatile uint32_t rx_head = 0u;
    volatile uint32_t rx_tail = 0u;
    // OUT endpoint left unarmed because the ring had no room for a packet
    volatile bool rx_paused = false;
    // DTR from the last SET_CONTROL_LINE_STATE: host has the port open
    volatile bool dtr = false;
    // Time the pending transfer started
    uint32_t tx_timestamp = 0u;

    // The only object
    static UsbCdcUart* instance;

    // *************************************************************************
    // ***   Private: OnReceive   **********************************************
    // *************************************************************************
    // * Data received. True if the OUT endpoint should be armed again.
    bool OnReceive(const uint8_t* buf, uint32_t len);

    // *************************************************************************
    // ***   Private: IsTxPending   ********************************************
    // *************************************************************************
    // * True while a transfer is pending; aborts a stale one first.
    bool IsTxPending(void);

    // *************************************************************************
    // ***   Private: ResumeRx   ***********************************************
    // *************************************************************************
    void ResumeRx(void);

    // *************************************************************************
    // ***   Private: Constructor   ********************************************
    // *************************************************************************
    UsbCdcUart() {};

    // *************************************************************************
    // ***   Private: Constructors and assign operator - prevent copying   *****
    // *************************************************************************
    UsbCdcUart(const UsbCdcUart&);
    UsbCdcUart& operator=(const UsbCdcUart);
};

#endif
