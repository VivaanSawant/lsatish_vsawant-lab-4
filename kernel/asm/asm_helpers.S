/**
 * @file asm_helpers.S
 *
 * @brief      Assembly helpers for the kernel.
 *
 * @date       2026-10-26
 *
 * @author     Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

.cpu cortex-m4
.syntax unified
.thumb

.section .text

/*
 * enter_user_mode: sets up the user stack and jumps to the user runtime.
 * _crt0 writes CONTROL = 3 (unprivileged + PSP), so PSP must already point
 * at the top of the user stack before we branch. Never returns.
 */
.thumb_func
.global enter_user_mode
enter_user_mode:
  ldr r0, =__psp_stack_top   @ top of the user (process) stack from the linker script
  msr psp, r0                @ sp becomes this once _crt0 switches to PSP
  b _crt0                    @ drop privileges and run main; no return