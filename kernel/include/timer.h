#ifndef _TIMER_H_
#define _TIMER_H_

#define TIM2_INT_NUM    28
#define TIM3_INT_NUM    29
#define TIM4_INT_NUM    30
#define TIM5_INT_NUM    50

#include <unistd.h>

/*
 * Starts the timer
 *
 * @param timer      - The timer
 * @param prescaler  - Prescalar for clock
 * @param Period     - Period of the timer interrupt
 */
void timer_init(int timer, uint32_t prescalar, uint32_t period);

/*
 * Stops the timer
 *
 * @param timer      - The timer
 */
void timer_disable(int timer);

/*
 * Clears the timer interrupt bit
 *
 * @param timer      - The timer
 */
void timer_clear_interrupt_bit(int timer);

#endif /* _TIMER_H_ */