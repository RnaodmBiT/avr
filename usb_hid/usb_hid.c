#include "usb_hid.h"
#include <usb.h>
#include "target.h"

#define HID_GET_DESCRIPTOR  0x06
#define HID_SET_DESCRIPTOR  0x07
#define HID_GET_REPORT      0x01
#define HID_GET_IDLE        0x02
#define HID_GET_PROTOCOL    0x03
#define HID_SET_REPORT      0x09
#define HID_SET_IDLE        0x0A
#define HID_SET_PROTOCOL    0x0B

// Prototype the IRQ handlers
bool hid_control_handler(void*);
bool hid_interrupt_in(void*);

static const char config_descriptor_mouse[] = {
    USB_LEN_CONFIG,                 // bLength
    USB_DESC_CONFIG,                // bDescriptorType
    0x22, 0,                        // wTotalLength
    1,                              // bNumInterfaces
    1,                              // bConfigurationValue
    0,                              // iConfiguration
    0xC0,                           // bmAttributes
    USB_POWER_MA(500),              // bMaxPower

    // Interface descriptor
    USB_LEN_INTERFACE,              // bLength
    USB_DESC_INTERFACE,             // bDescriptorType
    0,                              // bInterfaceNumber
    0,                              // bAlternateSetting
    1,                              // bNumEndpoints
    0x03,                           // bInterfaceClass
    0x00,                           // bInterfaceSubClass
    0,                              // bInterfaceProtocol
    0,                              // iInterface


    // HID descriptor
    9,                              // bLength
    0x21,                           // bDescriptorType
    0x01, 0x01,                     // bcdHID
    0,                              // bCountryCode
    1,                              // bNumDescriptors
    0x22,                           // bDescriptorType0
    0x32, 0x00,                     // wDescriptorLength0


    // Endpoint descriptor
    USB_LEN_ENDPOINT,               // bLength
    USB_DESC_ENDPOINT,              // bDescriptorType
    1 | USB_DESC_IN,                // bAddress
    USB_DESC_INTERRUPT,             // bmAttributes
    0x08, 0,                        // wMaxPacketSize
    0x0A,                           // bInterval

    0, 0
};

static const char mouse_report_descriptor[] = {
    // Report descriptor
    0x05, 0x01,                     // Usage Page (Generic Desktop)
    0x09, 0x02,                     // Usage (Mouse)
    0xA1, 0x01,                     // Collection (Application)
    0x09, 0x01,                     // Usage (Pointer)
    0xA1, 0x00,                     // Collection (Physical)

    0x05, 0x09,                     // Usage Page (Buttons)
    0x19, 0x01,                     // Usage Minimum (1)
    0x29, 0x03,                     // Usage Maximum (3)
    0x15, 0x00,                     // Logical Minimum (0)
    0x25, 0x01,                     // Logical Maximum (1)
    0x95, 0x03,                     // Report Count (3)
    0x75, 0x01,                     // Report Size (1)
    0x81, 0x02,                     // Input (data, variable, absolute)

    0x95, 0x01,                     // Report Count (1)
    0x75, 0x05,                     // Report Size (5)
    0x81, 0x01,                     // Input (constant)

    0x05, 0x01,                     // Usage Page (Generic Desktop)
    0x15, 0x81,                     // Logical Minimum (-127)
    0x25, 0x7F,                     // Logical Maximum (127)
    0x75, 0x08,                     // Report Size (8)
    0x95, 0x02,                     // Report Count (2)
    0x09, 0x30,                     // Usage (X)
    0x09, 0x31,                     // Usage (Y)
    0x81, 0x06,                     // Input (Data, Variable, Relative)

    0xC0,                           // End Collection
    0xC0,                           // End Collection
};


static const char config_descriptor_keyboard[] = {
    USB_LEN_CONFIG,                 // bLength
    USB_DESC_CONFIG,                // bDescriptorType
    0x22, 0,                        // wTotalLength
    1,                              // bNumInterfaces
    1,                              // bConfigurationValue
    0,                              // iConfiguration
    0xC0,                           // bmAttributes
    USB_POWER_MA(500),              // bMaxPower

    // Interface descriptor
    USB_LEN_INTERFACE,              // bLength
    USB_DESC_INTERFACE,             // bDescriptorType
    0,                              // bInterfaceNumber
    0,                              // bAlternateSetting
    1,                              // bNumEndpoints
    0x03,                           // bInterfaceClass
    0x00,                           // bInterfaceSubClass
    0,                              // bInterfaceProtocol
    0,                              // iInterface


    // HID descriptor
    9,                              // bLength
    0x21,                           // bDescriptorType
    0x01, 0x01,                     // bcdHID
    0,                              // bCountryCode
    1,                              // bNumDescriptors
    0x22,                           // bDescriptorType0
    0x27, 0x00,                     // wDescriptorLength0


    // Endpoint descriptor
    USB_LEN_ENDPOINT,               // bLength
    USB_DESC_ENDPOINT,              // bDescriptorType
    1 | USB_DESC_IN,                // bAddress
    USB_DESC_INTERRUPT,             // bmAttributes
    0x08, 0,                        // wMaxPacketSize
    0x0A,                           // bInterval

    0, 0
};

