#include "nrf.h"
#include "pio.h"
#include "spi.h"
#include "target.h"


typedef enum NRF24Command {
    R_REGISTER = 0x00,
    W_REGISTER = 0x20,
    R_RX_PAYLOAD = 0x61,
    W_TX_PAYLOAD = 0xa0,
    FLUSH_TX = 0xe1,
    FLUSH_RX = 0xe2,
    REUSE_TX_PL = 0xe3,
    NOP = 0xff
} NRF24Command;


typedef enum NRF24Register {
    CONFIG,
    EN_AA,
    EN_RXADDR,
    SETUP_AW,
    SETUP_RETR,
    RF_CH,
    RF_SETUP,
    STATUS,
    OBSERVE_TX,
    CD,
    RX_ADDR_P0,
    RX_ADDR_P1,
    RX_ADDR_P2,
    RX_ADDR_P3,
    RX_ADDR_P4,
    RX_ADDR_P5,
    TX_ADDR,
    RX_PW_P0,
    RX_PW_P1,
    RX_PW_P2,
    RX_PW_P3,
    RX_PW_P4,
    RX_PW_P5,
    FIFO_STATUS,
    DYNPD = 0x1c,
    FEATURE
} NRF24Register;

#define CONFIG_MASK_RX_DR           BIT(6)
#define CONFIG_MASK_TX_DS           BIT(5)
#define CONFIG_MASK_MAX_RT          BIT(4)
#define CONFIG_EN_CRC               BIT(3)
#define CONFIG_CRCO                 BIT(2)
#define CONFIG_PWR_UP               BIT(1)
#define CONFIG_PRIM_RX              BIT(0)

// mask of the common bits used when switching between TX and RX
#define CONFIG_RXTX_MASK            CONFIG_PRIM_RX | \
                                    CONFIG_MASK_MAX_RT | \
                                    CONFIG_MASK_RX_DR | \
                                    CONFIG_MASK_TX_DS

// common bits for TX mode
#define CONFIG_TX                   CONFIG_MASK_RX_DR
// common bits for RX mode
#define CONFIG_RX                   CONFIG_PRIM_RX
                                    // CONFIG_MASK_TX_DS | \
                                    // CONFIG_MASK_MAX_RT

#define STATUS_DR                   BIT(6)
#define STATUS_DS                   BIT(5)
#define STATUS_RT                   BIT(4)
#define STATUS_TX_FULL              BIT(0)

#define RF_SETUP_DR                 BIT(3)
#define RF_SETUP_PWR0               0x00
#define RF_SETUP_PWR1               0x02
#define RF_SETUP_PWR2               0x04
#define RF_SETUP_PWR3               0x06
#define RF_SETUP_LNA                BIT(0)

void nrf_write_reg8(NRF24Register reg, uint8_t value) {
    pio_output_low(NRF_CS);
    spi_write_byte(W_REGISTER | reg);
    spi_write_byte(value);
    pio_output_high(NRF_CS);
}

void nrf_write_reg40(NRF24Register reg, uint64_t value) {
    pio_output_low(NRF_CS);
    spi_write_byte(W_REGISTER | reg);
    spi_write_byte(value & 0xff);
    spi_write_byte((value >> 8) & 0xff);
    spi_write_byte((value >> 16) & 0xff);
    spi_write_byte((value >> 24) & 0xff);
    spi_write_byte((value >> 32) & 0xff);
    pio_output_high(NRF_CS);
}


uint8_t nrf_read_reg8(NRF24Register reg) {
    pio_output_low(NRF_CS);
    spi_write_byte(R_REGISTER | reg);
    uint8_t value = spi_write_byte(0);
    pio_output_high(NRF_CS);
    return value;
}


uint64_t nrf_read_reg40(NRF24Register reg) {
    uint64_t value = 0;
    pio_output_low(NRF_CS);
    spi_write_byte(R_REGISTER | reg);
    value |= (uint64_t)spi_write_byte(0);
    value |= (uint64_t)spi_write_byte(0) << 8;
    value |= (uint64_t)spi_write_byte(0) << 16;
    value |= (uint64_t)spi_write_byte(0) << 24;
    value |= (uint64_t)spi_write_byte(0) << 32;
    pio_output_high(NRF_CS);
    return value;
}


void nrf_write_reg8_validate(NRF24Register reg, uint8_t value) {
    nrf_write_reg8(reg, value);
    uint8_t recv = nrf_read_reg8(reg);
    if (recv != value) {
        panic();
    }
}


void nrf_write_reg40_validate(NRF24Register reg, uint64_t value) {
    nrf_write_reg40(reg, value);
    uint64_t recv = nrf_read_reg40(reg);
    if (recv != value) {
        panic();
    }
}


void nrf_flush_tx(void) {
    pio_output_low(NRF_CS);
    spi_write_byte(FLUSH_TX);
    pio_output_high(NRF_CS);
    delay_ms(5);
}


void nrf_flush_rx(void) {
    pio_output_low(NRF_CS);
    spi_write_byte(FLUSH_RX);
    pio_output_high(NRF_CS);
    delay_ms(5);
}


