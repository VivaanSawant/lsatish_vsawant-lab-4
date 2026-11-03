/**
 * @file
 *
 * @brief
 *
 * @date
 *
 * @author
 */

#include <unistd.h>
#include <timer.h>

#define UNUSED __attribute__((unused))

/** @brief tim2_5 */
struct tim2_5 {
  volatile uint32_t cr1; /**< 00 Control Register 1 */
  volatile uint32_t cr2; /**< 04 Control Register 2 */
  volatile uint32_t smcr; /**< 08 Slave Mode Control */
  volatile uint32_t dier; /**< 0C DMA/Interrupt Enable */
  volatile uint32_t sr; /**< 10 Status Register */
  volatile uint32_t egr; /**< 14 Event Generation */
  volatile uint32_t ccmr[2]; /**< 18-1C Capture/Compare Mode */
  volatile uint32_t ccer; /**< 20 Capture/Compare Enable */
  volatile uint32_t cnt; /**< 24 Counter Register */
  volatile uint32_t psc; /**< 28 Prescaler Register */
  volatile uint32_t arr; /**< 2C Auto-Reload Register */
  volatile uint32_t reserved_1; /**< 30 */
  volatile uint32_t ccr[4]; /**< 34-40 Capture/Compare */
  volatile uint32_t reserved_2; /**< 44 */
  volatile uint32_t dcr; /**< 48 DMA Control Register */
  volatile uint32_t dmar; /**< 4C DMA address for full transfer Register */
  volatile uint32_t or; /**< 50 Option Register */
};

struct tim2_5* const timer_base[] = {(void *)0x0,   // N/A - Don't fill out
                                     (void *)0x0,   // N/A - Don't fill out
                                     (void *)-1,    // TODO: fill out address for TIMER 2
                                     (void *)-1,    // TODO: fill out address for TIMER 3
                                     (void *)-1,    // TODO: fill out address for TIMER 4
                                     (void *)-1};   // TODO: fill out address for TIMER 5


void timer_init(UNUSED int timer, UNUSED uint32_t prescalar, UNUSED uint32_t period) {

}

void timer_disable(UNUSED int timer) {

}

void timer_clear_interrupt_bit(UNUSED int timer) {

}