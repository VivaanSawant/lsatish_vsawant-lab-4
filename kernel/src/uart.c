/**
 * @file uart.c
 *
 * @brief      Interrupt-driven USART2 driver.
 *
 *             TX and RX each use a fixed-size ring buffer. uart_put_byte and
 *             uart_get_byte are the software side; the USART2 interrupt moves
 *             bytes between the buffers and the data register. TXEIE is only
 *             on while the TX buffer has data, and RXNEIE is only on while
 *             the RX buffer has room, so every interrupt does useful work.
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#include <unistd.h>
#include <rcc.h>
#include <uart.h>
#include <uart_polling.h>
#include <nvic.h>
#include <stdint.h>
#include <gpio.h>

#ifndef UART_BUF_SIZE
/** @brief Size of each kernel ring buffer (TX and RX), in bytes */
#define UART_BUF_SIZE 64
#endif

/** @brief NVIC IRQ number for USART2 */
#define USART2_IRQ 38

/** @brief USART register map */
struct uart_reg_map {
  volatile uint32_t SR;
  volatile uint32_t DR;
  volatile uint32_t BRR;
  volatile uint32_t CR1;
  volatile uint32_t CR2;
  volatile uint32_t CR3;
  volatile uint32_t GTPR;
};

/** @brief USART2 base address */
#define USART2_BASE 0x40004400
#define UART ((struct uart_reg_map *) USART2_BASE)

/** @brief UART pins on GPIO A */
#define UART_TX_PIN 2
#define UART_RX_PIN 3

/* RCC_APB1ENR */
#define CLOCK_EN   (1u << 17)
#define APB_CLOCK  16000000

/** @brief BRR layout: 12-bit mantissa above a 4-bit fraction */
#define BRR_MANTISSA_SHIFT 4
#define OVERSAMPLE 16

/* CR1 */
#define CR1_RE     (1u << 2)
#define CR1_TE     (1u << 3)
#define CR1_RXNEIE (1u << 5)
#define CR1_TXEIE  (1u << 7)
#define CR1_UE     (1u << 13)

/* SR */
#define SR_RXNE    (1u << 5)
#define SR_TC      (1u << 6)
#define SR_TXE     (1u << 7)

/** @brief Fixed-size FIFO with O(1) push and pop */
typedef struct {
  char data[UART_BUF_SIZE]; /**< storage */
  int head;                 /**< next write index */
  int tail;                 /**< next read index */
  int count;                /**< bytes currently stored */
} ringbuf_t;

static ringbuf_t tx_buf, rx_buf;
static int tx_irq_on = 0; /* shadow of CR1.TXEIE to avoid MMIO reads */
static int rx_irq_on = 0; /* shadow of CR1.RXNEIE */

/**
 * @brief Disables interrupts and returns the previous PRIMASK.
 *
 * @return PRIMASK value to pass to irq_restore
 */
static inline uint32_t irq_save(void) {
  uint32_t pm;
  __asm volatile ("mrs %0, primask\n cpsid i" : "=r"(pm) :: "memory");
  return pm;
}

/**
 * @brief Restores PRIMASK saved by irq_save.
 *
 * @param pm saved PRIMASK
 */
static inline void irq_restore(uint32_t pm) {
  __asm volatile ("msr primask, %0" :: "r"(pm) : "memory");
}

/** @brief Returns 1 if the ring buffer is full */
static inline int rb_full(const ringbuf_t *rb)  { return rb->count == UART_BUF_SIZE; }

/** @brief Returns 1 if the ring buffer is empty */
static inline int rb_empty(const ringbuf_t *rb) { return rb->count == 0; }

/** @brief Resets a ring buffer to empty */
static void rb_init(ringbuf_t *rb) { rb->head = rb->tail = rb->count = 0; }

/**
 * @brief Appends a byte. Caller ensures the buffer is not full.
 *
 * @param rb buffer
 * @param c  byte to add
 */
