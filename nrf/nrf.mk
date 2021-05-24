
NRF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
VPATH += $(NRF_DIR)

PERIPHERALS += spi

CFLAGS += -I$(NRF_DIR)
SRC += nrf.c
