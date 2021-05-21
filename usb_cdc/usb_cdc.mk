
CDC_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

PERIPHERALS += usb

CFLAGS += -I$(CDC_DIR)
SRC += $(CDC_DIR)usb_cdc.c

include ../ring/ring.mk
