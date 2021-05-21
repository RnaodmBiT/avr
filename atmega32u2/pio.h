#ifndef _PIO_H_
#define _PIO_H_

#include "system.h"

#if defined(__cplusplus)
extern "C" {
#endif

// PORTxn       Output on Port x Pin n - When input pin, logic 1 sets pull-up
// DDRxn        Direction on Port x Pin n - Logic 1 is output
// PINxn        Input on Port x Pin n - Writing 1 here toggles PORTxn

typedef enum { PORT_B, PORT_C, PORT_D } pio_port_t;

typedef enum {
    PIO_INPUT,
    PIO_INPUT_PULLUP,
    PIO_OUTPUT_LOW,
    PIO_OUTPUT_HIGH
} pio_mode_t;

typedef uint16_t pio_t;

// Set the low byte as the port number and the high byte as the pin mask
#define PIO_DEFINE(port, pin)  ((pio_t)((((port) - PORT_B) * 3) | (BIT(pin) << 8)))

// Get the bitmask for the given pio
#define PIO_BITMASK(pio) ((pio) >> 8)

// Get the PIN register
#define PIO_PIN(pio) (*(&PINB + ((pio) & 0xff)))

// Get the DDR register
#define PIO_DDR(pio) (*(&DDRB + ((pio) & 0xff)))

// Get the PORT register
#define PIO_PORT(pio) (*(&PORTB + ((pio) & 0xff)))

inline void pio_mode_set(pio_t pio, pio_mode_t mode) {
    switch (mode) {
        case PIO_INPUT:
            PIO_DDR(pio) &= ~PIO_BITMASK(pio);
            PIO_PORT(pio) &= ~PIO_BITMASK(pio);
            break;

        case PIO_INPUT_PULLUP:
            PIO_DDR(pio) &= ~PIO_BITMASK(pio);
            PIO_PORT(pio) |= PIO_BITMASK(pio);
            break;

        case PIO_OUTPUT_LOW:
            PIO_DDR(pio) |= PIO_BITMASK(pio);
            PIO_PORT(pio) &= ~PIO_BITMASK(pio);
            break;

        case PIO_OUTPUT_HIGH:
            PIO_DDR(pio) |= PIO_BITMASK(pio);
            PIO_PORT(pio) |= PIO_BITMASK(pio);
            break;
    }
}


inline void pio_output_high(pio_t pio) {
    PIO_PORT(pio) |= PIO_BITMASK(pio);
}


inline void pio_output_low(pio_t pio) {
    PIO_PORT(pio) &= ~PIO_BITMASK(pio);
}


inline void pio_output_toggle(pio_t pio) {
    PIO_PIN(pio) |= PIO_BITMASK(pio);
}


inline bool pio_input_get(pio_t pio) {
    return (PIO_PIN(pio) & PIO_BITMASK(pio)) != 0;
}


inline void pio_output_set(pio_t pio, bool state) {
    state ? pio_output_high(pio) : pio_output_low(pio);
}

// Define the list of available PIOs
#define PB0_PIO PIO_DEFINE(PORT_B, 0)
#define PB1_PIO PIO_DEFINE(PORT_B, 1)
#define PB2_PIO PIO_DEFINE(PORT_B, 2)
#define PB3_PIO PIO_DEFINE(PORT_B, 3)
#define PB4_PIO PIO_DEFINE(PORT_B, 4)
#define PB5_PIO PIO_DEFINE(PORT_B, 5)
#define PB6_PIO PIO_DEFINE(PORT_B, 6)
#define PB7_PIO PIO_DEFINE(PORT_B, 7)

#define PC0_PIO PIO_DEFINE(PORT_C, 0)
#define PC1_PIO PIO_DEFINE(PORT_C, 1)
#define PC2_PIO PIO_DEFINE(PORT_C, 2)
#define PC3_PIO PIO_DEFINE(PORT_C, 3)
#define PC4_PIO PIO_DEFINE(PORT_C, 4)
#define PC5_PIO PIO_DEFINE(PORT_C, 5)
#define PC6_PIO PIO_DEFINE(PORT_C, 6)
#define PC7_PIO PIO_DEFINE(PORT_C, 7)

#define PD0_PIO PIO_DEFINE(PORT_D, 0)
#define PD1_PIO PIO_DEFINE(PORT_D, 1)
#define PD2_PIO PIO_DEFINE(PORT_D, 2)
#define PD3_PIO PIO_DEFINE(PORT_D, 3)
#define PD4_PIO PIO_DEFINE(PORT_D, 4)
#define PD5_PIO PIO_DEFINE(PORT_D, 5)
#define PD6_PIO PIO_DEFINE(PORT_D, 6)
#define PD7_PIO PIO_DEFINE(PORT_D, 7)


#if defined(__cplusplus)
}
#endif

#endif // _PIO_H_

