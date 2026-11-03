/**
 * @file syscall.c
 *
 * @brief      Kernel implementations of the newlib system calls.
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#include <unistd.h>
#include <stdint.h>
#include <syscall.h>
#include <uart.h>
#include <printk.h>
#include <lcd_driver.h>
#include <arm.h>

/** @brief Heap bounds from the linker script */
extern char __heap_low;
extern char __heap_top;

/** @brief Enough for "-2147483648" plus the null terminator */
#define INT_STR_LEN 12
#define DECIMAL_BASE 10

/**
 * @brief Grows the user heap by incr bytes.
 *
 * @param incr bytes to add (may be negative)
 *
 * @return the previous break, or (void *)-1 if the heap would overflow
 */
void *sys_sbrk(int incr) {
  static char *brk = &__heap_low;

  if (brk + incr > &__heap_top || brk + incr < &__heap_low) {
    return (void *)-1;
  }
  char *prev = brk;
  brk += incr;
  return prev;
}

/**
 * @brief Writes to stdout over UART.
 *
 * @param file must be stdout
 * @param ptr  bytes to write
 * @param len  number of bytes
 *
 * @return bytes written, or -1 on error
 */
int sys_write(int file, char *ptr, int len) {
  return uart_write(file, ptr, len);
}

/**
 * @brief Reads from stdin over UART, with echo and line editing.
 *
 * @param file must be stdin
 * @param ptr  output buffer
 * @param len  buffer size
 *
 * @return bytes read, or -1 on error
 */
int sys_read(int file, char *ptr, int len) {
  return uart_read(file, ptr, len);
}

/**
 * @brief Converts an int to a decimal string.
 *
 * @param value number to convert
 * @param out   buffer of at least INT_STR_LEN bytes
 */
static void int_to_str(int value, char *out) {
  char tmp[INT_STR_LEN];
  int i = 0;
  unsigned int mag = (value < 0) ? -(unsigned int)value : (unsigned int)value;

  do {
    tmp[i++] = '0' + (mag % DECIMAL_BASE);
    mag /= DECIMAL_BASE;
  } while (mag > 0);

  int j = 0;
  if (value < 0) out[j++] = '-';
  while (i > 0) out[j++] = tmp[--i];
  out[j] = '\0';
}

/**
 * @brief Ends the user program: reports the exit code on UART and the LCD,
 *        then sleeps forever with interrupts disabled.
 *
 * @param status exit code
 */
void sys_exit(int status) {
  char status_str[INT_STR_LEN];

  printk("Exit status: %d\n", status);
  uart_flush();

  int_to_str(status, status_str);
  lcd_clear();
  lcd_print("Exit: ");
  lcd_print(status_str);

  disable_interrupts();
  while (1) {
    wait_for_interrupt();
  }
}