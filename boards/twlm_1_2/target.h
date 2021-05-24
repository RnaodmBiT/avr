#ifndef _TARGET_H_
#define _TARGET_H_

#include <pio.h>

// Define the IO pins
#define LED_STATUS      PD1_PIO
#define LED_ERROR       PD2_PIO

// Define the buttons
#define BUTTON_SYNC     PD6_PIO
#define BUTTON_BOOT     PD7_PIO

// Define the radio pins
#define NRF_CS          PB0_PIO
#define NRF_CE          PB5_PIO
#define NRF_IRQ         PB4_PIO

// Define the motor driver pins
#define AIN1            PB7_PIO
#define AIN2            PD0_PIO
#define BIN1            PC5_PIO
#define BIN2            PC6_PIO

#define MOTOR_NSLEEP    PD6_PIO
// could be: PD0, PD4, PD5, PD6, PB6, PC7


#define SOFT_PWM_PINS   { AIN1, AIN2, BIN1, BIN2 }

// Define the USB settings
#define USB_PACKET_SIZE         32
#define USB_CLASS_SUBCLASS      2, 0
#define USB_VENDOR_ID           0x03EB
#define USB_PRODUCT_ID          0x2040
#define USB_DEVICE_VERSION      0x0000
#define USB_MANUFACTURER        L"BM"
#define USB_PRODUCT             L"TWLM v1.1"
#define USB_SERIAL              L"001"


#endif // _TARGET_H_
