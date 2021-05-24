
RING_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
VPATH += $(RING_DIR)

CFLAGS += -I$(RING_DIR)
SRC += ring.c
