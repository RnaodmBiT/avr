#include <system.h>
#include <usb_cdc.h>
#include <pio.h>
#include <nrf.h>
#include <target.h>

int main(void) {

    system_init(CLOCK_EXT, CLOCK_DIV_1);

    pio_mode_set(LED_STATUS, PIO_OUTPUT_HIGH);
    pio_mode_set(BUTTON_SYNC, PIO_INPUT_PULLUP);

    nrf_init(false);

    uint8_t blink = 1;

    while (true) {
        delay_ms(10);

        if (--blink == 0) {
            pio_output_toggle(LED_STATUS);
            blink = 20;
        }

        if (pio_input_get(BUTTON_SYNC) == 0) {
            char hello[32];
            sprintf(hello, "Hello %hhu\n", blink);
            nrf_write(hello, sizeof(hello));
        }
    }
}



void panic(void) {
    while (true) {
        pio_output_toggle(LED_STATUS);
        delay_ms(100);
    }
}
