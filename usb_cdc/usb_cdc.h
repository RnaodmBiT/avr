#ifndef _USB_CDC_H_
#define _USB_CDC_H_

#include <system.h>
#include <stdio.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif


#define USB_CDC_CLASS_SUBCLASS          0x02, 0x02


// Initialize the USB CDC driver.
void usb_cdc_init(void);


// Setup the USB CDC driver to receive printfs
void usb_cdc_init_printf(void);


FILE* usb_cdc_init_stream(void);


// Check if the cdc buffer is avaiable to write to
bool usb_cdc_can_write(void);


// Write a character to the driver
void usb_cdc_putc(char c);


// Write a string to the driver
void usb_cdc_puts(const char* s);


// Read a character from the driver
char usb_cdc_getc(void);


// Check if there is any data in the read buffer
bool usb_cdc_has_data(void);


// Clear the input buffer of any data
void usb_cdc_clear(void);


// Check if there is a terminal currently connected
bool usb_cdc_is_connected(void);


#if defined(__cplusplus)
}
#endif

#endif // _USB_CDC_H_
