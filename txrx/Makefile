
ifndef BOARD
$(info Defaulting to Node v1.0 board)
BOARD = node_1_0
endif

SRC = txrx.c

TARGET = txrc

include ../nrf/nrf.mk

# the rest of the build chain
include ../avr.mk
