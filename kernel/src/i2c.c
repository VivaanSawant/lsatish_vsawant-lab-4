/**
 * @file i2c.c
 *
 * @brief Polled I2C master driver for the STM32F401 (I2C1, standard mode).
 *
 * Used by the LCD driver to talk to the PCF8574 extender board. I2C1 is
 * wired to Arduino pins D15 (SCL, PB8) and D14 (SDA, PB9).
 *
 * Intended call pattern for one transmission:
 *     i2c_master_start();
 *     i2c_master_write(buf, len, slave_addr);
 *     i2c_master_stop();
 */

#include <unistd.h>

#include <gpio.h>
#include <i2c.h>
#include <rcc.h>

/** @brief The I2C register map. */
struct i2c_reg_map {
    volatile uint32_t CR1;   /**< Control Register 1 */
    volatile uint32_t CR2;   /**< Control Register 2 */
    volatile uint32_t OAR1;  /**< Own Address Register 1 */
    volatile uint32_t OAR2;  /**< Own Address Register 2 */
    volatile uint32_t DR;    /**< Data Register */
    volatile uint32_t SR1;   /**< Status Register 1 */
    volatile uint32_t SR2;   /**< Status Register 2 */
    volatile uint32_t CCR;   /**< Clock Control Register */
    volatile uint32_t TRISE; /**< Maximum Rise Time Register */
    volatile uint32_t FLTR;  /**< Noise Filter Register */
};

/** @brief Base address for I2C1 */
#define I2C1_BASE (struct i2c_reg_map *) 0x40005400

/** @brief RCC_APB1ENR clock enable bit for I2C1 */
#define I2C1_CLOCK_EN (1 << 21)

/** @brief Pins: SCL = D15 = PB8, SDA = D14 = PB9 */
#define I2C_SCL_PORT GPIO_B
#define I2C_SCL_PIN  8
#define I2C_SDA_PORT GPIO_B
#define I2C_SDA_PIN  9

/** @brief CR1 bits */
#define I2C_CR1_PE    (1 << 0)
#define I2C_CR1_START (1 << 8)
#define I2C_CR1_STOP  (1 << 9)
#define I2C_CR1_ACK   (1 << 10)
#define I2C_CR1_POS   (1 << 11)

/** @brief CR2 FREQ field: peripheral (APB1) clock in MHz */
#define I2C_CR2_FREQ_MASK 0x3F

/** @brief SR1 bits */
#define I2C_SR1_SB   (1 << 0)  /**< Start bit sent (EV5) */
#define I2C_SR1_ADDR (1 << 1)  /**< Address sent and ACKed (EV6) */
#define I2C_SR1_BTF  (1 << 2)  /**< Byte transfer finished (EV8_2) */
#define I2C_SR1_RXNE (1 << 6)  /**< Data register not empty (EV7) */
#define I2C_SR1_TXE  (1 << 7)  /**< Data register empty (EV8_1, EV8) */
#define I2C_SR1_AF   (1 << 10) /**< Acknowledge failure */

/** @brief APB1 clock after reset (HSI, no prescalers) */
#define APB_CLOCK_HZ 16000000
#define APB_CLOCK_MHZ 16
#define HZ_PER_KHZ 1000

/** @brief Standard-mode SCL has equal high and low time, so 2 * CCR periods per SCL cycle */
#define SM_CCR_PERIODS_PER_SCL_CYCLE 2

/** @brief Address byte layout: 7-bit address in bits 7:1, R/W in bit 0 */
#define I2C_ADDR_SHIFT 1
#define I2C_WRITE_BIT  0
#define I2C_READ_BIT   1

/**
 * @brief Waits until the given SR1 flag is set, or the slave fails to ACK.
 *
 * @param flag SR1 bit to wait for
 * @return 0 once the flag is set, -1 if an acknowledge failure occurred
 *         (AF is cleared before returning; caller should still send a Stop)
 */
static int wait_sr1_flag(uint32_t flag) {
    struct i2c_reg_map *i2c = I2C1_BASE;

    while (1) {
        uint32_t status = i2c -> SR1;

        if (status & flag) {
            return 0;
        }
        if (status & I2C_SR1_AF) {
            i2c -> SR1 &= ~I2C_SR1_AF; // AF is cleared by writing 0
            return -1;
        }
    }
}