static void rb_push(ringbuf_t *rb, char c) {
  rb->data[rb->head] = c;
  if (++rb->head == UART_BUF_SIZE) rb->head = 0;
  rb->count++;
}

/**
 * @brief Removes the oldest byte. Caller ensures the buffer is not empty.
 *
 * @param rb buffer
 *
 * @return the removed byte
 */
static char rb_pop(ringbuf_t *rb) {
  char c = rb->data[rb->tail];
  if (++rb->tail == UART_BUF_SIZE) rb->tail = 0;
  rb->count--;
  return c;
}

/** @brief Turns TXEIE on, writing CR1 only if it was off */
static void tx_irq_enable(void)  { if (!tx_irq_on) { UART->CR1 |= CR1_TXEIE;  tx_irq_on = 1; } }

/** @brief Turns TXEIE off, writing CR1 only if it was on */
static void tx_irq_disable(void) { if (tx_irq_on)  { UART->CR1 &= ~CR1_TXEIE; tx_irq_on = 0; } }

/** @brief Turns RXNEIE on, writing CR1 only if it was off */
static void rx_irq_enable(void)  { if (!rx_irq_on) { UART->CR1 |= CR1_RXNEIE;  rx_irq_on = 1; } }

/** @brief Turns RXNEIE off, writing CR1 only if it was on */
static void rx_irq_disable(void) { if (rx_irq_on)  { UART->CR1 &= ~CR1_RXNEIE; rx_irq_on = 0; } }

/**
 * @brief Moves one byte from the TX buffer to the data register.
 *
 * Only runs while TXEIE is on, which implies the TX buffer is non-empty.
 */
void USART2_TX_IRQHandler(void) {
  UART->DR = rb_pop(&tx_buf); // plain store, never |=
  if (rb_empty(&tx_buf)) tx_irq_disable();
}

/**
 * @brief Moves one byte from the data register to the RX buffer.
 *
 * Only runs while RXNEIE is on, which implies the RX buffer has room.
 */
void USART2_RX_IRQHandler(void) {
  rb_push(&rx_buf, (char)UART->DR); // plain load
  if (rb_full(&rx_buf)) rx_irq_disable();
}

/**
 * @brief USART2 interrupt entry: dispatches to the RX and/or TX handler.
 */
void uart_irq_handler(void) {
  uint32_t sr = UART->SR;  // read status once
  if ((sr & SR_RXNE) && rx_irq_on) USART2_RX_IRQHandler();
  if ((sr & SR_TXE)  && tx_irq_on) USART2_TX_IRQHandler();
  nvic_clear_pending(USART2_IRQ);
}

/**
 * @brief Initializes USART2 for interrupt-driven I/O.
 *
 * TXEIE starts off (nothing to send); RXNEIE starts on (buffer has room).
 *
 * @param baud baud rate
 */
void uart_init(int baud) {
  // pins: PA2 = TX, PA3 = RX, alternate function 7
  gpio_init(GPIO_A, UART_TX_PIN, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);
  gpio_init(GPIO_A, UART_RX_PIN, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);

  //clock
  struct rcc_reg_map *rcc = RCC_BASE;
  rcc->apb1_enr |= CLOCK_EN;

  //USARTDIV = fck / (16 * baud), split into mantissa and 4-bit fraction
  uint32_t mantissa  = APB_CLOCK / (OVERSAMPLE * baud);
  uint32_t remainder = APB_CLOCK % (OVERSAMPLE * baud);
  uint32_t fraction  = (remainder * OVERSAMPLE) / (OVERSAMPLE * baud);
  UART->BRR = (mantissa << BRR_MANTISSA_SHIFT) | fraction;

  // buffers + interrupt-enable initial state
  rb_init(&tx_buf);
  rb_init(&rx_buf);
  tx_irq_on = 0; // TX buffer empty -> TXEIE off
  rx_irq_on = 0;

  // enable UART, receiver, transmitter, and RXNEIE in one write
  UART->CR1 |= (CR1_UE | CR1_RE | CR1_TE | CR1_RXNEIE);
  rx_irq_on = 1; // keep shadow in sync

  nvic_irq(USART2_IRQ, IRQ_ENABLE);
}

