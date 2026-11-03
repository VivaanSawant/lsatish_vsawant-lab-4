/** @file syscall_thread.h
 *
 *  @brief  Custom syscalls to support thread library.
 *
 *  @date   March 27, 2019
 *
 *  @author Ronit Banerjee <ronitb@andrew.cmu.edu>
 */

#ifndef _SYSCALL_THREAD_H_
#define _SYSCALL_THREAD_H_

#include <unistd.h>


/**
 * @brief      The PendSV interrupt handler.
 */
void *pendsv_c_handler( void * );

/**
 * @brief      Initialize the thread library
 *
 *             A user program must call this initializer before attempting to
 *             create any threads or starting the scheduler.
 *
 * @param[in]  max_threads        Maximum number of threads that will be
 *                                created.
 * @param[in]  stack_size         Declares the size in words of all user and
 *                                kernel stacks created.
 * @param[in]  idle_fn            Pointer to a thread function to run when no
 *                                other threads are runnable. If NULL is
 *                                is supplied, the kernel will provide its
 *                                own idle function that will sleep.
 * @param[in]  max_mutexes        Maximum number of mutexes that will be
 *                                created.
 *
 * @return     0 on success or -1 on failure
 */
int sys_thread_init(
  uint32_t        max_threads,
  uint32_t        stack_size,
  void           *idle_fn,
  uint32_t        max_mutexes
);

/**
 * @brief      Create a new thread running the given function. The thread will
 *             not be created if the UB test fails, and in that case this function
 *             will return an error.
 *
 * @param[in]  fn     Pointer to the function to run in the new thread.
 * @param[in]  prio   Priority of this thread. Lower number are higher
 *                    priority.
 * @param[in]  C      Real time execution time (scheduler ticks).
 * @param[in]  T      Real time task period (scheduler ticks).
 * @param[in]  vargp  Argument for thread function (usually a pointer).
 *
 * @return     0 on success or -1 on failure
 */
int sys_thread_create( void *fn, uint32_t prio, uint32_t C, uint32_t T, void *vargp );

/**
 * @brief      Allow the kernel to start running the thread set.
 *
 *             This function should enable SysTick and thus enable your
 *             scheduler. It will not return immediately unless there is an error.
 *			   It may eventually return successfully if all thread functions are
 *   		   completed or killed.
 *
 * @param[in]  frequency  Frequency (Hz) of context swaps.
 *
 * @return     0 on success or -1 on failure
 */
int sys_scheduler_start( uint32_t frequency );

/**
 * @brief      Get the current time.
 *
 * @return     The time in ticks.
 */
uint32_t sys_get_time( void );

/**
 * @brief      Get the effective priority of the current running thread
 *
 * @return     The thread's effective priority
 */
uint32_t sys_get_priority( void );

/**
 * @brief      Gets the total elapsed time for the thread (since its first
 *             ever period).
 *
 * @return     The time in ticks.
 */
uint32_t sys_thread_time( void );

/**
 * @brief      Waits efficiently by descheduling thread.
 */
void sys_wait_until_next_period( void );

/**
* @brief      Kills current running thread. Aborts program if current thread is
*             main thread or the idle thread or if current thread exited
*             while holding a mutex.
*
* @return     Does not return.
*/
void sys_thread_kill( void );

/**
 * @brief      Get the current time.
 *
 * @return     The time in ticks.
 */
uint32_t sys_get_time( void );

/**
 * @brief      Get the effective priority of the current running thread
 *
 * @return     The thread's effective priority
 */
uint32_t sys_get_priority( void );

/**
 * @brief      Gets the total elapsed time for the thread (since its first
 *             ever period).
 *
 * @return     The time in ticks.
 */
uint32_t sys_thread_time( void );

/**
 * @brief      Waits efficiently by descheduling thread.
 */
void sys_wait_until_next_period( void );

/**
* @brief      Kills current running thread. Aborts program if current thread is
*             main thread or the idle thread or if current thread exited
*             while holding a mutex.
*
* @return     Does not return.
*/
void sys_thread_kill( void );

#endif /* _SYSCALL_THREAD_H_ */