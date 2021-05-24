
ifneq ($(BOARD),node_1_0)
$(error BOARD must be node_1_0)
endif

SRC = tank_remote.c

TARGET = tank_remote

include ../nrf/nrf.mk
include ../usb_cdc/usb_cdc.mk

# the rest of the build chain
include ../avr.mk
