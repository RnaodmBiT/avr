#include "system.h"


void system_init(clock_source_t source, clock_div_t div) {
    system_clock_set(source, div);

    // Let's just disable this by default, it can be enabled again later
    system_watchdog_timer_disable();
}


void system_clock_set(clock_source_t source, clock_div_t div) {
    if (source == CLOCK_RC) {
        CLKSEL0 &= ~CLKS;
    } else {
        CLKSEL0 |= CLKS;
    }

    CLKPR = BIT(CLKPCE); // Unlock the prescaler register
    CLKPR = div;
}


void system_watchdog_timer_disable(void) {
    wdt_reset();

    // Stop this from overriding the watchdog control register
    MCUSR &= ~BIT(WDRF);
    // Allow editing of the watchdog enable
    WDTCSR |= BIT(WDCE) | BIT(WDE);
    // Disable the watchdog
    WDTCSR = 0;
}



// __attribute__((weak))
// void panic(void) {
//     while (true) ;
// }
