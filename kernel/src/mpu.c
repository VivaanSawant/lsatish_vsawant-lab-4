/** @file    mpu.c
 *
 *  @brief   MPU interface implementation for thread-wise
 *           memory protection.
 *
 *  @date    30 Mar 2020
 *
 *  @author  Benjamin Huang <zemingbh@andrew.cmu.edu>
 */

#include "arm.h"
#include "debug.h"
#include "printk.h"
#include "syscall.h"
#include "mpu.h"
#include "unistd.h"

/** @brief Macro for unused variables*/
#define UNUSED __attribute__((unused))

/**
 * @struct mpu_t
 * @brief  MPU MMIO register layout.
 */
typedef struct
{
  /**@brief provides information about the MPU */
  volatile uint32_t TYPER;
  /**@brief MPU enable/disable and background region control. */
  volatile uint32_t CTRL;
  /** @brief Select which MPU region to be configured*/
  volatile uint32_t RNR;
  /**@brief Defines base address of a MPU region */
  volatile uint32_t RBAR;
  /**@brief Defines size and attribues of a MPU region*/
  volatile uint32_t RASR;

  /**@brief Field aliases. */
  //@{
  volatile uint32_t RBAR_A1;
  volatile uint32_t RASR_A1;
  volatile uint32_t RBAR_A2;
  volatile uint32_t RASR_A2;
  volatile uint32_t RBAR_A3;
  volatile uint32_t RASR_A3;
  //@}
} mpu_t;

/**
 * @struct system_control_block_t
 * @brief  System control block register layout.
 */
typedef struct
{
  /**@brief System handler control and state register.*/
  volatile uint32_t SHCRS;
  /**@brief Configurable fault status register.*/
  volatile uint32_t CFSR;
  /**@brief HardFault Status Register */
  volatile uint32_t HFSR;
  /**@brief Hint information of debug events.*/
  volatile uint32_t DFSR;
  /**@brief Addr value of memfault*/
  volatile uint32_t MMFAR;
  /**@brief Addr of bus fault */
  volatile uint32_t BFAR;
  /**@brief Auxilliary fault status register.*/
  volatile uint32_t AFSR;
} system_control_block_t;

/**@brief MPU base address.*/
#define MPU_BASE ((mpu_t *)0xE000ED90);

/** @brief MPU CTRL register flags. */
//@{
#define CTRL_ENABLE_BG_REGION (1 << 2)
#define CTRL_ENABLE_PROTECTION (1 << 0)
//@}

/** @brief MPU RNR register flags. */
#define RNR_REGION (0xFF)
/** @brief Maximum region number. */
#define REGION_NUMBER_MAX 7

/** @brief MPU RBAR register flags. */
//@{
#define RBAR_VALID (1 << 4)
#define RBAR_REGION (0xF)
//@}

/** @brief MPU RASR register masks. */
//@{
#define RASR_XN (1 << 28)
#define RASR_AP_KERN (1 << 26)
#define RASR_AP_USER (1 << 25 | 1 << 24)
#define RASR_SIZE (0b111110)
#define RASR_ENABLE (1 << 0)
//@}

/** @brief Memory Management Fault Enable bit */
#define MM_FAULT_ENABLE (1 << 16)

/** @brief MPU RASR AP user mode encoding. */
//@{
#define RASR_AP_USER_READ_ONLY (0b10 << 24)
#define RASR_AP_USER_READ_WRITE (0b11 << 24)
//@}

/**@brief Systen control block MMIO location.*/
#define SCB_BASE ((volatile system_control_block_t *)0xE000ED24)

/**@brief Stacking error.*/
#define MSTKERR 0x1 << 4
/**@brief Unstacking error.*/
#define MUNSTKERR 0x1 << 3
/**@brief Data access error.*/
#define DACCVIOL 0x1 << 1
/**@brief Instruction access error.*/
#define IACCVIOL 0x1 << 0
/**@brief Indicates the MMFAR is valid.*/
#define MMARVALID 0x1 << 7

/**
 * @brief Handles memory protection and checks the currently running thread's psp and then kills the currently running thread
 *
 * @param psp the PSP of the current thread
 * @return does not return (void)
 **/
void mm_c_handler(void *psp)
{

  (void)psp;
}

/**
 * @brief  Enables a memory protection region. Regions must be aligned!
 *
 * @param  region_number      The region number to enable.
 * @param  base_address       The region's base (starting) address.
 * @param  size_log2          log[2] of the region size.
 * @param  execute            1 if the region should be executable by the user.
 *                            0 otherwise.
 * @param  user_write_access  1 if the user should have write access, 0 if
 *                            read-only
 *
 * @return 0 on success, -1 on failure
 */
int mm_region_enable(
    uint32_t region_number,
    void *base_address,
    uint8_t size_log2,
    int execute,
    int user_write_access)
{
  (void)region_number;
  (void)base_address;
  (void)size_log2;
  (void)execute;
  (void)user_write_access;

  return 0;
}

/**
 * @brief  Disables a memory protection region.
 *
 * @param  region_number      The region number to disable.
 */
void mm_region_disable(uint32_t region_number)
{
  mpu_t *mpu = MPU_BASE;
  mpu->RNR = region_number & RNR_REGION;
  mpu->RASR &= ~RASR_ENABLE;
}

/**
 * @brief  Returns ceiling (log_2 n).
 */
uint32_t mm_log2ceil_size(uint32_t n)
{
  uint32_t ret = 0;
  while (n > (1U << ret))
  {
    ret++;
  }
  return ret;
}

/**
 * @brief  To call in kernel_main and initialize memory protection
 */
void mm_init()
{
  system_control_block_t *scb = (system_control_block_t *)SCB_BASE;
  // enable memory protection faults
  scb->SHCRS |= MM_FAULT_ENABLE;

  mpu_t *mpu = MPU_BASE;
  // enable access to background region
  mpu->CTRL |= CTRL_ENABLE_BG_REGION;
  mpu->CTRL |= CTRL_ENABLE_PROTECTION;
}
