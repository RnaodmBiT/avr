#include "timer.h"


void timer_init(timer_t id,
                timer_mode_t mode,
                timer_clock_t clock) {

    switch (id) {
        case TIMER_0:
            if (mode == TIMER_MODE_NORMAL) {
                TCCR0A = 0;
            } else if (mode == TIMER_MODE_COMPARE) {
                TCCR0A = WGM01;
            }

            TCCR0B |= clock & TIMER_CLOCK_SOURCE_MASK;

            break;

        case TIMER_1:
            TCCR1A = 0;
            if (mode == TIMER_MODE_NORMAL) {
                TCCR1B = 0;
            } else if (mode == TIMER_MODE_COMPARE) {
                TCCR1B = BIT(WGM12);
            }

            TCCR1B |= clock & TIMER_CLOCK_SOURCE_MASK;

            break;
    }
}


void timer_interrupt_enable(timer_t id,
                            timer_interrupt_t irq) {

    switch (id) {
        case TIMER_0:
            TIMSK0 |= irq & TIMER0_INTERRUPT_MASK;
            break;

        case TIMER_1:
            TIMSK1 |= irq & TIMER1_INTERRUPT_MASK;
            break;
    }
}


void timer_interrupt_disable(timer_t id,
                            timer_interrupt_t irq) {
    switch (id) {
        case TIMER_0:
            TIMSK0 &= ~(irq & TIMER0_INTERRUPT_MASK);
            break;

        case TIMER_1:
            TIMSK1 &= ~(irq & TIMER1_INTERRUPT_MASK);
            break;
    }
}


void timer_period_set(timer_t id,
                      timer_channel_t channel,
                      timer_period_t period) {

    switch (id) {
        case TIMER_0:
            if (channel == TIMER_CHANNEL_A) {
                OCR0A = period & 0xFF;
            } else if (channel == TIMER_CHANNEL_B) {
                OCR0B = period & 0xFF;
            }
            break;

        case TIMER_1:
            if (channel == TIMER_CHANNEL_A) {
                OCR1AL = LO(period);
                OCR1AH = HI(period);
            } else if (channel == TIMER_CHANNEL_B) {
                OCR1AL = LO(period);
                OCR1AH = HI(period);
            } else if (channel == TIMER_CHANNEL_C) {
                OCR1AL = LO(period);
                OCR1AH = HI(period);
            }
            break;
    }
}
