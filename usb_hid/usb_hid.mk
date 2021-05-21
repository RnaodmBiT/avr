
HID_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

PERIPHERALS += usb

CFLAGS += -I$(HID_DIR)
SRC += $(HID_DIR)usb_hid.c
