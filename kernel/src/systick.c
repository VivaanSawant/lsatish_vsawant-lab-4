/**
* @file
*
* @brief
*
* @date
*
* @author
*/

#include <unistd.h>
#include <systick.h>
#include <syscall_thread.h>

/** @brief SysTick control/status register */
#define STK_CTRL   (*(volatile uint32_t *)0xE000E010)
/** @brief SysTick reload value register */
#define STK_LOAD   (*(volatile uint32_t *)0xE000E014)
/** @brief SysTick current value register */
#define STK_VAL    (*(volatile uint32_t *)0xE000E018)

#define STK_CTRL_ENABLE     (1 << 0)  /**< start the counter */
#define STK_CTRL_TICKINT    (1 << 1)  /**< interrupt when it hits 0 */
#define STK_CTRL_CLKSOURCE  (1 << 2)  /**< use processor clock (16 MHz) */

/** @brief Ticks since boot. volatile: changed by the ISR */
static volatile uint32_t ticks = 0;


void systick_init(uint32_t frequency) {
    STK_LOAD = (BASE_FREQ / frequency) - 1;  // count includes 0
    STK_VAL = 0; // clear current count so first tick is a full period
    STK_CTRL = STK_CTRL_ENABLE | STK_CTRL_TICKINT | STK_CTRL_CLKSOURCE;
}

void systick_delay(uint32_t delay_ticks) { // renamed because of conflicting name
    uint32_t start = ticks;
    while (ticks - start < delay_ticks);
}

uint32_t systick_get_ticks() {
    return ticks;
}

void systick_c_handler() {
    ticks++;
    scheduler_tick(); // no-op until scheduler_start
}