static const char keyboard_report_descriptor[] = {
    // Report descriptor
    0x05, 0x01,                     // Usage Page (Generic Desktop)
    0x09, 0x06,                     // Usage (Mouse)
    0xA1, 0x01,                     // Collection (Application)

    // Modifier keys
    0x05, 0x07,                     // Usage Page (Key Codes)
    0x19, 0xE0,                     // Usage Minimum (224)
    0x29, 0xE7,                     // Usage Maximum (231)
    0x15, 0x00,                     // Logical Minimum (0)
    0x25, 0x01,                     // Logical Maximum (1)
    0x95, 0x08,                     // Report Count (8)
    0x75, 0x01,                     // Report Size (1)
    0x81, 0x02,                     // Input (data, variable, absolute)


    0x05, 0x07,                     // Usage Page (Key Codes)
    0x15, 0x00,                     // Logical Minimum (0)
    0x25, 0x65,                     // Logical Maximum (101)
    0x19, 0x00,                     // Usage Minimum (0)
    0x29, 0x65,                     // Usage Maximum (101)
    0x75, 0x08,                     // Report Size (8)
    0x95, 0x06,                     // Report Count (6)
    0x81, 0x00,                     // Input (Data, Array)

    0xC0,                           // End Collection
};



static usb_callback_t endpoints[] = {
    hid_control_handler,
    NULL
};



// static bool state_changed = true;
static bool active_reporting = true;
static const char* hid_report = NULL;



void usb_hid_init_generic(const char* usb_descriptor, const char* hid_descriptor) {
    hid_report = hid_descriptor;
    usb_init(usb_descriptor, endpoints);
}


void usb_hid_write_generic(const void* data, uint8_t len) {
    usb_ep_set(1);
    usb_wait_in();
    usb_puts(data, len);
    usb_clear_in();
}




// Setup the USB HID driver
void usb_hid_init_mouse(void) {
    hid_report = mouse_report_descriptor;
    usb_init(config_descriptor_mouse, endpoints);
}


void usb_hid_move_mouse(int8_t dx, int8_t dy) {
    usb_ep_set(1);
    usb_wait_in();
    usb_putc(0); // no buttons pressed
    usb_putc(dx); // x movement
    usb_putc(dy); // y movement
    usb_clear_in();
}


void usb_hid_init_keyboard(void) {
    hid_report = keyboard_report_descriptor;
    usb_init(config_descriptor_keyboard, endpoints);
}


void usb_hid_press_key(uint8_t key) {
    usb_hid_press_key_e(key, 0);
}

void usb_hid_press_key_e(uint8_t key, uint8_t key2) {
    usb_ep_set(1);

    usb_wait_in();
    usb_putc(0);        // modifier
    usb_putc(key);      // key 0
    usb_putc(key2);     // key 1
    usb_putc(0);        // key 2
    usb_putc(0);        // key 3
    usb_putc(0);        // key 4
    usb_putc(0);        // key 5
    usb_clear_in();

    usb_wait_in();
    usb_putc(0);        // modifier
    usb_putc(0);        // key 0
    usb_putc(0);        // key 1
    usb_putc(0);        // key 2
    usb_putc(0);        // key 3
    usb_putc(0);        // key 4
    usb_putc(0);        // key 5
    usb_clear_in();
}



// Handle any control messages coming our way
bool hid_control_handler(void* ptr) {
    usb_setup_t* setup = (usb_setup_t*)ptr;

    if (setup->request_type == 0xA1 && setup->request == HID_GET_REPORT) {
        usb_putc(0); // no buttons pressed
        usb_putc(0); // x movement
        usb_putc(0); // y movement
        usb_clear_in();
        return true;
    }

    if (setup->request_type == 0x21 && setup->request == HID_SET_REPORT) {
        usb_clear_out();
        return true;
    }


    if (setup->request_type == 0xA1 && setup->request == HID_GET_IDLE) {
        usb_putc(active_reporting);
        usb_clear_in();
        return true;
    }


    if (setup->request_type == 0x21 && setup->request == HID_SET_IDLE) {
        if (HI(setup->value) == 0) {
            active_reporting = false;
        }
        usb_clear_in();
        return true;
    }

    if (setup->request_type == 0xA1 && setup->request == HID_GET_PROTOCOL) {
        usb_putc(1); // report protocol
        usb_clear_in();
        return true;
    }

    if (setup->request_type == 0x21 && setup->request == HID_SET_PROTOCOL) {
        usb_clear_out();
        return true;
    }

    if (setup->request_type == 0x81 && setup->request == HID_GET_DESCRIPTOR) {
        switch (HI(setup->value)) {
            case 0x22: // report descriptor
                usb_write(hid_report, setup->length);
                return true;
        }
        return false;
    }

    if (setup->request_type == 0x01 && setup->request == HID_SET_DESCRIPTOR) {
        usb_clear_out();
        return true;
    }

    return false;
}
