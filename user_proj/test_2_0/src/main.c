/**
 * @file   main.c
 *
 * @brief  Tests for idle thread context saving. Requires WUNP.
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
#define NUM_THREADS 1
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 1000

/** @brief Computation time of the task */
#define THREAD_C_MS 10
/** @brief Period of the thread */
#define THREAD_T_MS 10

/** @brief Idle thread that does something! */
void idle_thread() {

  if ( print_fibs( 10000000, 100000, 1000000000 ) == 380546875) {
    printf( "Test passed!\n" );
  } else {
    printf( "Test failed...\n" );
  }

  while ( 1 );

}

void thread_0( UNUSED void *vargp ) {
  while( 1 ) {
    wait_until_next_period();
  }
}

int main( UNUSED int argc, UNUSED char *const argv[]  ) {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, &idle_thread, NUM_MUTEXES ) );

  ABORT_ON_ERROR( thread_create( &thread_0, 0, THREAD_C_MS, THREAD_T_MS, NULL ) );

  printf( "Successfully created threads! Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return 0;

}
