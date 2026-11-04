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
#include <unistd.h>

/** @brief UART baud rate required by the handout */
#define UART_BAUD 115200

/** @brief SysTick rate: 1 kHz, so one tick is 1 ms */
#define SYSTICK_FREQ 1000

/** @brief Defined in asm_helpers.S; never returns */
extern void enter_user_mode(void);

/**
 * @brief Kernel main: sets up hardware, then hands off to user mode.
 *
 * @return never returns
 */
int kernel_main(void) {
  /* also sets SVC below SysTick/PendSV priority, so syscalls that wait on
   * those interrupts (UART buffers, systick_delay) can be preempted */
  init_349(); // DO NOT REMOVE THIS LINE

  uart_init(UART_BAUD);
  systick_init(SYSTICK_FREQ); // must precede lcd_driver_init, which uses systick_delay
  lcd_driver_init();

  enter_user_mode();
  return 0;
}