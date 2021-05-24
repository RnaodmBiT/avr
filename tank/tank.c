#include <system.h>
#include <pio.h>
#include <target.h>
#include <nrf.h>
#include <soft_pwm.h>
#include <usb_cdc.h>


void set_motor_left_coast(int8_t speed) {
    if (speed >= 0) {
        soft_pwm_set(AIN1, (uint8_t)speed * 2);
        soft_pwm_set(AIN2, 0);
    } else {
        soft_pwm_set(AIN1, 0);
        soft_pwm_set(AIN2, (uint8_t)(-speed) * 2);
    }
}

void set_motor_right_coast(int8_t speed) {
    if (speed >= 0) {
        soft_pwm_set(BIN1, 0);
        soft_pwm_set(BIN2, (uint8_t)speed * 2);
    } else {
        soft_pwm_set(BIN1, (uint8_t)(-speed) * 2);
        soft_pwm_set(BIN2, 0);
    }
}

void set_motor_left_brake(int8_t speed) {
    if (speed >= 0) {
        soft_pwm_set(AIN2, (uint8_t)speed * 2);
        soft_pwm_set(AIN1, 255);
    } else {
        soft_pwm_set(AIN2, 255);
        soft_pwm_set(AIN1, (uint8_t)(-speed) * 2);
    }
}

void set_motor_right_brake(int8_t speed) {
    if (speed >= 0) {
        soft_pwm_set(BIN2, 255);
        soft_pwm_set(BIN1, (uint8_t)speed * 2);
    } else {
        soft_pwm_set(BIN2, (uint8_t)(-speed) * 2);
        soft_pwm_set(BIN1, 255);
    }
}

int main(void) {
    system_init(CLOCK_EXT, CLOCK_DIV_1);

    pio_mode_set(LED_STATUS, PIO_OUTPUT_HIGH);
    pio_mode_set(LED_ERROR, PIO_OUTPUT_LOW);

    pio_mode_set(MOTOR_NSLEEP, PIO_OUTPUT_HIGH);

    soft_pwm_init();
    usb_cdc_init();

    sei();

    nrf_init(false);

    uint8_t timeout = 0;

    int8_t left = 0, right = 0;

    uint8_t blink = 1;

    while (true) {
        delay_ms(5);

        if (--blink == 0) {
            pio_output_toggle(LED_STATUS);
            blink = 20;
        }

        while (nrf_has_data()) {
            char buf[32];
            nrf_read(buf, sizeof(buf));

            if (sscanf(buf, "%hhi,%hhi\n", &left, &right) == 2) {
                timeout = 20;



                char str[64];
                sprintf(str, "l = %hhi, r = %hhi\n", left, right);
                usb_cdc_puts(str);
            } else {
                left = 0;
                right = 0;
            }
        }

        // we are still in communiation with the remote, behave as normal
        if (timeout) {
            timeout--;

            pio_output_low(LED_ERROR);

            set_motor_left_coast(left);
            set_motor_right_coast(right);
        } else {
            // lost connection with remote
            pio_output_high(LED_ERROR);


            soft_pwm_set(AIN1, 0);
            soft_pwm_set(AIN2, 0);
            soft_pwm_set(BIN1, 0);
            soft_pwm_set(BIN2, 0);

        }
    }


    return 0;
}


void panic(void) {
    while (true) {
        delay_ms(100);
        pio_output_toggle(LED_ERROR);
    }
}
