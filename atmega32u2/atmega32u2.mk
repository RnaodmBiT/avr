
DIR := $(dir $(lastword $(MAKEFILE_LIST)))
VPATH += $(DIR)

ifndef F_CPU
$(error F_CPU must be defined)
endif

ifndef VERBOSE
Q = @
endif


SRC += system.c $(foreach p, $(PERIPHERALS), $(p).c)

CC = avr-gcc
OBJCOPY = avr-objcopy

CFLAGS += -I$(DIR) -g -O3 -mmcu=atmega32u2 -DF_CPU=$(F_CPU)

ELF = $(TARGET).elf
HEX = $(TARGET).hex

OBJ = $(SRC:.c=.o)

all: $(ELF)

%.o: %.c
	$(info CC $^)
	$(Q) $(CC) $(CFLAGS) -o $@ -c $^

$(ELF): $(OBJ)
	$(info LD $@)
	$(Q) $(CC) $(CFLAGS) -o $@ $^

$(HEX): $(ELF)
	$(info OBJCOPY $@)
	$(Q) $(OBJCOPY) -O ihex $^ $@



clean:
	rm -f $(OBJ) $(ELF) $(HEX)

erase:
	$(info Erasing flash...)
	$(Q) dfu-programmer $(MCU) erase

program: $(HEX) erase
	$(info Uploading...)
	$(Q) dfu-programmer $(MCU) flash $<
	$(Q) dfu-programmer $(MCU) launch || /bin/true
