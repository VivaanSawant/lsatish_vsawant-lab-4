#include <stdlib.h>
#include <string.h>

#include <gpio.h>

#include <systick.h>
#include <lcd_driver.h>
#include <keypad_driver.h>


#include <timer.h>
#include <nvic.h>

#include "arm.h"
#include "kernel.h"
#include "timer.h"
#include "i2c.h"
#include "lcd_driver.h"
#include "printk.h"
#include "uart_polling.h"
#include "uart.h"
#include "stdint.h"
#include "syscall.h"
#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "arm.h"


#define BAUD_RATE_115200 0x8B

int kernel_main() {
  //Do not remove this function
    init_349();

    while(1){
    }
    

    return 0;
}
