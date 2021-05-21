#include "spi.h"


void spi_init_master(spi_mode_t mode, spi_speed_t speed) {
    pio_mode_set(PB1_PIO, PIO_OUTPUT_HIGH);
    pio_mode_set(PB2_PIO, PIO_OUTPUT_HIGH);
    pio_mode_set(PB3_PIO, PIO_INPUT_PULLUP);

    // Enable SPI in master mode
    SPCR |= BIT(SPE) | BIT(MSTR);

    // Set the mode and clock rate
    SPCR |= (mode << 2) | speed;
}

