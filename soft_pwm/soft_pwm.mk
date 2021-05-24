
SPWM_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
VPATH += $(SPWM_DIR)

CFLAGS += -I$(SPWM_DIR)
SRC += soft_pwm.c

PERIPHERALS += timer
