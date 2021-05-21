#include "usb.h"
#include "pio.h"

#if !defined(USB_PACKET_SIZE)
#error USB_PACKET_SIZE is not defined.
#endif

#if !defined(USB_VENDOR_ID)
#error USB_VENDOR_ID is not defined.
#endif

#if !defined(USB_PRODUCT_ID)
#error USB_PRODUCT_ID is not defined.
#endif

#if !defined(USB_DEVICE_VERSION)
#error USB_DEVICE_VERSION is not defined.
#endif

#if !defined(USB_PRODUCT)
#error USB_PRODUCT is not defined.
#endif

#if !defined(USB_MANUFACTURER)
#error USB_MANUFACTURER is not defined.
#endif

#if !defined(USB_SERIAL)
#error USB_SERIAL is not defined.
#endif

#if !defined(USB_CLASS_SUBCLASS)
#error USB_CLASS_SUBCLASS is not defined.
#endif


typedef enum {
    USB_GET_STATUS = 0,
    USB_CLEAR_FEATURE = 1,
    USB_SET_FEATURE = 3,
    USB_SET_ADDRESS = 5,
    USB_GET_DESCRIPTOR = 6,
    USB_GET_CONFIGURATION = 8,
    USB_SET_CONFIGURATION = 9,
    USB_GET_INTERFACE = 10,
    USB_SET_INTERFACE = 11
} usb_std_msg_t;

#define MAX_NUM_ENDPOINTS       7

#define USB_EP_SIZE(x)      (x == 8 ?  0x00 : \
                             x == 16 ? 0x10 : \
                             x == 32 ? 0x20 : 0x30)



// Keep a list of the endpoint callbacks
static const usb_callback_t* usb_callbacks = NULL;

// Keep a reference to the configuration data
static const char* usb_config = NULL;

// Define the basic device properties
static const char usb_device[] = {
    0x12,   // bLength
    0x01,   // bDescriptorType
    0x02,   // bcdUSB
    0x00,
    USB_CLASS_SUBCLASS,   // bDeviceClass and bDeviceSubClass
    0x00,   // bDeviceProtocol
    USB_PACKET_SIZE,    // bMaxPacketSize0
    LO(USB_VENDOR_ID),  // idVendor
    HI(USB_VENDOR_ID),
    LO(USB_PRODUCT_ID), // idProduct
    HI(USB_PRODUCT_ID),
    HI(USB_DEVICE_VERSION), // bcdDevice
    LO(USB_DEVICE_VERSION),
    1,      // iManufacturer
    2,      // iProduct
    3,      // iSerialNumber,
    1       // bNumConfigurations
};


// String descriptor
typedef struct {
    uint8_t length;
    uint8_t descriptor_type;
    wchar_t string[];
} usb_string_t;

#define USB_STRING(str) { sizeof(str) + 2, 3, str }
#define USB_ENGLISH() { 4, 3, { 0x09, 0x04 }}


// Define the USB strings
static const usb_string_t language = USB_ENGLISH();
static const usb_string_t manufacturer = USB_STRING(USB_MANUFACTURER);
static const usb_string_t product = USB_STRING(USB_PRODUCT);
static const usb_string_t serial = USB_STRING(USB_SERIAL);

static const usb_string_t* string_table[] = {
    &language,
    &manufacturer,
    &product,
    &serial
};


// Check if there is a reset event to be handler
inline bool usb_reset_ready(void) {
    return UDINT & BIT(EORSTI);
}


// Clear the reset event
inline void usb_clear_reset(void) {
    UDINT = 0;
}



// Attach to the bus
inline void usb_attach(void) {
    UDCON &= ~BIT(DETACH);
}


// Detach from the bus
inline void usb_detach(void) {
    UDCON |= BIT(DETACH);
}




// Check if the specified endpoint has an interrupt ready
inline bool usb_ep_has_interrupt(uint8_t ep) {
    return UEINT & BIT(ep);
}


// Set the address for the device
inline void usb_set_address(uint8_t address) {
    UDADDR = address | BIT(ADDEN);
}


// Configur the given endpoint with the given settings
void usb_setup_endpoint(uint8_t id,
                        usb_type_t type,
                        usb_dir_t dir) {
    usb_ep_set(id);

    // Enable the endpoint
    UECONX = BIT(EPEN);
    // Set the mode and direction
    UECFG0X = type | dir;
    // Set the packet size and allocate the buffers
    UECFG1X = USB_EP_SIZE(USB_PACKET_SIZE) | BIT(ALLOC);

    // Enable the interrupts
    if (id == 0) {
        UEIENX = BIT(RXSTPE);
    } else if (dir == USB_IN && type == USB_BULK) {
        // UEIENX = BIT(TXINE);
    } else {
        UEIENX = BIT(RXOUTE);
    }

    if ((UESTA0X & BIT(CFGOK)) == 0) {
#if defined(LED_ERROR)
        pio_output_low(LED_ERROR);
#endif
        while (1) ;
    }
}


