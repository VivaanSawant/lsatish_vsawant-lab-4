/**
 * @file timer.c
 *
 * @brief      Driver for the TIM2-TIM5 general purpose timers, configured to
 *             fire an update interrupt every time the counter overflows.
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#include <unistd.h>
#include <timer.h>
#include <rcc.h>
#include <nvic.h>

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

/** @brief Base addresses of TIM2-TIM5 (RM0368 memory map), indexed by timer number */
struct tim2_5* const timer_base[] = {(void *)0x0,         // N/A - Don't fill out
                                     (void *)0x0,         // N/A - Don't fill out
                                     (void *)0x40000000,  // TIM2
                                     (void *)0x40000400,  // TIM3
                                     (void *)0x40000800,  // TIM4
                                     (void *)0x40000C00}; // TIM5

/** @brief NVIC IRQ number for each timer, indexed by timer number */
static const uint8_t timer_irq[] = {0, 0, TIM2_INT_NUM, TIM3_INT_NUM,
                                    TIM4_INT_NUM, TIM5_INT_NUM};

/** @brief Valid timer numbers */
#define TIMER_MIN 2
#define TIMER_MAX 5

/** @brief RCC_APB1ENR: TIM2EN is bit 0, TIM3EN bit 1, TIM4EN bit 2, TIM5EN bit 3 */
#define RCC_APB1_TIM_EN(timer) (1u << ((timer) - TIMER_MIN))

/** @brief CR1: counter enable */
#define TIM_CR1_CEN (1u << 0)
/** @brief DIER: update interrupt enable */
#define TIM_DIER_UIE (1u << 0)
/** @brief SR: update interrupt flag */
#define TIM_SR_UIF (1u << 0)
/** @brief EGR: update generation (reloads PSC and ARR immediately) */
#define TIM_EGR_UG (1u << 0)

/**
 * @brief Returns 1 if timer is one of TIM2-TIM5.
 *
 * @param timer timer number
 */
static int timer_valid(int timer) {
  return timer >= TIMER_MIN && timer <= TIMER_MAX;
}

/**
 * @brief Starts a timer that fires an update interrupt every period ticks.
 *
 * The counter clock is the 16 MHz system clock divided by prescalar, so the
 * interrupt rate is 16 MHz / (prescalar * period). The hardware divides by
 * PSC + 1 and overflows after ARR + 1 counts, hence the - 1 on each.
 *
 * @param timer     2 to 5
 * @param prescalar clock divider (>= 1)
 * @param period    counter ticks between interrupts (>= 1)
 */
void timer_init(int timer, uint32_t prescalar, uint32_t period) {
  if (!timer_valid(timer) || prescalar == 0 || period == 0) return;

  struct rcc_reg_map *rcc = RCC_BASE;
  struct tim2_5 *tim = timer_base[timer];

  rcc->apb1_enr |= RCC_APB1_TIM_EN(timer);

  tim->psc = prescalar - 1;
  tim->arr = period - 1;

  /* PSC is buffered until the next update event; force one now so the new
   * prescaler takes effect immediately, then clear the flag it sets */
  tim->egr = TIM_EGR_UG;
  tim->sr = ~TIM_SR_UIF;

  tim->dier |= TIM_DIER_UIE;
  nvic_irq(timer_irq[timer], IRQ_ENABLE);
  tim->cr1 |= TIM_CR1_CEN;
}

/**
 * @brief Stops a timer and turns off its clock.
 *
 * @param timer 2 to 5
 */
void timer_disable(int timer) {
  if (!timer_valid(timer)) return;

  struct rcc_reg_map *rcc = RCC_BASE;
  struct tim2_5 *tim = timer_base[timer];

  tim->cr1 &= ~TIM_CR1_CEN;
  tim->dier &= ~TIM_DIER_UIE;
  nvic_irq(timer_irq[timer], IRQ_DISABLE);
  rcc->apb1_enr &= ~RCC_APB1_TIM_EN(timer);
}

/**
 * @brief Clears the update interrupt flag. Call from the timer's handler.
 *
 * SR bits are rc_w0 (cleared by writing 0, unaffected by writing 1), so a
 * plain write clears only UIF without a read-modify-write.
 *
 * @param timer 2 to 5
 */
void timer_clear_interrupt_bit(int timer) {
  if (!timer_valid(timer)) return;
  timer_base[timer]->sr = ~TIM_SR_UIF;
}