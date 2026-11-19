/** @file   syscall_thread.c
 *
 *  @brief  Thread system calls, PendSV context switching, the
 *          rate-monotonic scheduler, and HLP mutexes.
 *
 *          TCBs are statically allocated and indexed by static priority
 *          (0 to MAX_USER_THREADS - 1), followed by the idle thread and the
 *          default (main) thread. A thread's context is saved on its own
 *          kernel stack by psv_asm_handler; the TCB only keeps that stack
 *          pointer and the SVC-active state.
 *
 *  @date   2026-11-03
 *
 *  @author Vivaan Sawant (vsawant@andrew.cmu.edu)
 */

#include <stdint.h>
#include <arm.h>
#include <mpu.h>
#include <systick.h>
#include <syscall.h>
#include <printk.h>
#include "syscall_thread.h"
#include "syscall_mutex.h"

/** @brief      Initial XPSR value, all 0s except thumb bit. */
#define XPSR_INIT 0x1000000

/** @brief Interrupt return code to user mode using PSP.*/
#define LR_RETURN_TO_USER_PSP 0xFFFFFFFD
/** @brief Interrupt return code to kernel mode using MSP.*/
#define LR_RETURN_TO_KERNEL_MSP 0xFFFFFFF1

/** @brief Max user threads; one per static priority level */
#define MAX_USER_THREADS 14
/** @brief TCB index of the idle thread */
#define IDLE_IDX MAX_USER_THREADS
/** @brief TCB index of the default (main) thread */
#define DEFAULT_IDX (MAX_USER_THREADS + 1)
/** @brief Total TCBs: user threads, idle, and default */
#define NUM_TCBS (MAX_USER_THREADS + 2)

/** @brief Smallest stack we hand out: 1KB, as log2 of bytes */
#define MIN_STACK_LOG2 10
/** @brief Bytes per stack word */
#define WORD_BYTES 4

/**
 * @brief      Heap high and low pointers.
 */
//@{
extern char
  __thread_u_stacks_low,
  __thread_u_stacks_top,
  __thread_k_stacks_low,
  __thread_k_stacks_top;
//@}

/** @brief User-mode stubs (svc_stubs.S) used as thread entry/exit points */
//@{
extern void thread_kill(void);
extern void idle_default(void);
//@}

/**
 * @brief      Precalculated values for UB test.
 */
float ub_table[] = {
  0.000, 1.000, .8284, .7798, .7568,
  .7435, .7348, .7286, .7241, .7205,
  .7177, .7155, .7136, .7119, .7106,
  .7094, .7083, .7075, .7066, .7059,
  .7052, .7047, .7042, .7037, .7033,
  .7028, .7025, .7021, .7018, .7015,
  .7012, .7009
};

/**
 * @struct user_stack_frame
 *
 * @brief  Stack frame upon exception.
 */
typedef struct {
  uint32_t r0;   /** @brief Register value for r0 */
  uint32_t r1;   /** @brief Register value for r1 */
  uint32_t r2;   /** @brief Register value for r2 */
  uint32_t r3;   /** @brief Register value for r3 */
  uint32_t r12;  /** @brief Register value for r12 */
  uint32_t lr;   /** @brief Register value for lr*/
  uint32_t pc;   /** @brief Register value for pc */
  uint32_t xPSR; /** @brief Register value for xPSR */
} interrupt_stack_frame;

/**
 * @brief  Registers psv_asm_handler pushes on the kernel stack, lowest
 *         address first (matches push {r0, r4-r11, lr}).
 */
typedef struct {
  uint32_t psp;         /**< thread's process stack pointer */
  uint32_t r4_r11[8];   /**< callee-saved registers */
  uint32_t exc_return;  /**< EXC_RETURN code: mode and stack to resume on */
} saved_context;

/** @brief Thread states */
typedef enum {
  THREAD_UNUSED = 0, /**< slot is free (never created or killed) */
  THREAD_RUNNABLE,   /**< ready to run (includes the running thread) */
  THREAD_WAITING     /**< waiting for its next period */
} thread_state;

/** @brief Thread control block */
typedef struct {
  void *msp;              /**< saved kernel stack pointer (context) */
  uint32_t svc_status;    /**< 1 if the thread was inside an SVC */
  thread_state state;     /**< scheduling state */
  uint32_t C;             /**< computation budget per period, in ticks */
  uint32_t T;             /**< period, in ticks */
  uint32_t period_time;   /**< ticks used in the current period */
  uint32_t total_time;    /**< ticks used since the scheduler started */
  uint32_t next_release;  /**< system time when the next period starts */
  uint32_t stack_slot;    /**< which user/kernel stack pair it owns */
  uint32_t effPrio;       /**< priority including held ceilings */
  uint32_t held;          /**< bitmask of held mutexes */
} tcb_t;

