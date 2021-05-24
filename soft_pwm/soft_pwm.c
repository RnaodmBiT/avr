#include <target.h>
#include <timer.h>
#include "soft_pwm.h"


const static pio_t pwm_pins[] = SOFT_PWM_PINS;
#define SOFT_PWM_CHANNELS ARRAY_SIZE(pwm_pins)

static pwm_duty_t pwm_duty[SOFT_PWM_CHANNELS] = { 0 };


void soft_pwm_init(void) {
    timer_init(TIMER_0, TIMER_MODE_NORMAL, TIMER_CLOCK_DIV_1);
    timer_interrupt_enable(TIMER_0, TIMER_INTERRUPT_OVERFLOW);

    for (uint8_t i = 0; i < SOFT_PWM_CHANNELS; ++i) {
        pio_mode_set(pwm_pins[i], PIO_OUTPUT_LOW);
    }
}


void soft_pwm_set(pio_t pin,
                         pwm_duty_t duty) {
    for (uint8_t i = 0; i < SOFT_PWM_CHANNELS; ++i) {
        if (pwm_pins[i] == pin) {
            pwm_duty[i] = 255 - duty;
        }
    }
}


ISR(TIMER0_OVF_vect) {
    static uint8_t time = 0;

#define CLEAR_PIN(p) \
    if (SOFT_PWM_CHANNELS > p) {\
        pio_output_low(pwm_pins[p]); \
    }

#define SET_PIN(p) \
    if (SOFT_PWM_CHANNELS > p && time == pwm_duty[p]) {\
        pio_output_high(pwm_pins[p]); \
    }

    if (++time == 0) {
        // Check for a new cycle, then clear all output pins to low
        CLEAR_PIN(0);
        CLEAR_PIN(1);
        CLEAR_PIN(2);
        CLEAR_PIN(3);
        CLEAR_PIN(4);
        CLEAR_PIN(5);
        CLEAR_PIN(6);
        CLEAR_PIN(7);
    }

    SET_PIN(0);
    SET_PIN(1);
    SET_PIN(2);
    SET_PIN(3);
    SET_PIN(4);
    SET_PIN(5);
    SET_PIN(6);
    SET_PIN(7);

#undef CLEAR_PIN
#undef SET_PIN
}

