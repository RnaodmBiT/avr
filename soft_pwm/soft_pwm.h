#ifndef _SOFT_PWM_H_
#define _SOFT_PWM_H_

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(SOFT_PWM_PINS)
#error SOFT_PWM_PINS is not defined.
#endif


#if defined(TIMER0_USED)
#error Timer 0 has already been used.
#endif

#define TIMER0_USED

typedef uint8_t pwm_channel_t;
typedef uint8_t pwm_duty_t;

// Initialize the software PWM module
void soft_pwm_init(void);

// Set the output for the given channel on a pin at a given duty cycle
void soft_pwm_set(pio_t pin,
                         pwm_duty_t duty);

// Disable the PWM control over it's set PIO pin
void soft_pwm_output_clear(pwm_channel_t channel);


#if defined(__cplusplus)
}
#endif


#endif // _SOFT_PWM_H_
