#ifndef _USB_HID_H_
#define _USB_HID_H_

#include <system.h>
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif


// Initialize the USB HID mouse driver.
void usb_hid_init_mouse(void);

void usb_hid_move_mouse(int8_t dx, int8_t dy);

// Initialize the USB HID keyboard driver
void usb_hid_init_keyboard(void);

void usb_hid_press_key(uint8_t key);
void usb_hid_press_key_e(uint8_t key, uint8_t key2);

// Initialize a generic HID driver
void usb_hid_init_generic(const char* usb_descriptor, const char* hid_descriptor);
void usb_hid_write_generic(const void* data, uint8_t len);


#if defined(__cplusplus)
}
#endif

#endif // _USB_HID_H_
