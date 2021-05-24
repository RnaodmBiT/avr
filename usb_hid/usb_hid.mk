
HID_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
VPATH += $(HID_DIR)

PERIPHERALS += usb

CFLAGS += -I$(HID_DIR)
SRC += usb_hid.c
