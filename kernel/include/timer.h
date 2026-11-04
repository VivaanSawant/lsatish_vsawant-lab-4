/**
 * @file timer.h
 *
 * @brief      Interface for the TIM2-TIM5 general purpose timers.
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#ifndef _TIMER_H_
#define _TIMER_H_

/** @brief NVIC IRQ numbers for each timer */
#define TIM2_INT_NUM    28
#define TIM3_INT_NUM    29
#define TIM4_INT_NUM    30
#define TIM5_INT_NUM    50

#include <unistd.h>

/**
 * @brief Starts the timer
 *
 * @param timer      The timer (2 to 5)
 * @param prescalar  Clock divider
 * @param period     Period of the timer interrupt, in prescaled ticks
 */
void timer_init(int timer, uint32_t prescalar, uint32_t period);

/**
 * @brief Stops the timer
 *
 * @param timer      The timer (2 to 5)
 */
void timer_disable(int timer);

/**
 * @brief Clears the timer interrupt bit
 *
 * @param timer      The timer (2 to 5)
 */
void timer_clear_interrupt_bit(int timer);

#endif /* _TIMER_H_ */