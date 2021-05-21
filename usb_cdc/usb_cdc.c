#include "usb_cdc.h"
#include <ring.h>
#include <usb.h>
#include "target.h"


// Ensure a valid buffer size is given
#if !defined(USB_CDC_BUFFER_SIZE)
#define USB_CDC_BUFFER_SIZE 96
#endif


#define CDC_SET_LINE_CODING             0x20
#define CDC_GET_LINE_CODING             0x21
#define CDC_SET_CONTROL_LINE_STATE      0x22

static ring_t read_buffer;
static ring_t write_buffer;

static bool is_connected = false;


// Prototype the IRQ handlers
bool cdc_control_handler(void*);
bool cdc_bulk_out_handler(void*);
bool cdc_bulk_in_handler(void*);


static const char config_descriptor[] = {
    USB_LEN_CONFIG,                 // bLength
    USB_DESC_CONFIG,                // bDescriptorType
    0x43, 0,                        // wTotalLength
    2,                              // bNumInterfaces
    1,                              // bConfigurationValue
    0,                              // iConfiguration
    0xC0,                           // bmAttributes
    USB_POWER_MA(100),              // bMaxPower

    // Interface descriptor
    USB_LEN_INTERFACE,              // bLength
    USB_DESC_INTERFACE,             // bDescriptorType
    0,                              // bInterfaceNumber
    0,                              // bAlternateSetting
    1,                              // bNumEndpoints
    0x02,                           // bInterfaceClass
    0x02,                           // bInterfaceSubClass
    0,                              // bInterfaceProtocol
    0,                              // iInterface

    // Header functional descriptor
    0x05,                           // bFunctionLength
    0x24,                           // bDescriptor Type: CS_INTERFACE
    0x00,                           // bDescriptor SubType: Header Func Desc
    0x10,                           // bcdCDC:1.1
    0x01,

    // ACM functional descriptor
    0x04,                           // bFunctionLength
    0x24,                           // bDescriptor Type: CS_INTERFACE
    0x02,                           // bDescriptor SubType: ACM Func desc
    0x00,                           // bmCapabilities

    // Union functional descriptor
    0x05,                           // bFunctionLength
    0x24,                           // bDescriptor Type: CS_INTERFACE
    0x06,                           // bDescriptor SybType: Union Func desc
    0x00,                           // bMasterInterface: Communication Class Interface
    0x01,                           // bSlaveInterface0: Data class interface

    // Call management functional descriptor
    0x05,                           // bFunctionLength
    0x24,                           // bDescriptor Type: CS_INTERFACE
    0x01,                           // bDescriptor SybType: Call management desc
    0x00,                           // bmCapabilites: D1 + D0
    0x01,                           // bSlaveInterface0: Data class interface 1

    // Enpoint 3 descriptor
    USB_LEN_ENDPOINT,               // bEndpointLength
    USB_DESC_ENDPOINT,              // bDescriptorType
    3 | USB_DESC_IN,                // bAddress
    USB_DESC_INTERRUPT,             // bmAttributes
    0x08, 0,                        // wMaxPacketSize
    0xFF,                           // bInterval

    // Data class interface descriptor INT IN
    USB_LEN_INTERFACE,              // bLength
    USB_DESC_INTERFACE,             // bDescriptorType
    1,                              // bInterfaceNumber
    0,                              // bAlternateSetting
    2,                              // bNumEndpoints
    0x0A,                           // bInterfaceClass
    0x00,                           // bInterfaceSubClass
    0x00,                           // bInterfaceProtocol
    0,                              // iInterface

    // Enpoint 1 descriptor BULK OUT
    USB_LEN_ENDPOINT,               // bEndpointLength
    USB_DESC_ENDPOINT,              // bDescriptorType
    1 | USB_DESC_OUT,               // bAddress
    USB_DESC_BULK,                  // bmAttributes
    USB_PACKET_SIZE, 0,             // wMaxPacketSize
    0x00,                           // bInterval

    // Enpoint 2 descriptor BULK IN
    USB_LEN_ENDPOINT,               // bEndpointLength
    USB_DESC_ENDPOINT,              // bDescriptorType
    2 | USB_DESC_IN,                // bAddress
    USB_DESC_BULK,                  // bmAttributes
    USB_PACKET_SIZE, 0,             // wMaxPacketSize
    0x00,                           // bInterval

    0, 0
};