uint8_t nrf_read_status(void) {
    // transmit a NOP and read the status byte
    pio_output_low(NRF_CS);
    uint8_t status = spi_write_byte(NOP);
    pio_output_high(NRF_CS);
    return status;
}



void nrf_listen(void) {
    uint8_t config = nrf_read_reg8(CONFIG);
    nrf_write_reg8(CONFIG, (config & ~CONFIG_RXTX_MASK) | CONFIG_RX);

    delay_us(100);

    // move into receive mode
    pio_output_high(NRF_CE);
    delay_us(150);
}



void nrf_init(bool fast) {

    spi_init_master(SPI_MODE_0, SPI_DIV_4);

    // software CS works better than hardware /shrug
    pio_mode_set(NRF_CS, PIO_OUTPUT_HIGH);
    pio_mode_set(NRF_CE, PIO_OUTPUT_LOW);
    pio_mode_set(NRF_IRQ, PIO_INPUT);

    delay_ms(2);

    for (int i = 0; i < 8; ++i) {
        // try to bring the device to a default power down state
        nrf_write_reg8(CONFIG, CONFIG_CRCO | CONFIG_EN_CRC);
        delay_ms(2);
    }


    nrf_flush_rx();
    delay_ms(2);
    nrf_flush_tx();
    delay_ms(2);

    nrf_write_reg8(STATUS, STATUS_DR | STATUS_DS | STATUS_RT);

    // things to setup:

    // data pipes
    nrf_write_reg8_validate(EN_RXADDR, BIT(0));
    // TX/RX address
    nrf_set_address(0x0123456789);
    // payload size
    nrf_write_reg8_validate(RX_PW_P0, 32);

    // RF setup
    if (fast) {
        nrf_write_reg8_validate(RF_SETUP, RF_SETUP_PWR3 | RF_SETUP_LNA | RF_SETUP_DR);
    } else {
        nrf_write_reg8_validate(RF_SETUP, RF_SETUP_PWR3 | RF_SETUP_LNA);
    }

    // RF channel
    nrf_set_channel(42);

    // disable auto retries
    nrf_write_reg8_validate(SETUP_RETR, 0x00);
    // auto acks (default is enabled)
    nrf_write_reg8_validate(EN_AA, 0x00);

    // config
    nrf_write_reg8_validate(CONFIG, CONFIG_CRCO |
                                    CONFIG_EN_CRC |
                                    CONFIG_PWR_UP);

    delay_ms(5);

    nrf_listen();
}


bool nrf_rx_fifo_empty(void) {
    return (nrf_read_reg8(FIFO_STATUS) & BIT(0)) > 0;
}


bool nrf_has_data(void) {
    return nrf_read_status() & STATUS_DR;
    // return !nrf_rx_fifo_empty();
    // return pio_input_get(NRF_IRQ) == 0;
}


void nrf_read(void* data, int len) {
    // clear the status bit

    if (len > 32) len = 32;

    pio_output_low(NRF_CS);
    spi_write_byte(R_RX_PAYLOAD);
    spi_read(data, len);
    // for (int i = len; i < 32; ++i)
        // spi_write_byte(0); // pull out the remaining bytes
    pio_output_high(NRF_CS);

    nrf_flush_rx();

    nrf_write_reg8(STATUS, STATUS_DR);
}


bool nrf_write(const void* data, int len) {
    uint8_t blank[32] = { 0 };

    if (len > 32) len = 32;

    pio_output_low(NRF_CE);
    delay_ms(1);

    // clear the data sent and max retries flags
    nrf_write_reg8(STATUS, STATUS_DS | STATUS_RT);

        // setup the config register
    uint8_t config = nrf_read_reg8(CONFIG);
    nrf_write_reg8(CONFIG, (config & ~CONFIG_RXTX_MASK) | CONFIG_TX);

    nrf_flush_tx();

    // load the message to send
    pio_output_low(NRF_CS);
    spi_write_byte(W_TX_PAYLOAD);
    spi_write(data, len);
    for (int i = len; i < 32; ++i)
        spi_write_byte(0); // fill the rest of the packet with blanks
    pio_output_high(NRF_CS);

    // pulse a transmit
    pio_output_high(NRF_CE);
    delay_us(15);
    pio_output_low(NRF_CE);

    // loop until data is sent or max retries (or timeout)
    uint8_t status;
    uint8_t timeout = 20;
    while (--timeout && pio_input_get(NRF_IRQ)) {
        status = nrf_read_status();
        if (status & STATUS_DS || status & STATUS_RT) {
            break;
        }

        delay_us(100);
    };

    // clear the data sent and max retries flags
    nrf_write_reg8(STATUS, STATUS_DS | STATUS_RT);

    delay_ms(2);

    // NOTE: automatically put the radio back in RX mode
    nrf_listen();

    return (status & STATUS_DS) && timeout > 0;
}


void nrf_set_address(uint64_t address) {
    nrf_write_reg40(RX_ADDR_P0, address);
    nrf_write_reg40(TX_ADDR, address);
}


void nrf_set_channel(uint8_t channel) {
    nrf_write_reg8_validate(RF_CH, channel);
}

