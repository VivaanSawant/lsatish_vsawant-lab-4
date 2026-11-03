/** @file rcc.h
 *
 * @brief      MMIO defs for rcc clock register.
 *
 * @date       3/11/2019
 *
 * @author     Kevin DeVincentis <kdvince@andrew.cmu.edu>
 */

#ifndef _RCC_H_
#define _RCC_H_

/**
 * @brief      The Reset and Clock Control register (RCC)
 */
struct rcc_reg_map  {
  volatile unsigned long cr;           /** @brief 0  - Clock control */
  volatile unsigned long pll_cfgr;     /** @brief 4  - PLL Config    */
  volatile unsigned long cfgr;         /** @brief 8  - Clock config */
  volatile unsigned long cir;          /** @brief C  - Clock interrupt */
  volatile unsigned long ahb1_rstr;    /** @brief 10 - AHB1 Peripheral Reset */
  volatile unsigned long ahb2_rstr;    /** @brief 14 - AHB2 Peripheral Reset */
  volatile unsigned long reserved_1;   /** @brief 18 */
  volatile unsigned long reserved_2;   /** @brief 1C */
  volatile unsigned long apb1_rstr;    /** @brief 20 - APB1 peripheral reset */
  volatile unsigned long apb2_rstr;    /** @brief 24 - APB2 peripheral reset */
  volatile unsigned long reserved_3;   /** @brief 28 */
  volatile unsigned long reserved_4;   /** @brief 2C */
  volatile unsigned long ahb1_enr;     /** @brief 30 - AHB1 Peripheral Clock Enable */
  volatile unsigned long ahb2_enr;     /** @brief 34 - AHB2 Peripheral Clock Enable */
  volatile unsigned long reserved_5;   /** @brief 38 */
  volatile unsigned long reserved_6;   /** @brief 3C */
  volatile unsigned long apb1_enr;     /** @brief 40 - APB2 peripheral clock enable */
  volatile unsigned long apb2_enr;     /** @brief 44 - APB1 peripheral clock enable */
  volatile unsigned long reserved_7;   /** @brief 48 */
  volatile unsigned long reserved_8;   /** @brief 4C */
  volatile unsigned long ahb1_lpenr;   /** @brief 50 - AHB1 Peripheral Clock Low Power Enable */
  volatile unsigned long ahb2_lpenr;   /** @brief 54 - AHB1 Peripheral Clock Low Power Enable */
  volatile unsigned long reserved_9;   /** @brief 58 */
  volatile unsigned long reserved_10;  /** @brief 5C */
  volatile unsigned long apb1_lpenr;   /** @brief 60 - APB2 peripheral clock low power enable */
  volatile unsigned long apb2_lpenr;   /** @brief 64 - APB1 peripheral clock low power enable */
  volatile unsigned long reserved_11;  /** @brief 68 */
  volatile unsigned long reserved_12;  /** @brief 6C */
  volatile unsigned long bdcr;         /** @brief 70 - Backup domain control register */
  volatile unsigned long csr;          /** @brief 74 - Control/status register */
  volatile unsigned long reserved_13;  /** @brief 78 */
  volatile unsigned long reserved_14;  /** @brief 7C */
  volatile unsigned long sscgr;        /** @brief 80 - spread spectrum clock generation */
  volatile unsigned long pll_I2Scfgr;  /** @brief 84 - PLLI2S configuration */
  volatile unsigned long reserved_15;  /** @brief 88 */
  volatile unsigned long dckcfgr;      /** @brief 8C - Dedicated Clocks Configuration*/
};

#define RCC_BASE ((struct rcc_reg_map *) 0x40023800)

#define RCC_APB1_UART_EN (1 << 17)

#define RCC_APB2_ADC1_EN  (1 << 8)

#define RCC_APB1_I2C1_EN (1 << 21)
#define RCC_APB2_SPI1_EN (1 << 12)
#define RCC_APB1_SPI2_EN (1 << 14)

#define RCC_APB2_SYSCFG_EN (1 << 14)


#endif /* _RCC_H_ */