/**
 * @brief Queues a byte for transmission.
 *
 * @param c byte to send
 *
 * @return 0 on success, -1 if the TX buffer is full
 */
int uart_put_byte(char c) {
  uint32_t pm = irq_save();
  if (rb_full(&tx_buf)) { irq_restore(pm); return -1; }
  rb_push(&tx_buf, c);
  tx_irq_enable();
  irq_restore(pm);
  return 0;
}

/**
 * @brief Takes a received byte out of the RX buffer.
 *
 * @param c output: the byte read
 *
 * @return 0 on success, -1 if the RX buffer is empty
 */
int uart_get_byte(char *c) {
  uint32_t pm = irq_save();
  if (rb_empty(&rx_buf)) { irq_restore(pm); return -1; }
  *c = rb_pop(&rx_buf);
  rx_irq_enable();
  irq_restore(pm);
  return 0;
}

/** @brief ASCII end-of-transmission: closes the input stream */
#define EOT_CHAR 4

/**
 * @brief Queues a byte, waiting for room in the TX buffer if it's full.
 *
 * @param c byte to send
 */
static void put_byte_blocking(char c) {
  while (uart_put_byte(c) != 0);
}

/**
 * @brief Waits for a byte to arrive in the RX buffer.
 *
 * @return the received byte
 */
static char get_byte_blocking(void) {
  char c;
  while (uart_get_byte(&c) != 0);
  return c;
}

/**
 * @brief Writes a buffer to the serial console.
 *
 * @param file must be STDOUT_FILENO
 * @param ptr  bytes to write
 * @param len  number of bytes
 *
 * @return len on success, -1 if file is not stdout
 */
int uart_write(int file, char *ptr, int len) {
  if (file != STDOUT_FILENO) return -1;
  for (int i = 0; i < len; i++) put_byte_blocking(ptr[i]);
  return len;
}

/**
 * @brief Reads a line from the serial console, echoing as it goes.
 *
 * EOT returns immediately, backspace erases the previous character, and
 * a newline (\n, \r, or \r\n) is stored and ends the read.
 *
 * @param file must be STDIN_FILENO
 * @param ptr  output buffer
 * @param len  buffer size
 *
 * @return number of bytes read, or -1 if file is not stdin
 */
int uart_read(int file, char *ptr, int len) {
  static int last_was_cr = 0;   /* swallow the \n of a Windows \r\n */
  if (file != STDIN_FILENO) return -1;

  int n = 0;
  while (n < len) {
    char c = get_byte_blocking();

    if (c == '\n' && last_was_cr) {
      last_was_cr = 0;
      continue;
    }
    last_was_cr = (c == '\r');
    if (c == '\r') c = '\n';

    if (c == EOT_CHAR) return n;

    if (c == '\b') {
      if (n > 0) {
        n--;
        put_byte_blocking('\b');
        put_byte_blocking(' ');
        put_byte_blocking('\b');
      }
      continue;
    }

    ptr[n++] = c;
    put_byte_blocking(c);
    if (c == '\n') return n;
  }
  return n;
}

/**
 * @brief Drains the TX buffer by polling. Only called from sys_exit.
 *
 * TXEIE is turned off first so the TX interrupt can't pop the buffer
 * concurrently with this loop.
 */
void uart_flush(void) {
  tx_irq_disable();
  while (!rb_empty(&tx_buf)) {
    while (!(UART->SR & SR_TXE));
    UART->DR = rb_pop(&tx_buf);
  }
  while (!(UART->SR & SR_TC));
}