#include <system.h>
#include <pio.h>
#include <target.h>
#include <nrf.h>
#include <usb_cdc.h>

#include <string.h>


int main(void) {
    system_init(CLOCK_EXT, CLOCK_DIV_1);

    pio_mode_set(LED_STATUS, PIO_OUTPUT_HIGH);

    usb_cdc_init();
    sei();

    nrf_init(false);

    uint8_t blink = 1;

    int8_t left = 0, right = 0;

    uint8_t pos = 0;
    char buffer[255];
    memset(buffer, 0, 255);

    while (true) {
        delay_ms(10);

        if (--blink == 0) {
            pio_output_toggle(LED_STATUS);
            blink = 10;
        }

        while (usb_cdc_has_data()) {
            buffer[pos] = usb_cdc_getc();


            // usb_cdc_putc('\n');
            // usb_cdc_puts(buffer);
            usb_cdc_putc(buffer[pos]); // echo it back out

            if (buffer[pos] == '\n') {
                int8_t tmp_left, tmp_right;
                if (sscanf(buffer, "%hhi,%hhi\n", &tmp_left, &tmp_right) == 2) {
                    left = tmp_left;
                    right = tmp_right;

                    char out[100];
                    sprintf(out, "> set L=%hhi, R=%hhi\n", left, right);
                    usb_cdc_puts(out);
                }

                // reset the buffer
                pos = 0;
                memset(buffer, 0, sizeof(buffer));
            } else if (++pos == 255) {
                // reset the buffer
                pos = 0;
                memset(buffer, 0, sizeof(buffer));
            }
        }

        char buf[32];
        sprintf(buf, "%hhi,%hhi\n", left, right);

        nrf_write(buf, sizeof(buf));
    }

    return 0;
}


void panic(void) {
    while (true) {
        delay_ms(500);
        pio_output_toggle(LED_STATUS);
    }
}