/**
 * @brief Clears ADDR (EV6) by reading SR1 then SR2.
 */
static void clear_addr_flag() {
    struct i2c_reg_map *i2c = I2C1_BASE;

    volatile uint32_t status_read = i2c -> SR1;
    status_read = i2c -> SR2;
    (void) status_read;
}

/**
 * @brief initializes I2C1 as a 7-bit addressing master in standard mode
 *
 * @param clk desired SCL frequency in kHz (100 for standard mode)
 */
void i2c_master_init(uint16_t clk){
    struct i2c_reg_map *i2c = I2C1_BASE;

    // SCL and SDA are open-drain; external pull-ups on the extender board
    gpio_init(I2C_SCL_PORT, I2C_SCL_PIN, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, PUPD_NONE, ALT4);
    gpio_init(I2C_SDA_PORT, I2C_SDA_PIN, MODE_ALT, OUTPUT_OPEN_DRAIN, OUTPUT_SPEED_LOW, PUPD_NONE, ALT4);

    // Setting clock enable bit to 1
    struct rcc_reg_map *rccAPB = RCC_BASE;
    rccAPB -> apb1_enr |= I2C1_CLOCK_EN;

    // Errata 2.1.6: a dummy read gives the peripheral time to come out of reset
    // before its registers are touched
    volatile uint32_t clock_settle = rccAPB -> apb1_enr;
    (void) clock_settle;

    // Peripheral clock frequency, used internally for setup/hold times
    i2c -> CR2 = (i2c -> CR2 & ~I2C_CR2_FREQ_MASK) | APB_CLOCK_MHZ;

    // Thigh = Tlow = CCR * TPCLK1, so CCR = fPCLK1 / (2 * fSCL)
    i2c -> CCR = APB_CLOCK_HZ / (SM_CCR_PERIODS_PER_SCL_CYCLE * clk * HZ_PER_KHZ);

    // Standard mode max SCL rise time is 1000 ns; TRISE = (1000 ns / TPCLK1) + 1
    i2c -> TRISE = APB_CLOCK_MHZ + 1;

    // CCR and TRISE may only be written while PE = 0, so enable last
    i2c -> CR1 |= I2C_CR1_PE;

    return;
}

/**
 * @brief generates a Start condition and waits for it to be sent (EV5)
 */
void i2c_master_start() {
    struct i2c_reg_map *i2c = I2C1_BASE;

    i2c -> CR1 |= I2C_CR1_START;

    // SB is cleared by reading SR1 (done by this poll) then writing DR
    while (!(i2c -> SR1 & I2C_SR1_SB)) {
        // wait
    }

    return;
}

/**
 * @brief generates a Stop condition and waits for the bus to be released
 *
 * Call after i2c_master_write has returned (successfully or not).
 */
void i2c_master_stop() {
    struct i2c_reg_map *i2c = I2C1_BASE;

    i2c -> CR1 |= I2C_CR1_STOP;

    // Hardware clears STOP once the Stop is on the bus; CR1 must not be
    // written before then
    while (i2c -> CR1 & I2C_CR1_STOP) {
        // wait
    }

    return;
}

/**
 * @brief writes bytes to a slave. Must be called after i2c_master_start.
 *
 * Sends the address (EV5 -> EV6), then each byte (EV8_1, EV8), and waits for
 * the last byte to finish shifting out (EV8_2). Does not send the Stop.
 *
 * @param buf        bytes to send
 * @param len        number of bytes in buf
 * @param slave_addr 7-bit slave address (not pre-shifted)
 * @return 0 on success, -1 if the slave did not acknowledge
 */
