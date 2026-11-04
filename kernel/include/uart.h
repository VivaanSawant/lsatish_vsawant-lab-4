/**
 * @file uart.h
 *
 * @brief      Interrupt-driven UART interface used by printk and syscalls.
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#ifndef _UART_H_
#define _UART_H_

void uart_init(int baud);

int uart_put_byte(char c);

int uart_get_byte(char *c);

void uart_flush(void);

int uart_write( int file, char *ptr, int len );

int uart_read(int file, char *ptr, int len );

#endif /* _UART_H_ */