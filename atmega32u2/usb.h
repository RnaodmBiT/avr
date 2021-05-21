#ifndef _USB_H_
#define _USB_H_

#include "system.h"
#include "target.h"

#if defined(__cplusplus)
extern "C" {
#endif


#if !defined(USB_PACKET_SIZE)
#error USB_PACKET_SIZE is not defined.
#endif

#define USB_LEN_DEVICE      18
#define USB_LEN_CONFIG      9
#define USB_LEN_INTERFACE   9
#define USB_LEN_ENDPOINT    7

#define USB_POWER_MA(ma)    ((ma) / 2)


typedef bool (*usb_callback_t)(void*);


typedef struct {
    uint8_t length;
    uint8_t descriptor_type;
    uint8_t address;
    uint8_t attributes;
    uint16_t max_packet_size;
    uint8_t interval;
} usb_endpoint_t;


typedef enum {
    USB_DESC_DEVICE = 1,
    USB_DESC_CONFIG,
    USB_DESC_STRING,
    USB_DESC_INTERFACE,
    USB_DESC_ENDPOINT
} usb_desc_t;


typedef enum {
    USB_CONTROL = 0x00,
    USB_ISOCHRONOUS = 0x40,
    USB_BULK = 0x80,
    USB_INTERRUPT = 0xC0
} usb_type_t;


typedef enum {
    USB_DESC_CONTROL,
    USB_DESC_ISOCHRONOUS,
    USB_DESC_BULK,
    USB_DESC_INTERRUPT
} usb_desc_type_t;


typedef enum {
    USB_OUT,
    USB_IN
} usb_dir_t;


typedef enum {
    USB_DESC_OUT,
    USB_DESC_IN = 0x80,
} usb_desc_dir_t;


typedef struct __attribute__((packed)) {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} usb_setup_t;


// Initialize the USB driver.
// Config is a pointer to the entire block of configuration data.
// Callbacks is a list of the endpoint callbacks.
void usb_init(const char* config, usb_callback_t* callbacks);


inline void usb_putc(char c) {
    UEDATX = c;
}


inline uint8_t usb_getc(void) {
    return UEDATX;
}


inline void usb_puts(const void* s, uint8_t len) {
    const char* str = (const char*)s;
    while (len--) {
        usb_putc(*str++);
    }
}


inline void usb_gets(void* s, uint8_t len) {
    char* str = (char*)s;
    while (len--) {
        *str++ = usb_getc();
    }
}


inline bool usb_in_ready(void) {
    return UEINTX & BIT(TXINI);
}


inline bool usb_out_ready(void) {
    return UEINTX & BIT(RXOUTI);
}


inline void usb_wait_in(void) {
    while (!usb_in_ready()) ;
}


inline void usb_wait_out(void) {
    while (!usb_out_ready()) ;
}


inline void usb_clear_in(void) {
    UEINTX &= ~(BIT(TXINI) | BIT(FIFOCON));
}


inline void usb_clear_out(void) {
    UEINTX &= ~(BIT(RXOUTI) | BIT(FIFOCON));
}


inline void usb_clear_setup(void) {
    UEINTX &= ~BIT(RXSTPI);
}


inline bool usb_fifo_available(void) {
    return UEINTX & BIT(RWAL);
}


inline void usb_wait_fifo(void) {
    while (!usb_fifo_available()) ;
}


inline void usb_ep_set(uint8_t ep) {
    UENUM = ep;
}


inline void usb_write(const void* buf, uint16_t len) {
    do {
        usb_wait_in();

        uint16_t chunk = MIN(len, USB_PACKET_SIZE);

        usb_puts(buf, chunk);

        len -= chunk;
        buf += chunk;

        usb_clear_in();
    } while (len);
}


inline void usb_read(void* buf, uint16_t len) {
    uint8_t* data = (uint8_t*)buf;
    while (len) {
        usb_wait_out();

        while (len && UEBCLX) {
            *data++ = usb_getc();
            len--;
        }

        usb_clear_out();
    }
}


inline void usb_gobble(uint16_t len) {
    while (len) {
        usb_wait_out();

        while (len && UEBCLX) {
            usb_getc();
            len--;
        }

        usb_clear_out();
    }
}



inline uint16_t usb_bytes_in_endpoint(void) {
    return UEBCLX;// | ((uint16_t)UEBCHX << 8);
}


inline void usb_enable_in_irq(void) {
    UEIENX |= BIT(TXINE);
}


inline void usb_disable_in_irq(void) {
    UEIENX &= ~BIT(TXINE);
}



// TODO: This is a hack for the 32u2
inline bool usb_vbus_connected(void) {
    return true;
}


#if defined(__cplusplus)
}
#endif

#endif // _USB_H_
