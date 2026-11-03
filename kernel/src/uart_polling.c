#include <unistd.h>

#include <gpio.h>
#include <rcc.h>
#include <uart_polling.h>

/** @brief The UART register map. */
struct uart_reg_map {
    volatile uint32_t SR;   /**< Status Register */
    volatile uint32_t DR;   /**<  Data Register */
    volatile uint32_t BRR;  /**<  Baud Rate Register */
    volatile uint32_t CR1;  /**<  Control Register 1 */
    volatile uint32_t CR2;  /**<  Control Register 2 */
    volatile uint32_t CR3;  /**<  Control Register 3 */
    volatile uint32_t GTPR; /**<  Guard Time and Prescaler Register */
};

/** @brief Base address for UART2 */
#define UART2_BASE  (struct uart_reg_map *) 0x40004400

/** @brief Enable  Bit for UART Config register */
#define UART_EN (1 << 13)
#define RECEIVER_EN (1 << 2)
#define TRANSMITTER_EN (1 << 3)
#define CLOCK_EN (1 << 17)
#define APB_CLOCK 16000000
#define UART_TXE (1 << 7) // data register status (for puts)
#define UART_RXNE (1 << 5) // data register status (for gets)

/**
 * @brief initializes UART to given baud rate with 8-bit word length, 1 stop bit, 0 parity bits
 *
 * @param baud Baud rate
 */
void uart_polling_init (int baud){
    struct uart_reg_map *uart = UART2_BASE;

    // TX: PA_2
    gpio_init(GPIO_A, 2, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);

    // RX: PA_3
    gpio_init(GPIO_A, 3, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);

    // Setting clock enable bit to 1
    struct rcc_reg_map* rccAPB = RCC_BASE;
    rccAPB -> apb1_enr |= CLOCK_EN;


    // setting the baud rate at runtime

    uint32_t mantissa = APB_CLOCK / (16 * baud);
    uint32_t remainder = APB_CLOCK % (16 * baud);
    uint32_t fraction = (remainder * 16) / (16 * baud);

    uart -> BRR = (mantissa << 4) | fraction; // uart init earlier
    // UART --> calculated to be 8.605 in this

    // setting the enables
    uart -> CR1 |= (UART_EN | RECEIVER_EN | TRANSMITTER_EN);

    return;
}

/**
 * @brief transmits a byte over UART
 *
 * @param c character to be sent
 */
void uart_polling_put_byte (char c){
    struct uart_reg_map *uart = UART2_BASE;

    while (!(uart -> SR & UART_TXE)) {
        // wait
    }
    uart -> DR = c;
    return;
}

/**
 * @brief receives a byte over UART
 */
char uart_polling_get_byte () {
    struct uart_reg_map *uart = UART2_BASE;

    while (!(uart -> SR & UART_RXNE)) {
        // wait
    }
    
    return uart -> DR;
}
