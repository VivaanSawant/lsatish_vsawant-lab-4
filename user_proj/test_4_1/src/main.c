/**
 * @file   main.c
 *
 * @brief  Tests spawning after scheduler_start.
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
#define NUM_THREADS 6
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 1000

/** @brief repetitions for the spawner */
#define SPAWNER_REPS 3
/** @brief repetitions for the functions */
#define FN_REPS 2
/** @brief thread return counters */
int counters[ NUM_THREADS ] = { 0, 0, 0, 0, 0 };

void thread_fn( void *vargp ) {
  int cnt = 0;
  int num = ( int )vargp;
  while ( cnt < FN_REPS ) {
    print_num_status_cnt( num, cnt++ );
    wait_until_next_period();
  }
  counters[ num ]++;
  return;
}

void thread_spawner( UNUSED void *vargp ) {
  int cnt = 0;
  int status = 0;
  while ( cnt < SPAWNER_REPS ) {
    print_num_status_cnt( 0, cnt++ );
    for ( int i = 1; i < NUM_THREADS; i++ ) {
      status = thread_create( &thread_fn, i, 50, 500, ( void * )i );
      if ( status ) printf( "Test failed on thread_create %d.\n", i );
    }
    wait_until_next_period();
  }
  counters[ 0 ]++;
}

int main( void ) {

  printf( "In user mode.\n" );

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ) );

  ABORT_ON_ERROR( thread_create( &thread_spawner, 0, 100, 2000, NULL ) );

  printf( "Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  if ( counters[ 0 ] == 1 && counters[ 1 ] == SPAWNER_REPS &&
      counters[ 2 ] == SPAWNER_REPS && counters[ 3 ] == SPAWNER_REPS &&
      counters[ 4 ] == SPAWNER_REPS && counters[ 5 ] == SPAWNER_REPS ) {
    printf( "Test passed!\n" );
  } else {
    printf( "Test failed.\n" );
    return 1;
  }

  return 0;

}
