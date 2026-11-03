/**
 * @file svc_handler.c
 *
 * @brief      C half of the SVC handler: decodes the SVC number from the
 *             caller's svc instruction and dispatches to the system call.
 *
 *             The assembly stub passes the user's PSP, which points at the
 *             hardware-stacked exception frame. Arguments come from the
 *             stacked r0-r3, and the return value is written back into the
 *             stacked r0 so the user sees it after exception return.
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#include <stdint.h>
#include <debug.h>
#include <svc_num.h>
#include <syscall.h>
#include <servok.h>

/** @brief Word offsets into the hardware-stacked exception frame */
#define FRAME_R0 0
#define FRAME_R1 1
#define FRAME_R2 2
#define FRAME_PC 6

/** @brief Size of a 16-bit Thumb svc instruction, in bytes */
#define SVC_INSTR_SIZE 2

/** @brief The svc immediate lives in the low byte of the instruction */
#define SVC_IMM_MASK 0xFF

/** @brief Generic failure return for unsupported file operations */
#define SYSCALL_FAIL ((uint32_t)-1)

/**
 * @brief Dispatches a system call made by the user program.
 *
 * @param frame user exception stack frame (the PSP at SVC entry)
 */
void svc_c_handler(uint32_t *frame) {
  /* stacked PC is the instruction after svc; step back to read the svc itself */
  uint16_t svc_instr = *(uint16_t *)(frame[FRAME_PC] - SVC_INSTR_SIZE);
  int svc_number = svc_instr & SVC_IMM_MASK;

  uint32_t arg0 = frame[FRAME_R0];
  uint32_t arg1 = frame[FRAME_R1];
  uint32_t arg2 = frame[FRAME_R2];

  switch (svc_number) {
  case SVC_SBRK:
    frame[FRAME_R0] = (uint32_t)sys_sbrk((int)arg0);
    break;
  case SVC_WRITE:
    frame[FRAME_R0] = sys_write((int)arg0, (char *)arg1, (int)arg2);
    break;
  case SVC_READ:
    frame[FRAME_R0] = sys_read((int)arg0, (char *)arg1, (int)arg2);
    break;
  case SVC_EXIT:
    sys_exit((int)arg0);
    break;
  case SVC_SERVO_ENABLE:
    frame[FRAME_R0] = sys_servo_enable((uint8_t)arg0, (uint8_t)arg1);
    break;
  case SVC_SERVO_SET:
    frame[FRAME_R0] = sys_servo_set((uint8_t)arg0, (uint8_t)arg1);
    break;
  /* no filesystem: newlib probes these, so fail them instead of asserting */
  case SVC_CLOSE:
  case SVC_FSTAT:
  case SVC_ISATTY:
  case SVC_LSEEK:
    frame[FRAME_R0] = SYSCALL_FAIL;
    break;
  default:
    DEBUG_PRINT("Not implemented, svc num %d\n", svc_number);
    ASSERT(0);
  }
}