int i2c_master_write(uint8_t *buf, uint16_t len, uint8_t slave_addr){
    struct i2c_reg_map *i2c = I2C1_BASE;

    // Writing DR after reading SR1 (in i2c_master_start) clears SB
    i2c -> DR = (slave_addr << I2C_ADDR_SHIFT) | I2C_WRITE_BIT;

    if (wait_sr1_flag(I2C_SR1_ADDR) < 0) {
        return -1;
    }

    // ADDR is cleared by reading SR1 then SR2
    clear_addr_flag();

    // No data: BTF would never set, so stop here
    if (len == 0) {
        return 0;
    }

    for (uint16_t i = 0; i < len; i++) {
        if (wait_sr1_flag(I2C_SR1_TXE) < 0) {
            return -1;
        }
        i2c -> DR = buf[i];
    }

    // Last byte is fully shifted out once BTF is set
    if (wait_sr1_flag(I2C_SR1_BTF) < 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief reads bytes from a slave. Must be called after i2c_master_start.
 *
 * Unlike write, this ends the transmission itself: the Stop has to be
 * requested before the last byte arrives, so do NOT call i2c_master_stop
 * afterwards. Uses the 1-byte, 2-byte, and N>2-byte closing sequences
 * from the reference manual so the last byte is NACKed.
 *
 * @param buf        buffer to fill, at least len bytes
 * @param len        number of bytes to read
 * @param slave_addr 7-bit slave address (not pre-shifted)
 * @return 0 on success, -1 if the slave did not acknowledge its address
 */
int i2c_master_read(uint8_t *buf, uint16_t len, uint8_t slave_addr){
    struct i2c_reg_map *i2c = I2C1_BASE;

    if (len == 0) {
        i2c -> CR1 |= I2C_CR1_STOP;
        while (i2c -> CR1 & I2C_CR1_STOP) {
            // wait
        }
        return 0;
    }

    // ACK every byte except the last; the closing sequences below turn it off
    i2c -> CR1 |= I2C_CR1_ACK;

    // Writing DR after reading SR1 (in i2c_master_start) clears SB
    i2c -> DR = (slave_addr << I2C_ADDR_SHIFT) | I2C_READ_BIT;

    if (wait_sr1_flag(I2C_SR1_ADDR) < 0) {
        i2c -> CR1 &= ~I2C_CR1_ACK;
        i2c -> CR1 |= I2C_CR1_STOP;
        while (i2c -> CR1 & I2C_CR1_STOP) {
            // wait
        }
        return -1;
    }

    if (len == 1) {
        // NACK must be set before ADDR is cleared, then Stop right after
        i2c -> CR1 &= ~I2C_CR1_ACK;
        clear_addr_flag();
        i2c -> CR1 |= I2C_CR1_STOP;

        while (!(i2c -> SR1 & I2C_SR1_RXNE)) {
            // wait
        }
        buf[0] = i2c -> DR;
    } else if (len == 2) {
        // POS makes the NACK apply to the second byte instead of the first
        i2c -> CR1 &= ~I2C_CR1_ACK;
        i2c -> CR1 |= I2C_CR1_POS;
        clear_addr_flag();

        // Byte 1 in DR, byte 2 in shift register
        while (!(i2c -> SR1 & I2C_SR1_BTF)) {
            // wait
        }
        i2c -> CR1 |= I2C_CR1_STOP;
        buf[0] = i2c -> DR;

        // Byte 2 moves from the shift register into DR
        while (!(i2c -> SR1 & I2C_SR1_RXNE)) {
            // wait
        }
        buf[1] = i2c -> DR;

        i2c -> CR1 &= ~I2C_CR1_POS;
    } else {
        clear_addr_flag();

        // Read normally until three bytes are left
        uint16_t i;
        for (i = 0; i < len - 3; i++) {
            while (!(i2c -> SR1 & I2C_SR1_RXNE)) {
                // wait
            }
            buf[i] = i2c -> DR;
        }

        // Byte N-2 in DR, N-1 in shift register
        while (!(i2c -> SR1 & I2C_SR1_BTF)) {
            // wait
        }
        i2c -> CR1 &= ~I2C_CR1_ACK;
        buf[i++] = i2c -> DR;

        // Byte N-1 in DR, N in shift register
        while (!(i2c -> SR1 & I2C_SR1_BTF)) {
            // wait
        }
        i2c -> CR1 |= I2C_CR1_STOP;
        buf[i++] = i2c -> DR;

        // Byte N moves from the shift register into DR
        while (!(i2c -> SR1 & I2C_SR1_RXNE)) {
            // wait
        }
        buf[i] = i2c -> DR;
    }

    // Hardware clears STOP once the Stop is on the bus
    while (i2c -> CR1 & I2C_CR1_STOP) {
        // wait
    }

    return 0;
}