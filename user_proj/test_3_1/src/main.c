/**
 * @file   main.c
 *
 * @brief  Test for rate-monotonic scheduling, no preemption
 *         T0 (50, 1000), T1 (50, 2000), T2 (50, 3000)
 *
 * @author Benjamin Huang <zemingbh@andrew.cmu.edu>
 */

#include <349_lib.h>
#include <349_threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/** @brief thread user space stack size - 1KB */
#define USR_STACK_WORDS 256
#define NUM_THREADS 3
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 1000

/** @brief Computation time of the task */
#define THREAD_C_MS 50
/** @brief Period of the thread */
static const int THREAD_T_MS[] = {1000, 2000, 3000};

/** @brief Thread function
 */
void thread_fn( void* vargp ) {
  int cnt = 0;
  int num = ( int )vargp;
  while( 1 ) {
    print_num_status_cnt( num, cnt++ );
    wait_until_next_period();
  }
}

int main( void ) {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ) );

  for ( int i = 0; i < NUM_THREADS; i++ ) {
    ABORT_ON_ERROR( thread_create( &thread_fn, i, THREAD_C_MS, THREAD_T_MS[i], ( void * )i ),
      "Failed to create thread %d\n", i
    );
  }

  printf( "Expecting threads 0-1-2-0-0-1-0-2-0-1-0, repeating.\n" );
  printf( "Successfully created threads! Starting scheduler...\n" );

  ABORT_ON_ERROR(
    scheduler_start( CLOCK_FREQUENCY ),
    "Threads are unschedulable!\n"
  );

  return 0;
}