/** @brief All thread control blocks */
static tcb_t tcbs[NUM_TCBS];

/** @brief TCB index of the running thread; main runs first */
static volatile uint32_t running = DEFAULT_IDX;

/** @brief Nonzero once scheduler_start has run and threads remain */
static volatile int scheduler_running = 0;

/** @brief Ticks since the scheduler started */
static volatile uint32_t system_time = 0;

/** @brief Values from thread_init */
//@{
static uint32_t max_user_threads = 0;
static uint32_t stack_bytes = 0;
static void *idle_function = NULL;
//@}

/** @brief Bitmask of stack slots owned by live threads */
static uint32_t used_stack_slots = 0;

/** @brief All mutexes; user handles point into this array */
static kmutex_t mutexes[MAX_MUTEXES];

/** @brief max_mutexes from thread_init, and how many have been created */
//@{
static uint32_t mutexLimit = 0;
static uint32_t mutexCount = 0;
//@}

/**
 * @brief Sets a thread's effPrio to the highest of its static priority
 *        and the ceilings of the mutexes it holds.
 *
 * @param idx TCB index of a user thread
 */
static void update_prio(uint32_t idx) {
  tcb_t *tcb = &tcbs[idx];
  uint32_t prio = idx;
  for (uint32_t i = 0; i < mutexCount; i++) {
    if ((tcb->held & (1U << i)) && mutexes[i].prio_ceil < prio) {
      prio = mutexes[i].prio_ceil;
    }
  }
  tcb->effPrio = prio;
}

/**
 * @brief Re-enables interrupts if they were enabled before
 *        save_interrupt_state_and_disable.
 *
 * @param primask value returned by save_interrupt_state_and_disable
 */
static void restore_irqs(int primask) {
  if (!primask) enable_interrupts();
}

/**
 * @brief Warns if a thread is holding mutexes.
 *
 * @param idx    TCB index of a user thread
 * @param reason what the thread did
 */
static void warn_held(uint32_t idx, const char *reason) {
  if (tcbs[idx].held) {
    printk("Mutexes still held, thread %d %s\n", (int)idx, reason);
  }
}

/**
 * @brief Checks that a pointer is a mutex returned by sys_mutex_init.
 *
 * @param mutex pointer from the user
 *
 * @return 1 if valid, 0 otherwise
 */
static int mutex_valid(kmutex_t *mutex) {
  uintptr_t off = (uintptr_t)mutex - (uintptr_t)mutexes;
  return (uintptr_t)mutex >= (uintptr_t)mutexes &&
         off < mutexCount * sizeof(kmutex_t) &&
         off % sizeof(kmutex_t) == 0;
}

/**
 * @brief Finds the runnable user thread with the highest effective
 *        priority. Ties go to the thread holding a mutex.
 *
 * @param[out] anyAlive 1 if any user thread is runnable or waiting
 *
 * @return TCB index to run, or IDLE_IDX if none is runnable
 */
static uint32_t pick_next(int *anyAlive) {
  uint32_t next = IDLE_IDX;
  uint32_t bestPrio = UINT32_MAX;
  int bestHeld = 0;
  *anyAlive = 0;

  for (uint32_t i = 0; i < MAX_USER_THREADS; i++) {
    tcb_t *tcb = &tcbs[i];
    if (tcb->state == THREAD_UNUSED) continue;
    *anyAlive = 1;
    if (tcb->state != THREAD_RUNNABLE) continue;

    int held = tcb->held != 0;
    if (tcb->effPrio < bestPrio ||
        (tcb->effPrio == bestPrio && held && !bestHeld)) {
      next = i;
      bestPrio = tcb->effPrio;
      bestHeld = held;
    }
  }
  return next;
}

/**
 * @brief Builds a new thread's stacks so that the first PendSV return
 *        runs fn(vargp) in user mode, and fn returning calls thread_kill.
 *
 * @param tcb   TCB to fill in
 * @param fn    thread entry point
 * @param vargp argument passed in r0
 */
