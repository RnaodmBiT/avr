#ifndef _SPI_H_
#define _SPI_H_

#include "pio.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef enum {
    SPI_MODE_0,
    SPI_MODE_1,
    SPI_MODE_2,
    SPI_MODE_3
} spi_mode_t;


typedef enum {
    SPI_DIV_4,
    SPI_DIV_16,
    SPI_DIV_64,
    SPI_DIV_128
} spi_speed_t;


inline uint8_t spi_write_byte(uint8_t byte) {
    SPCR |= BIT(MSTR);
    SPDR = byte;
    while (!(SPSR & BIT(SPIF))) ;
    return SPDR;
}


inline void spi_write(const void* tx, uint8_t len) {
    const uint8_t* data = (const uint8_t*)tx;
    for (uint8_t i = 0; i < len; ++i) {
        spi_write_byte(data[i]);
    }
}


inline void spi_read(void* rx, uint8_t len) {
    uint8_t* data = (uint8_t*)rx;
    for (uint8_t i = 0; i < len; ++i) {
        data[i] = spi_write_byte(0);
    }
}


inline void spi_transact(const void* tx, void* rx, uint8_t len) {
    const uint8_t* data_tx = (const uint8_t*)tx;
    uint8_t* data_rx = (uint8_t*)rx;
    for (uint8_t i = 0; i < len; ++i) {
        data_rx[i] = spi_write_byte(data_tx[i]);
    }
}


void spi_init_master(spi_mode_t mode, spi_speed_t speed);

#if defined(__cplusplus)
}
#endif

#endif // _SPI_H_
