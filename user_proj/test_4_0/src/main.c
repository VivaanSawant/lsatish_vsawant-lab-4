/**
 * @file   main.c
 *
 * @brief  Tests thread and main exit.
 *
 * @author Ronit Banerjee <ronitb@andrew.cmu.edu>
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

void thread_0( UNUSED void *vargp ) {
  int cnt = 0;
  while (cnt < 5) {
    print_num_status_cnt( 0, cnt++ );
    wait_until_next_period();
  }
  printf( "Thread 0 exiting...\n" );
  return;
}

void idle_thread() {
  while ( 1 );
}

int main( void ) {

  printf( "Entered user mode\n" );

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, &idle_thread, NUM_MUTEXES ) );

  ABORT_ON_ERROR( thread_create( &thread_0, 0, 100, 500, NULL ) );

  printf( "Successfully created threads! Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  printf( "Test passed!\n" );

  return 0;
}
