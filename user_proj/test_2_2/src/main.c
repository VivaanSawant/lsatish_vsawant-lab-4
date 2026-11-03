/**
 * @file   main.c
 *
 * @brief  Tests stack allocation checks.
 *
 * @author Benjamin Huang <zemingbh@andrew.cmu.edu>
 */

#include <349_lib.h>
#include <349_threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/** @brief thread user space stack size - 1KB */
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 1000

void idle_thread() {
  while ( 1 );
}

int main( void ) {
  
  printf( "Entered user mode...\n" );

  int status;
  status = thread_init( 4, 16384, &idle_thread, NUM_MUTEXES );
  if (!status) {
    printf( "Test failed, stack should be too large (%d threads).\n", 4 );
    return -1;
  }

  status = thread_init( 1, 16384, &idle_thread, NUM_MUTEXES );
  if (!status) {
    printf( "Test failed, stack should be too large (%d threads).\n", 1 );
    return -1;
  }

  status = thread_init( 12, 1024, &idle_thread, NUM_MUTEXES );
  if (!status) {
    printf( "Test failed, stack should be too large (%d threads).\n", 12 );
    return -1;
  }

  printf( "Test passed!\n" );

  return 0;
}
