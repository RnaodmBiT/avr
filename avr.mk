
AVR_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

ifndef BOARD
$(error BOARD must be defined)
endif

# grab the board specifics
include $(AVR_DIR)boards/$(BOARD)/$(BOARD).mk

CFLAGS += -I$(AVR_DIR)boards/$(BOARD)/

ifndef MCU
$(error MCU wasn't defined by the board specification)
endif

ifndef F_CPU
$(error F_CPU wasn't defined by the board specification)
endif

# grab the MCU specifics (including the actual build rules)
include $(AVR_DIR)$(MCU)/$(MCU).mk
