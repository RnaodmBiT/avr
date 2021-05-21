#include <system.h>
#include <pio.h>

#include "target.h"


int main(void) {
    system_init(CLOCK_EXT, CLOCK_DIV_1);
    pio_mode_set(LED_STATUS, PIO_OUTPUT_HIGH);

    while (1) {
        delay_ms(200);
        pio_output_toggle(LED_STATUS);
    }

    return 0;
}

