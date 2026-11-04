/**
 * @file   svc_stubs.S
 *
 * @brief  User-side stubs for newlib and 349libc syscalls. Each stub traps
 *         into the kernel with the matching svc number and returns; the
 *         kernel writes the result into the stacked r0, so r0 already holds
 *         the return value when the stub returns.
 */

.cpu cortex-m4
.syntax unified
.section .swi_stub
.thumb

#include "../../kernel/include/svc_num.h"

.thumb_func
.global _start
_start:
  bkpt

.thumb_func
.global _sbrk
_sbrk:
  svc #SVC_SBRK
  bx lr

.thumb_func
.global _write
_write:
  svc #SVC_WRITE
  bx lr

.thumb_func
.global _close
_close:
  svc #SVC_CLOSE
  bx lr

.thumb_func
.global _fstat
_fstat:
  svc #SVC_FSTAT
  bx lr

.thumb_func
.global _isatty
_isatty:
  svc #SVC_ISATTY
  bx lr

.thumb_func
.global _lseek
_lseek:
  svc #SVC_LSEEK
  bx lr

.thumb_func
.global _read
_read:
  svc #SVC_READ
  bx lr

.thumb_func
.global _exit
_exit:
  svc #SVC_EXIT
  bx lr

/* No kernel handler for these yet; fail without trapping so the SVC
 * dispatcher's default case is never hit. */
.thumb_func
.global _gettimeofday
_gettimeofday:
  mvn r0, #0           @ return -1
  bx lr

.thumb_func
.global _times
_times:
  mvn r0, #0           @ return -1
  bx lr

.thumb_func
.global _getpid
_getpid:
  movs r0, #1          @ single process, so pid 1
  bx lr

.thumb_func
.global _kill
_kill:
  mvn r0, #0           @ return -1
  bx lr

.thumb_func
.global servo_enable
servo_enable:
  svc #SVC_SERVO_ENABLE
  bx lr

.thumb_func
.global servo_set
servo_set:
  svc #SVC_SERVO_SET
  bx lr

/* Lab 4 thread/mutex stubs: still unimplemented */

.type thread_init, %function
.global thread_init
thread_init:
  bkpt

.type thread_create, %function
.global thread_create
thread_create:
  bkpt

.type thread_kill, %function
.global thread_kill
thread_kill:
  bkpt

.type scheduler_start, %function
.global scheduler_start
scheduler_start:
  bkpt

.type mutex_init, %function
.global mutex_init
mutex_init:
  bkpt

.type mutex_lock, %function
.global mutex_lock
mutex_lock:
  bkpt

.type mutex_unlock, %function
.global mutex_unlock
mutex_unlock:
  bkpt

.type wait_until_next_period, %function
.global wait_until_next_period
wait_until_next_period:
  bkpt

.global get_time
get_time:
  bkpt

.global get_priority
get_priority:
  bkpt

.global thread_time
thread_time:
  bkpt

.thumb_func
.global idle_default
idle_default:
  bkpt