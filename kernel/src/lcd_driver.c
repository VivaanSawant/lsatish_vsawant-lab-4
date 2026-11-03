/**
 * @file lcd_driver.c
 *
 * @brief Driver for the 16x2 HD44780U LCD behind a PCF8574 I2C extender.
 *
 * Each PCF8574 byte maps to the LCD pins as:
 *     P7-P4 = D7-D4, P3 = backlight, P2 = E, P1 = RW, P0 = RS
 * RW (P1) is always left at 0 since we only ever write to the LCD.
 * The LCD runs in 4-bit mode, so every 8-bit instruction or character is
 * sent as two nibbles (high first), each written once with E=1 and once
 * with E=0, all inside a single I2C transmission.
 *
 * All waits use systick_delay, so systick_init() must run before
 * lcd_driver_init().
 */

#include <unistd.h>

#include <i2c.h>
#include <lcd_driver.h>
#include <systick.h>

// pcf address
#define LCD_I2C_ADDR 0x27

// 100 khz
#define LCD_I2C_CLK_KHZ 100

// pinbit
#define LCD_RS (1 << 0) /**< 0 = instruction, 1 = data */
#define LCD_E (1 << 2) /**< enable strobe */
#define LCD_BACKLIGHT (1 << 3) /**< always 1 */
#define LCD_NIBBLE_SHIFT 4     /**< data nibble sits on P7-P4 */
#define LCD_NIBBLE_MASK  0x0F

/** @brief Bytes per I2C transmission: one nibble = 2 bytes (E high, E low) */
#define BYTES_PER_NIBBLE 2
#define BYTES_PER_BYTE   (2 * BYTES_PER_NIBBLE)

// instructions
#define LCD_CMD_CLEAR         0x01
#define LCD_CMD_ENTRY_MODE    0x04
#define LCD_CMD_DISPLAY_CTRL  0x08
#define LCD_CMD_FUNCTION_SET  0x20
#define LCD_CMD_SET_DDRAM     0x80

/** @brief Entry mode flag: increment cursor after each character, no shift */
#define LCD_ENTRY_INCREMENT (1 << 1)

/** @brief Function set flags: 2 lines (N), 5x8 font (F=0), 4-bit (DL=0) */
#define LCD_FUNC_2_LINES (1 << 3)

/** @brief Display control flags */
#define LCD_DISPLAY_ON (1 << 2)
#define LCD_CURSOR_ON  (1 << 1)
#define LCD_BLINK_ON   (1 << 0)

/** @brief Upper nibbles for the Figure 24 4-bit init sequence */
#define LCD_INIT_8BIT_NIBBLE 0x3
#define LCD_INIT_4BIT_NIBBLE 0x2

/** @brief DDRAM start address of each row (p. 11: line 2 starts at 0x40) */
#define LCD_ROW0_ADDR 0x00
#define LCD_ROW1_ADDR 0x40

/** @brief Figure 24 waits (p. 46), rounded up, in ms */
#define LCD_POWER_ON_DELAY_MS  50 /**< > 40 ms after VCC rises */
#define LCD_INIT_FIRST_WAIT_MS 5  /**< > 4.1 ms */
#define LCD_INIT_SECOND_WAIT_MS 1 /**< > 100 us */

/** @brief Handout requires a 2 s wait after every clear instruction */
#define LCD_CLEAR_DELAY_MS 2000


//packs
static void pack_nibble(uint8_t *out, uint8_t nibble, uint8_t rs) {
    uint8_t pins = ((nibble & LCD_NIBBLE_MASK) << LCD_NIBBLE_SHIFT) | LCD_BACKLIGHT | rs;

    out[0] = pins | LCD_E;  // E high with data already valid
    out[1] = pins;          // E low: LCD latches the nibble
}


// snes a nibble
static void lcd_send_nibble(uint8_t nibble) {
    uint8_t buf[BYTES_PER_NIBBLE];

    pack_nibble(buf, nibble, 0);

    i2c_master_start();
    i2c_master_write(buf, BYTES_PER_NIBBLE, LCD_I2C_ADDR);
    i2c_master_stop();
}

/**
 * @brief Sends a full 8-bit instruction or character, high nibble first.
 *
 * No delay is needed afterwards for anything but clear: the next I2C
 * transmission takes far longer than the 37 us instruction time (Table 6).
 *
 * @param value byte to send
 * @param rs    LCD_RS for a character, 0 for an instruction
 */
static void lcd_send_byte(uint8_t value, uint8_t rs) {
    uint8_t buf[BYTES_PER_BYTE];

    pack_nibble(&buf[0], value >> LCD_NIBBLE_SHIFT, rs);
    pack_nibble(&buf[BYTES_PER_NIBBLE], value, rs);

    i2c_master_start();
    i2c_master_write(buf, BYTES_PER_BYTE, LCD_I2C_ADDR);
    i2c_master_stop();
}

/**
 * @brief Initializes I2C and runs the HD44780U 4-bit init sequence
 *        (Figure 24, p. 46), leaving a 2-line display with a blinking cursor.
 */
void lcd_driver_init() {
    i2c_master_init(LCD_I2C_CLK_KHZ);

    systick_delay(LCD_POWER_ON_DELAY_MS);

    // Three "8-bit" function sets, then switch to 4-bit. Only D7-D4 are
    // wired, so each of these is a single nibble.
    lcd_send_nibble(LCD_INIT_8BIT_NIBBLE);
    systick_delay(LCD_INIT_FIRST_WAIT_MS);
    lcd_send_nibble(LCD_INIT_8BIT_NIBBLE);
    systick_delay(LCD_INIT_SECOND_WAIT_MS);
    lcd_send_nibble(LCD_INIT_8BIT_NIBBLE);
    lcd_send_nibble(LCD_INIT_4BIT_NIBBLE);

    // Rest of Figure 24: function set, display off, clear, entry mode
    lcd_send_byte(LCD_CMD_FUNCTION_SET | LCD_FUNC_2_LINES, 0);
    lcd_send_byte(LCD_CMD_DISPLAY_CTRL, 0);
    lcd_clear();
    lcd_send_byte(LCD_CMD_ENTRY_MODE | LCD_ENTRY_INCREMENT, 0);

    // Figure 24 leaves the display off; turn it on with a blinking cursor
    lcd_send_byte(LCD_CMD_DISPLAY_CTRL | LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_BLINK_ON, 0);
}

/**
 * @brief Prints a null-terminated string starting at the current cursor.
 *
 * @param input string to print
 */
void lcd_print(char *input){
    while (*input != '\0') {
        lcd_send_byte((uint8_t) *input, LCD_RS);
        input++;
    }
}

/**
 * @brief Moves the cursor to the given position.
 *
 * @param row 0 or 1
 * @param col 0 to 15
 */
void lcd_set_cursor(uint8_t row, uint8_t col){
    uint8_t row_addr = (row == 0) ? LCD_ROW0_ADDR : LCD_ROW1_ADDR;

    lcd_send_byte(LCD_CMD_SET_DDRAM | (row_addr + col), 0);
}

/**
 * @brief Clears the display and returns the cursor to row 0, column 0.
 *
 * Blocks for 2 s afterwards, as the handout requires.
 */
void lcd_clear() {
    lcd_send_byte(LCD_CMD_CLEAR, 0);
    systick_delay(LCD_CLEAR_DELAY_MS);
}