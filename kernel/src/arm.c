/**
 * @file arm.c
 *
 * @brief      Implementation of asm wrappers
 * @date       5/23/2019
 *
 * @author     Ronit Banerjee <ronitb@andrew.cmu.edu>
 */

#include "arm.h"

#include <rcc.h>
#include <stdint.h>

/* @brief Configuration and Control Register and flags */
//@{
#define CCR ((volatile uint32_t *)0xE000ED14)
#define CCR_STKALIGN (1 << 9)
#define CCR_NONBASETHRDENA 1
//@}
/* @brief Refister to enable/disable fpu */
#define CPACR ((volatile uint32_t *)0xE000ED88)
/* @brief FPU control data */
#define FPCCR ((volatile uint32_t *)0xE000EF34)
/* @brief Interrupt Control and State Register and flags */
//@{
#define ICSR ((volatile uint32_t *)0xE000ED04)
#define ICSR_PENDSVSET (1 << 28)
#define ICSR_PENDSVCLR (1 << 27)
//@}
/* @brief System handler priority register 1 */
#define SHPR1 ((volatile uint32_t *)0xE000ED18)
/* @brief System handler priority register 2 */
#define SHPR2 ((volatile uint32_t *)0xE000ED1C)
/* @brief System handler priority register 3 */
#define SHPR3 ((volatile uint32_t *)0xE000ED20)
/* @brief System handler control and state register and flags */
//@{
#define SHCSR ((volatile uint32_t *)0xE000ED24)
#define SHCSR_USGFAULTENA (1 << 18)
#define SHCSR_SVCALLACT (1 << 7)
//@}

/**
 * @brief      Disables stack alignement.
 */
void init_349(void)
{
  // stack alingment
  *CCR &= ~CCR_STKALIGN;

  // Enable return to thread mode when exceptions are active.
  *CCR |= CCR_NONBASETHRDENA;

  // activate usage fault
  *SHCSR |= SHCSR_USGFAULTENA;

  // Set SVC handler priority to 2
  *SHPR2 |= 0x20000000;
  *SHPR2 &= 0x20FFFFFF;

  // Set SysTick and PendSV priority to 1
  *SHPR3 |= 0x10100000;
  *SHPR3 &= 0x1010FFFF;

  // Set mmfault priority to 1
  *SHPR1 |= 0x00000010;
  *SHPR1 &= 0xFFFFFF10;

  data_sync_barrier();
  instruction_sync_barrier();
}

/**
 * @brief      Enables the fpu.
 */
void enable_fpu(void)
{
  *CPACR |= (0xF << 20);
  *FPCCR |= (0x1 << 31);
  data_sync_barrier();
  instruction_sync_barrier();
}

/**
 * @brief      Pends a pendsv.
 */
void pend_pendsv(void)
{
  *ICSR |= ICSR_PENDSVSET;
}

/**
 * @brief      Clears pendsv.
 */
void clear_pendsv(void)
{
  *ICSR |= ICSR_PENDSVCLR;
}

/**
 * @brief      Check if SVC is active/inactive.
 *
 * @return     0 if inactive, 1 if active
 */
int get_svc_status(void)
{
  if (*SHCSR & SHCSR_SVCALLACT)
    return 1;
  else
    return 0;
}

/**
 * @brief      Sets SVC active/inactive
 */
void set_svc_status(int status)
{
  if (status)
    *SHCSR |= SHCSR_SVCALLACT;
  else
    *SHCSR &= ~SHCSR_SVCALLACT;
}