// Handle a USB reset event,
// All of the endpoint needs to be reinitialized. Sigh.
void usb_handle_reset(void) {
    // Start with the default control endpoint
    usb_setup_endpoint(0, USB_CONTROL, 0);

    // Traverse the entries in the table looking for end point descriptors
    const char* entry = usb_config;
    const usb_endpoint_t* ep = (const usb_endpoint_t*)entry;
    do {
        if (ep->descriptor_type == USB_DESC_ENDPOINT) {
            uint8_t id = ep->address & 0x7F;
            usb_dir_t dir = (ep->address & 0x80) ? USB_IN : USB_OUT;
            usb_type_t type = 0;
            switch (ep->attributes) {
                case USB_DESC_CONTROL: type = USB_CONTROL; break;
                case USB_DESC_BULK: type = USB_BULK; break;
                case USB_DESC_INTERRUPT: type = USB_INTERRUPT; break;
                case USB_DESC_ISOCHRONOUS: type = USB_ISOCHRONOUS; break;
            }

            usb_setup_endpoint(id, type, dir);
        }

        entry += ep->length;
        ep = (const usb_endpoint_t*)entry;

    } while (ep->length != 0 && ep->descriptor_type != 0) ;
}


// Send the host one of our descriptors
void usb_get_descriptor(usb_setup_t* setup) {
    const usb_string_t* string;

    uint8_t type = HI(setup->value);
    uint8_t index = LO(setup->value);

    switch (type) {
        case USB_DESC_DEVICE:
            usb_write(usb_device, sizeof(usb_device));
            break;

        case USB_DESC_CONFIG:
            usb_write(usb_config, setup->length);
            break;

        case USB_DESC_STRING:
            string = string_table[index];
            usb_write(string, string->length);

        default:
            usb_clear_in();
    }
}


// Handle the standard control endpoint requests
void usb_standard_callback(usb_setup_t* setup) {
    switch (setup->request) {
        case USB_GET_DESCRIPTOR:
            usb_get_descriptor(setup);
            break;

        case USB_SET_CONFIGURATION:
            usb_clear_in();
            break;

        case USB_GET_CONFIGURATION:
            usb_putc(1);
            usb_clear_in();

        case USB_SET_ADDRESS:
            usb_clear_in();
            usb_wait_in();
            usb_set_address(setup->value);
            break;

        case USB_GET_STATUS:
            usb_putc(0);
            usb_putc(0);
            usb_clear_in();
            break;
    }
}


// Handle an endpoint event for the given endpoint.
void usb_ep_handle(uint8_t ep) {
    if (ep == 0) {
        // It's the default control endpoint. Check if the protocol handler
        // should deal with it, otherwise we'll process it
        usb_setup_t setup;
        usb_gets(&setup, sizeof(setup));
        usb_clear_setup();

        if (!usb_callbacks[ep] || !usb_callbacks[ep](&setup)) {
            usb_standard_callback(&setup);
        }
    } else {
        // It's just an endpoint callback, treat it normally
        if (usb_callbacks[ep]) {
            usb_callbacks[ep](NULL);
        }
    }
}


// General USB interrupt vector.
// This has two sources:
// The end-of-reset event.
// And the VBUS state change event.
ISR(USB_GEN_vect) {
    if (usb_reset_ready()) {
        usb_clear_reset();
        usb_handle_reset();
    }
}


// USB Endpoint interrupt vector.
// This has one source:
// The RXOUT interrupt for an OUT endpoint,
ISR(USB_COM_vect) {
    uint8_t ep = UENUM;

    for (uint8_t i = 0; i < MAX_NUM_ENDPOINTS; ++i) {
        while (usb_ep_has_interrupt(i)) {
            usb_ep_set(i);
            usb_ep_handle(i);
        }
    }

    usb_ep_set(ep); // Reset this back to what the original was
}


// Initialize the USB peripheral
void usb_init(const char* config, usb_callback_t* callbacks) {
    // Store the protocol data
    usb_config = config;
    usb_callbacks = callbacks;

    // Enable the USB controller and freeze the clock
    USBCON = BIT(USBE) | BIT(FRZCLK);
    // Enable the internal regulator
    //UHWCON = BIT(UVREGE);
    // Enable the end-of-reset interrupt
    UDIEN = BIT(EORSTE);

    // Setup the PLL input
    if (F_CPU == 16000000ull) {
        // For some reason PINDIV (2) isn't defined
        PLLCSR = BIT(PLLE) | BIT(2); // 16 MHz needs to be divided
    } else {
        PLLCSR = BIT(PLLE);
    }

    // Wait for the PLL to lock
    while (!(PLLCSR & BIT(PLOCK)));

    // Unfreeze the clock to start operations
    USBCON &= ~BIT(FRZCLK);

    // If we are already connected then lets get to it!
    //if (usb_vbus_connected()) {
    // TODO: How to handle bus connect/disconnect?
    usb_attach();
    //}
}
