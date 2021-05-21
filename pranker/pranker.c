#include <system.h>
#include <usb_hid.h>
#include <nrf.h>
#include <target.h>

int main(void) {
    system_init(CLOCK_EXT, CLOCK_DIV_1);

    pio_mode_set(LED_STATUS, PIO_OUTPUT_HIGH);

    usb_hid_init_mouse();

    nrf_init(false);

    sei();

    uint8_t rx = 0;

    while (true) {
        delay_ms(1);

        if (nrf_has_data()) {
            char buf[32];
            nrf_read(buf, sizeof(buf)); // discard the message

            rx = 10;
        }

        if (rx > 0) {
            pio_output_low(LED_STATUS);

            int dx = rand() - (RAND_MAX / 2);
            int dy = rand() - (RAND_MAX / 2);
            usb_hid_move_mouse(dx / 2000,
                               dy / 2000);

            rx--;
        } else {
            pio_output_high(LED_STATUS);
        }
    }
}