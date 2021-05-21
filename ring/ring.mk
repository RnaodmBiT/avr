
RING_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

CFLAGS += -I$(RING_DIR)
SRC += $(RING_DIR)ring.c
