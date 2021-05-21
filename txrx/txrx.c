#include <system.h>
#include <pio.h>
#include <nrf.h>

#include "target.h"


int main(void) {
    system_init(CLOCK_EXT, CLOCK_DIV_1);

    pio_mode_set(LED_STATUS, PIO_OUTPUT_HIGH);
    pio_mode_set(BUTTON_BOOT, PIO_INPUT);

    nrf_init(false);

    uint8_t rx = 0;
    uint8_t tx = 0;

    while (true) {
        delay_ms(10);

        while (nrf_has_data()) {
            char buf[32];
            nrf_read(buf, sizeof(buf));

            rx = 10;
        }


        if (rx) --rx;
        pio_output_set(LED_STATUS, rx == 0);


        if (pio_input_get(BUTTON_BOOT) == 0) {
            const char buf[] = "Hello";
            nrf_write(buf, sizeof(buf));
        }

    }

    return 0;
}


void panic(void) {
    while (1) {
        delay_ms(50);
        pio_output_toggle(LED_STATUS);
    }
}