#ifndef _TARGET_H_
#define _TARGET_H_

#include <pio.h>


#define LED_STATUS      PC6_PIO

// Define the buttons
#define BUTTON_BOOT     PD7_PIO

// Define the radio pins
#define NRF_CS        PB0_PIO
#define NRF_CE        PB4_PIO
#define NRF_IRQ       PB5_PIO

// Define the USB settings
#define USB_PACKET_SIZE         32
#define USB_CLASS_SUBCLASS      2, 0
#define USB_VENDOR_ID           0x03EB
#define USB_PRODUCT_ID          0x2040
#define USB_DEVICE_VERSION      0x0000
#define USB_MANUFACTURER        L"BM"
#define USB_PRODUCT             L"Node v0.1"
#define USB_SERIAL              L"0"


#endif // _TARGET_H_