static usb_callback_t endpoints[] = {
    cdc_control_handler,
    cdc_bulk_out_handler,
    cdc_bulk_in_handler,
    NULL
};


// Setup the USB CDC driver
void usb_cdc_init(void) {
    // Initialize the storage buffer
    ring_init(&read_buffer, USB_CDC_BUFFER_SIZE);
    ring_init(&write_buffer, USB_CDC_BUFFER_SIZE);

    usb_init(config_descriptor, endpoints);
}


// A stream writing function used for setting up printf
int usb_cdc_write_char(char c, FILE* stream) {
    usb_cdc_putc(c);
    return 0;
}


int usb_cdc_read_char(FILE* stream) {
    if (usb_cdc_has_data()) {
        return usb_cdc_getc();
    } else {
        return EOF;
    }
}


// Initialize the USB CDC to receive data from stdout
void usb_cdc_init_printf(void) {
    stdout = usb_cdc_init_stream();
}


FILE* usb_cdc_init_stream(void) {
    static FILE usb_cdc_stream = FDEV_SETUP_STREAM(usb_cdc_write_char,
                                                   usb_cdc_read_char,
                                                   _FDEV_SETUP_RW);
    return &usb_cdc_stream;
}


bool usb_cdc_can_write(void) {
    return !ring_is_full(&write_buffer);
}


// Write a character to the driver
void usb_cdc_putc(char c) {
    if (!usb_vbus_connected()) {
        return;
    }

    if (!is_connected) {
        return;
    }

    // NOTE: if no data is being read out then this overwrites the old data
    if (c == '\n') {
        ring_putc(&write_buffer, '\r');
    }
    ring_putc(&write_buffer, c);

    // enable interrupts on the IN packets to let the data out
    usb_ep_set(2);
    usb_enable_in_irq();
}


// Write a string to the driver
void usb_cdc_puts(const char* s) {
    while (*s) {
        char c = *s++;
        usb_cdc_putc(c);
    }
}


// Read a character from the driver
char usb_cdc_getc(void) {
    return ring_getc(&read_buffer);
}


// Check if there is any data in the read buffer
bool usb_cdc_has_data(void) {
    return !ring_is_empty(&read_buffer);
}


// Clear the input buffer
void usb_cdc_clear(void) {
    ring_clear(&read_buffer);
}


// Handle any control messages coming our way
bool cdc_control_handler(void* ptr) {
    usb_setup_t* setup = (usb_setup_t*)ptr;

    /*
     * In this handler we will only report our line coding. Any attempt to
     * change it will fail. So lets just statically declare a fast baud rate and
     * be done with it.
     */
    typedef struct {
        uint32_t baud_rate;
        uint8_t format;
        uint8_t parity;
        uint8_t bits;
    } cdc_line_coding;

    static const cdc_line_coding line_coding = {
        115200,
        0,
        0,
        8
    };

    switch (setup->request) {
        case CDC_SET_LINE_CODING:
            usb_gobble(setup->length); // read the OUT data (IMPORTANT)
            usb_clear_in(); // ZLP ack
            break;

        case CDC_GET_LINE_CODING:
            usb_write(&line_coding, sizeof(line_coding));
            usb_clear_out();
            break;

        case CDC_SET_CONTROL_LINE_STATE:
            // NOTE: this one doesnt send any OUT data
            usb_clear_in();

            // See SiLabs app note AN758 for details
            // This indicates if a terminal is currently listening
            is_connected = setup->value & BIT(0);
            break;

        default:
            return false; // let the standard handler have it
    }

    return true;
}


bool cdc_bulk_out_handler(void* p) {
    // Read all of the data into the static buffer
    while (usb_fifo_available()) {
        ring_putc(&read_buffer, usb_getc());
    }

    usb_clear_out();
    return true;
}


bool cdc_bulk_in_handler(void* p) {
    // write a packet of data out
    while (!ring_is_empty(&write_buffer) && usb_fifo_available()) {
        usb_putc(ring_getc(&write_buffer));
    }

    usb_clear_in(); // send the data we have

    if (ring_is_empty(&write_buffer)) {
        // if are out of data, no more IRQs until putc.
        usb_disable_in_irq();
    }
    return true;
}


bool usb_cdc_is_connected(void) {
    return is_connected;
}
