/**
 * @file   main.c
 *
 * @brief  Tests the implicit idle thread and creating too many threads.
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
#define NUM_THREADS 4
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 1000

/** @brief Computation time of the task */
#define THREAD_C_MS 100
/** @brief Period of the thread */
#define THREAD_T_MS 1000

void thread_fn( void *vargp ) {
  int num = ( int )vargp;
  print_num_status( num );
  while ( 1 ) wait_until_next_period();
}

void thread_fn_last( void *vargp ) {
  int num = ( int )vargp;
  print_num_status( num );
  printf(
    "A memory error after this is probably a\n"
    "implicit idle thread error.\n"
  );
  wait_until_next_period();
  printf( "Test passed!\n" );
  while ( 1 ) wait_until_next_period();
}

int main( void ) {

  printf( "In user mode.\n" );

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ),
    "Test failed: implicit idle thread.\n"
  );

  for ( int i = 0; i < NUM_THREADS - 1; i++ ) {
    ABORT_ON_ERROR( thread_create(&thread_fn, i, THREAD_C_MS, THREAD_T_MS, ( void * )i ),
      "Failed to create thread %d\n", i
    );
  }
  ABORT_ON_ERROR( thread_create( &thread_fn_last, NUM_THREADS - 1, THREAD_C_MS, THREAD_T_MS, ( void * )( NUM_THREADS - 1 ) ),
    "Failed to create thread %d\n", NUM_THREADS - 1
  );

  int stat = thread_create( &thread_fn_last, 2, THREAD_C_MS, THREAD_T_MS, NULL );
  if ( stat == 0 ) {
    printf("Test failed: max_threads exceeded.\n");
  }

  printf( "Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return 0;
}
