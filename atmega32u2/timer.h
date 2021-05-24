#ifndef _TIMER_H_
#define _TIMER_H_

#include "system.h"

#if defined(_cplusplus)
extern "C" {
#endif

#define TIMER_CLOCK_SOURCE_MASK     0x7
#define TIMER0_INTERRUPT_MASK       0x7
#define TIMER1_INTERRUPT_MASK       0xF


typedef enum {
    TIMER_0,
    TIMER_1
} timer_t;


typedef uint16_t timer_period_t;


typedef enum {
    TIMER_MODE_NORMAL,
    TIMER_MODE_COMPARE
} timer_mode_t;


typedef enum {
    TIMER_CLOCK_NONE,
    TIMER_CLOCK_DIV_1,
    TIMER_CLOCK_DIV_8,
    TIMER_CLOCK_DIV_64,
    TIMER_CLOCK_DIV_256,
    TIMER_CLOCK_DIV_1024,
    TIMER_CLOCK_EXT_FALLING,
    TIMER_CLOCK_EXT_RISING
} timer_clock_t;


typedef enum {
    TIMER_INTERRUPT_OVERFLOW = BIT(0),
    TIMER_INTERRUPT_OCA = BIT(1),
    TIMER_INTERRUPT_OCB = BIT(2),
    TIMER_INTERRUPT_OCC = BIT(3)
} timer_interrupt_t;


typedef enum {
    TIMER_CHANNEL_A,
    TIMER_CHANNEL_B,
    TIMER_CHANNEL_C
} timer_channel_t;


// Initialize a timer with the given mode and clock
void timer_init(timer_t id,
                timer_mode_t mode,
                timer_clock_t clock);


// Enable interrupts for the timer using a given source
void timer_interrupt_enable(timer_t id,
                            timer_interrupt_t irq);


// Disable interrupts for the timer from the given source
void timer_interrupt_disable(timer_t id,
                             timer_interrupt_t irq);


// Set the output compare period for the given channel
void timer_period_set(timer_t id,
                      timer_channel_t channel,
                      timer_period_t period);


#if defined(_cplusplus)
}
#endif

#endif // _TIMER_H_
