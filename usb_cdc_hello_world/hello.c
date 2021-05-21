#include <system.h>
#include <usb_cdc.h>
#include <pio.h>

#include <target.h>

int main(void) {

    system_init(CLOCK_EXT, CLOCK_DIV_1);

    pio_mode_set(LED_STATUS, PIO_OUTPUT_HIGH);

    usb_cdc_init();

    sei();

    while (true) {
        delay_ms(5);
        pio_output_toggle(LED_STATUS);

        usb_cdc_puts("Hello\n");

    }
}



void panic(void) {
    while (true) {
        pio_output_toggle(LED_STATUS);
        delay_ms(100);
    }
}
