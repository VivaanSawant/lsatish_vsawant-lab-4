/**
 * @file kernel.c
 *
 * @brief      Kernel entry point: initializes peripherals and launches the
 *             user program.
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#include <stdint.h>
#include <arm.h>
#include <kernel.h>
#include <systick.h>
#include <lcd_driver.h>
#include <printk.h>
#include <uart.h>
#include <servok.h>
#include <unistd.h>

/** @brief UART baud rate required by the handout */
#define UART_BAUD 115200

/** @brief System Handler Priority Register 2; SVCall priority is bits 31:24 */
#define SCB_SHPR2 ((volatile uint32_t *)0xE000ED1C)
#define SVC_PRIO_SHIFT 24

/** @brief Lowest priority (STM32 implements the top 4 bits) */
#define PRIO_LOWEST 0xF0

/** @brief Defined in asm_helpers.S; never returns */
extern void enter_user_mode(void);

/**
 * @brief Kernel main: sets up hardware, then hands off to user mode.
 *
 * @return never returns
 */
int kernel_main(void) {
  init_349(); // DO NOT REMOVE THIS LINE

  /* SVC must be lower priority than SysTick and USART2: syscalls wait on
   * those interrupts (UART buffers, systick_delay), so they need to preempt */
  *SCB_SHPR2 = (PRIO_LOWEST << SVC_PRIO_SHIFT);

  uart_init(UART_BAUD);
  systick_init();       // must precede lcd_driver_init, which uses systick_delay
  lcd_driver_init();
  servo_init();         // starts the TIM2 PWM tick; both channels start disabled

  enter_user_mode();
  return 0;
}