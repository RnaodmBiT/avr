
NRF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

PERIPHERALS += spi

CFLAGS += -I$(NRF_DIR)
SRC += $(NRF_DIR)nrf.c