static void init_thread_context(tcb_t *tcb, void *fn, void *vargp) {
  char *u_top = &__thread_u_stacks_top - tcb->stack_slot * stack_bytes;
  char *k_top = &__thread_k_stacks_top - tcb->stack_slot * stack_bytes;

  interrupt_stack_frame *frame = (interrupt_stack_frame *)u_top - 1;
  frame->r0 = (uint32_t)vargp;
  frame->r1 = frame->r2 = frame->r3 = frame->r12 = 0;
  frame->lr = (uint32_t)&thread_kill;
  frame->pc = (uint32_t)fn & ~1U;  // stacked pc must not carry the thumb bit
  frame->xPSR = XPSR_INIT;

  saved_context *ctx = (saved_context *)k_top - 1;
  for (int i = 0; i < 8; i++) ctx->r4_r11[i] = 0;
  ctx->psp = (uint32_t)frame;
  ctx->exc_return = LR_RETURN_TO_USER_PSP;

  tcb->msp = ctx;
  tcb->svc_status = 0;
}

/**
 * @brief Saves the interrupted thread's context, runs the scheduler, and
 *        returns the kernel stack pointer of the thread to resume.
 *
 *        Runs the highest-priority runnable user thread, else idle while
 *        any user thread is alive, else the default thread (which ends
 *        scheduling).
 *
 * @param context_ptr kernel stack pointer after psv_asm_handler's push
 *
 * @return kernel stack pointer for psv_asm_handler to pop from
 */
void *pendsv_c_handler(void *context_ptr){
  tcb_t *cur = &tcbs[running];
  cur->msp = context_ptr;
  cur->svc_status = get_svc_status();

  // a killed idle thread is replaced by the kernel's default idle function
  if (running == IDLE_IDX && cur->state == THREAD_UNUSED) {
    init_thread_context(cur, &idle_default, NULL);
    cur->state = THREAD_RUNNABLE;
  }

  int anyAlive;
  uint32_t next = pick_next(&anyAlive);
  if (!anyAlive) {
    scheduler_running = 0;
    next = DEFAULT_IDX;
  }

  running = next;
  set_svc_status(tcbs[running].svc_status);
  return tcbs[running].msp;
}

/**
 * @brief Called from the SysTick handler: charges the tick to the running
 *        thread, enforces C, releases threads whose period has started,
 *        and pends a context switch.
 */
void scheduler_tick(void) {
  if (!scheduler_running) return;

  system_time++;

  if (running < MAX_USER_THREADS) {
    tcb_t *cur = &tcbs[running];
    cur->total_time++;
    if (++cur->period_time >= cur->C && cur->state == THREAD_RUNNABLE) {
      warn_held(running, "ran out of C");
      cur->state = THREAD_WAITING;
    }
  }

  for (uint32_t i = 0; i < MAX_USER_THREADS; i++) {
    tcb_t *tcb = &tcbs[i];
    if (tcb->state != THREAD_UNUSED && system_time >= tcb->next_release) {
      tcb->next_release += tcb->T;
      tcb->period_time = 0;
      tcb->state = THREAD_RUNNABLE;
    }
  }

  pend_pendsv();
}

int sys_thread_init(
  uint32_t max_threads,
  uint32_t stack_size,
  void *idle_fn,
  uint32_t max_mutexes
){
  if (max_mutexes > MAX_MUTEXES) return -1;

  uint32_t size_log2 = mm_log2ceil_size(stack_size * WORD_BYTES);
  if (size_log2 < MIN_STACK_LOG2) size_log2 = MIN_STACK_LOG2;
  uint32_t bytes = 1U << size_log2;

  // every user thread plus the idle thread needs its own stack pair
  uint32_t region = &__thread_u_stacks_top - &__thread_u_stacks_low;
  if (max_threads == 0 || max_threads > MAX_USER_THREADS ||
      (max_threads + 1) * bytes > region) {
    return -1;
  }

  max_user_threads = max_threads;
  stack_bytes = bytes;
  idle_function = (idle_fn != NULL) ? idle_fn : (void *)&idle_default;
  for (int i = 0; i < NUM_TCBS; i++) tcbs[i].state = THREAD_UNUSED;
  used_stack_slots = 0;
  mutexLimit = max_mutexes;
  mutexCount = 0;

  tcbs[IDLE_IDX].stack_slot = max_threads; // slots 0..max-1 are for users
  init_thread_context(&tcbs[IDLE_IDX], idle_function, NULL);
  tcbs[IDLE_IDX].state = THREAD_RUNNABLE;
  return 0;
}

