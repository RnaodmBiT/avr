#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>


#if defined(__cplusplus)
extern "C" {
#endif


// Check for a bunch of required board macros
#if !defined(F_CPU)
#error F_CPU not defined
#endif


// Define some bit manipulation macros
#define BIT(x) (1 << (x))

#define LO(x) ((x) & 0xff)
#define HI(x) LO((x) >> 8)

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// Add in a couple of useful macros
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

#define NOOP __asm__ __volatile__("nop");

#define delay_ms(ms) _delay_ms(ms)
#define delay_us(us) _delay_us(us)

/*
 * Define the MCU clock selection options.
 * These choose which source to use when setting the clock.
 */
typedef enum {
    CLOCK_RC,
    CLOCK_EXT
} clock_source_t;

/*
 * Define the MCU clock divider settings.
 * These are used to precase the system clock.
 */
typedef enum {
    CLOCK_DIV_1,
    CLOCK_DIV_2,
    CLOCK_DIV_4,
    CLOCK_DIV_8,
    CLOCK_DIV_16,
    CLOCK_DIV_32,
    CLOCK_DIV_64,
    CLOCK_DIV_128,
    CLOCK_DIV_256
} clock_div_t;


// Initialize the system all in one go
void system_init(clock_source_t source, clock_div_t div);

// Setup the system clock source and divider
void system_clock_set(clock_source_t source, clock_div_t div);

// Get the clock frequency
uint32_t system_clock_get(void);

// Diable the system watchdog timer
void system_watchdog_timer_disable(void);


void panic(void);


#if defined(__cplusplus)
}
#endif

#endif // _SYSTEM_H_
