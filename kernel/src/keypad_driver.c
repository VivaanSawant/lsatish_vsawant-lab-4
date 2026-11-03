#include <unistd.h>

#include <gpio.h>
#include <keypad_driver.h>
#include <systick.h>

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 3

/** @brief Spin iterations to let a column's level settle before reading rows */
#define KEYPAD_COL_SETTLE_LOOPS 100

/** @brief Raw reading must hold this long before it counts as a state change */
#define DEBOUNCE_MS 20

//rows pinouts
#define KEYPAD_ROW1_PORT GPIO_A
#define KEYPAD_ROW1_PIN  0    // A0
#define KEYPAD_ROW2_PORT GPIO_A
#define KEYPAD_ROW2_PIN  1    // A1
#define KEYPAD_ROW3_PORT GPIO_A
#define KEYPAD_ROW3_PIN  4    // A2
#define KEYPAD_ROW4_PORT GPIO_B
#define KEYPAD_ROW4_PIN  0    // A3

//cols pinouts
#define KEYPAD_COL1_PORT GPIO_C
#define KEYPAD_COL1_PIN  1    // A4
#define KEYPAD_COL2_PORT GPIO_C
#define KEYPAD_COL2_PIN  0    // A5
#define KEYPAD_COL3_PORT GPIO_C
#define KEYPAD_COL3_PIN  7    // D9

static const gpio_port row_ports[KEYPAD_ROWS] = {
    KEYPAD_ROW1_PORT, KEYPAD_ROW2_PORT, KEYPAD_ROW3_PORT, KEYPAD_ROW4_PORT
};
static const unsigned int row_pins[KEYPAD_ROWS] = {
    KEYPAD_ROW1_PIN, KEYPAD_ROW2_PIN, KEYPAD_ROW3_PIN, KEYPAD_ROW4_PIN
};
static const gpio_port col_ports[KEYPAD_COLS] = {
    KEYPAD_COL1_PORT, KEYPAD_COL2_PORT, KEYPAD_COL3_PORT
};
static const unsigned int col_pins[KEYPAD_COLS] = {
    KEYPAD_COL1_PIN, KEYPAD_COL2_PIN, KEYPAD_COL3_PIN
};

//key layout
static const char keymap[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

// selected col -> low, all others high
static void set_col(int col) {
    for (int c = 0; c < KEYPAD_COLS; c++) {
        if (c == col) {
            gpio_clr(col_ports[c], col_pins[c]);
        } else {
            gpio_set(col_ports[c], col_pins[c]);
        }
    }
}

// return 1 if row is low
static int read_row(int row) {
    return gpio_read(row_ports[row], row_pins[row]) == 0;
}


void keypad_init() {
    // all rows -> high
    for (int r = 0; r < KEYPAD_ROWS; r++) {
        gpio_init(row_ports[r], row_pins[r], MODE_INPUT, OUTPUT_PUSH_PULL,
                  OUTPUT_SPEED_LOW, PUPD_PULL_UP, ALT0);
    }

    // all cols -> high
    for (int c = 0; c < KEYPAD_COLS; c++) {
        gpio_init(col_ports[c], col_pins[c], MODE_GP_OUTPUT, OUTPUT_PUSH_PULL,
                  OUTPUT_SPEED_LOW, PUPD_NONE, ALT0);
        gpio_set(col_ports[c], col_pins[c]);
    }
}

char keypad_read() {
    static char stable_key = '\0';        // last debounced state
    static char raw_key = '\0';           // last raw scan result
    static uint32_t raw_changed_at = 0;   // tick when raw_key last changed
    char current_key = '\0';

    // look for pressed key
    for (int c = 0; c < KEYPAD_COLS; c++) {
        set_col(c);
        for (volatile int i = 0; i < KEYPAD_COL_SETTLE_LOOPS; i++); // let signal chill
        for (int r = 0; r < KEYPAD_ROWS; r++) {
            if (read_row(r)) {
                current_key = keymap[r][c];
            }
        }
    }

    uint32_t now = systick_get_ticks();

    // raw reading moved: restart the stability window
    if (current_key != raw_key) {
        raw_key = current_key;
        raw_changed_at = now;
        return '\0';
    }

    // not stable long enough, or no actual state change
    if (now - raw_changed_at < DEBOUNCE_MS || raw_key == stable_key) {
        return '\0';
    }

    stable_key = raw_key;
    return stable_key;   // non-null only on a not-pressed -> pressed transition
}