int sys_thread_create(
  void *fn,
  uint32_t prio,
  uint32_t C,
  uint32_t T,
  void *vargp
){
  if (stack_bytes == 0 || prio >= MAX_USER_THREADS || C == 0 || C > T ||
      tcbs[prio].state != THREAD_UNUSED) {
    return -1;
  }

  // UB test over the live user threads plus the new one
  uint32_t n = 1;
  float utilization = (float)C / T;
  for (uint32_t i = 0; i < MAX_USER_THREADS; i++) {
    if (tcbs[i].state != THREAD_UNUSED) {
      n++;
      utilization += (float)tcbs[i].C / tcbs[i].T;
    }
  }
  if (n > max_user_threads || utilization > ub_table[n]) return -1;

  uint32_t slot = 0;
  while (used_stack_slots & (1U << slot)) slot++;
  used_stack_slots |= 1U << slot;

  tcb_t *tcb = &tcbs[prio];
  tcb->C = C;
  tcb->T = T;
  tcb->period_time = 0;
  tcb->total_time = 0;
  tcb->next_release = system_time + T;
  tcb->stack_slot = slot;
  tcb->effPrio = prio;
  tcb->held = 0;
  init_thread_context(tcb, fn, vargp);
  tcb->state = THREAD_RUNNABLE; // last, so SysTick never sees a half-built TCB
  return 0;
}

int sys_scheduler_start( uint32_t frequency ){
  int have_threads = 0;
  for (uint32_t i = 0; i < MAX_USER_THREADS; i++) {
    if (tcbs[i].state != THREAD_UNUSED) have_threads = 1;
  }
  if (!have_threads || frequency == 0) return -1;

  system_time = 0;
  scheduler_running = 1;
  systick_init(frequency);

  // PendSV preempts this SVC immediately; we resume here once every
  // user thread has exited
  pend_pendsv();
  return 0;
}

uint32_t sys_get_priority(){
  if (running < MAX_USER_THREADS) return tcbs[running].effPrio;
  return running;
}

uint32_t sys_get_time(){
  return system_time;
}

uint32_t sys_thread_time(){
  return tcbs[running].total_time;
}

void sys_thread_kill(){
  if (running == DEFAULT_IDX) {
    sys_exit(-1);
  }
  if (running < MAX_USER_THREADS && tcbs[running].held) {
    warn_held(running, "exited");
    sys_exit(-1);
  }
  if (running < MAX_USER_THREADS) {
    used_stack_slots &= ~(1U << tcbs[running].stack_slot);
  }
  // pendsv_c_handler swaps a killed idle thread for idle_default
  tcbs[running].state = THREAD_UNUSED;
  pend_pendsv();
}

void sys_wait_until_next_period(){
  if (running < MAX_USER_THREADS) {
    warn_held(running, "waited for next period");
    tcbs[running].state = THREAD_WAITING;
    pend_pendsv();
  }
}

kmutex_t *sys_mutex_init( uint32_t max_prio ) {
  kmutex_t *mutex = NULL;

  int pm = save_interrupt_state_and_disable();
  if (mutexCount < mutexLimit) {
    mutex = &mutexes[mutexCount];
    mutex->index = mutexCount;
    mutex->prio_ceil = max_prio;
    mutex->locked_by = NOT_LOCKED;
    mutexCount++;
  }
  restore_irqs(pm);
  return mutex;
}

void sys_mutex_lock( kmutex_t *mutex ) {
  if (!mutex_valid(mutex)) {
    printk("Invalid mutex, addr %x\n", (uint32_t)mutex);
    return;
  }
  if (running >= MAX_USER_THREADS) {
    printk("Idle/main thread can't lock mutexes\n");
    return;
  }

  uint32_t idx = running;
  uint32_t mask = 1U << mutex->index;
  if (tcbs[idx].held & mask) {
    printk("Mutex already held, thread %d mutex %d\n",
           (int)idx, (int)mutex->index);
    return;
  }
  if (idx < mutex->prio_ceil) {
    printk("Priority above mutex ceiling, killing thread %d mutex %d\n",
           (int)idx, (int)mutex->index);
    sys_thread_kill();
    return;
  }

  int pm = save_interrupt_state_and_disable();
  // only happens if the holder ran out of C while holding it
  while (mutex->locked_by != NOT_LOCKED) {
    pend_pendsv();
    restore_irqs(pm);
    pm = save_interrupt_state_and_disable();
  }
  mutex->locked_by = idx;
  tcbs[idx].held |= mask;
  update_prio(idx);
  restore_irqs(pm);
}

void sys_mutex_unlock( kmutex_t *mutex ) {
  if (!mutex_valid(mutex)) {
    printk("Invalid mutex, addr %x\n", (uint32_t)mutex);
    return;
  }

  uint32_t idx = running;
  if (mutex->locked_by != idx) {
    printk("Mutex not held, thread %d mutex %d\n",
           (int)idx, (int)mutex->index);
    return;
  }

  int pm = save_interrupt_state_and_disable();
  mutex->locked_by = NOT_LOCKED;
  tcbs[idx].held &= ~(1U << mutex->index);
  update_prio(idx);
  pend_pendsv();
  restore_irqs(pm);
}
