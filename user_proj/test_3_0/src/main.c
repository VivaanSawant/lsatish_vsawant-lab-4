/**
 * @file   main.c
 *
 * @brief  Tests the UB test.
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

void thread_fn( void *vargp ) {
  int cnt = 0;
  int num = ( int )vargp;
  while ( cnt < 2 ) {
    print_num_status_cnt( num, cnt++ );
    wait_until_next_period();
  }
  if ( num == NUM_THREADS - 1 ) printf( "Test passed!\n" );
  while ( 1 ) wait_until_next_period();
}

int main( void ) {

  printf( "In user mode.\n" );

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ) );

  for ( int i = 0; i < 2; i++ ) {
    ABORT_ON_ERROR( thread_create( &thread_fn, i, 50, 200, ( void * )i ),
      "Thread %d\n", i
    );
  }
  
  int stat, try_C;
  for ( try_C = 1000; try_C > 0; try_C -= 100 ) {
    stat = thread_create( &thread_fn, 2, try_C, 1000, ( void * )2 );
    if ( stat == 0 ) break;
  }
  
  if ( try_C != 200 ) {
    printf ( "Test failed, thread 2. C = %d\n", try_C );
    return 1;
  }
  
  for ( try_C = 1000; try_C > 0; try_C -= 25 ) {
    stat = thread_create( &thread_fn, 3, try_C, 5000, ( void * )3 );
    if ( stat == 0 ) break;
  }
  if ( try_C != 275 ) {
    printf ( "Test failed, thread 3. C = %d\n", try_C );
    return 1;
  }

  printf( "Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return 0;
}
