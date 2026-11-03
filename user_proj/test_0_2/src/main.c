/**
 * @file    main.c
 *
 * @brief   14 threads in round robin.
 *
 * @author  Benjamin Huang <zemingbh@andrew.cmu.edu>
 *          Ronit Banerjee <ronitb@andrew.cmu.edu>
 */

#include <349_lib.h>
#include <349_threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/** @brief thread user space stack size - 4KB */
#define USR_STACK_WORDS 256
#define NUM_THREADS 14
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 1

/** @brief Period of each task (since they are all the same) */
#define THREAD_TIME_MS 1
/** @brief Period of each thread (they are all the same) */
#define THREAD_PERIOD (NUM_THREADS * THREAD_TIME_MS * 2)

void thread_fn( UNUSED void *vargp ) {
  int cnt = 0;

  while( 1 ) {
    print_num_status_cnt( get_priority() , cnt++ );
    wait_until_next_period();
  }
}

void idle_thread() {
  while( 1 );
}

int main( void ) {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, &idle_thread, NUM_MUTEXES ) );

  int i;
  for ( i = 0; i < NUM_THREADS; ++i ) {
    ABORT_ON_ERROR( thread_create( &thread_fn, i, THREAD_TIME_MS, THREAD_PERIOD, NULL ),
      "Failed to create thread %d\n", i
    );
  }

  printf( "Successfully created threads! Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return 0;

}
