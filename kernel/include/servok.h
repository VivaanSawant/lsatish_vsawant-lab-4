/**
 * @file servok.h
 *
 * @brief      Kernel-side servo control: software PWM on two channels and
 *             the servo system calls.
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#ifndef _SERVOK_H_
#define _SERVOK_H_

#include <stdint.h>

extern int pwm_channel0;
extern int pwm_channel1;
extern int ch0_en;
extern int ch1_en;

void servo_init(void);

void servo_timer_handler(void);

int sys_servo_enable(uint8_t channel, uint8_t enabled);

int sys_servo_set(uint8_t channel, uint8_t angle);

#endif /* _SERVOK_H_ */