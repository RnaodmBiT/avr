
CDC_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
VPATH += $(CDC_DIR)

PERIPHERALS += usb

CFLAGS += -I$(CDC_DIR)
SRC += usb_cdc.c

include ../ring/ring.mk
