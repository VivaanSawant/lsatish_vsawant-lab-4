/**
 * @file   main.c
 *
 * @brief  Basic test for mutex implementation.
 *         Thread 2 should not get preempted by Thread 0.
 *         T0 (10, 100), S1 (0-10)
 *         T1 (50, 200), S1 (0-50)
 *         T2 (75, 400), S1 (0-75)
 *
 * @note  Expected output:
 *       t = 0 --- Task: 0, locked
 *       t = 8 --- Task: 0, unlocked
 *       t = 8 --- Task: 1, locked
 *       t = 56 --- Task: 1, unlocked
 *       t = 56 --- Task: 2, locked
 *       t = 129 --- Task: 2, unlocked
 *       t = 129 --- Task: 0, locked
 *       t = 137 --- Task: 0, unlocked
 *       t = 200 --- Task: 0, locked
 *       t = 208 --- Task: 0, unlocked
 *       t = 208 --- Task: 1, locked
 *       t = 256 --- Task: 1, unlocked
 *       t = 300 --- Task: 0, locked
 *       t = 308 --- Task: 0, unlocked
 *       t = 400 --- Task: 0, locked
 *       t = 408 --- Task: 0, unlocked
 *       t = 408 --- Task: 1, locked
 *       t = 456 --- Task: 1, unlocked
 *       t = 456 --- Task: 2, locked
 *       t = 529 --- Task: 2, unlocked
 *       t = 529 --- Task: 0, locked
 *       t = 537 --- Task: 0, unlocked
 *       t = 600 --- Task: 0, locked
 *       t = 608 --- Task: 0, unlocked
 *       t = 608 --- Task: 1, locked
 *       t = 656 --- Task: 1, unlocked
 */

#include <349_lib.h>
#include <349_threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/** @brief thread user space stack size - 1KB */
#define USR_STACK_WORDS 256
#define NUM_THREADS 3
#define NUM_MUTEXES 1
#define CLOCK_FREQUENCY 50

/** @brief How much to reduce spin wait */
#define REDUCE_SPIN_MS 2

/** @brief Basic thread which locks and unlocks a mutex
 *         T1 (10, 100)
 */
void thread_0( void *vargp ) {
  mutex_t *mutex = ( mutex_t * ) vargp;

  for ( int i = 0; i < 7; ++i ) {
    mutex_lock( mutex );
    print_status( "0 locked" );

    spin_wait( 10 - REDUCE_SPIN_MS );

    print_status( "0 unlocked" );
    mutex_unlock( mutex );

    wait_until_next_period();
  }
}

/** @brief Basic thread which locks and unlocks a mutex
 *         T2 (50, 200)
 */
void thread_1( void *vargp ) {
  mutex_t *mutex = ( mutex_t * ) vargp;

  for ( int i = 0; i < 4; ++i ) {
    mutex_lock( mutex );
    print_status( "1 locked" );

    spin_wait( 50 - REDUCE_SPIN_MS );

    print_status( "1 unlocked" );
    mutex_unlock( mutex );

    wait_until_next_period();
  }
}

/** @brief Basic thread which holds onto a mutex for a "long" time
 *         T3 (75, 400)
 */
void thread_2( void *vargp ) {
  mutex_t *mutex = ( mutex_t * )vargp;

  for ( int i = 0; i < 2; ++i ) {
    mutex_lock( mutex );
    print_status( "2 locked" );

    spin_wait( 75 - REDUCE_SPIN_MS );

    print_status( "2 unlocked" );
    mutex_unlock( mutex );

    wait_until_next_period();
  }
}


int main( void ) {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ) );

  mutex_t *s1 = mutex_init( 0 );
  if ( s1 == NULL ) {
    printf( "Failed to create mutex 0\n" );
    return -1;
  }

  ABORT_ON_ERROR( thread_create( &thread_0, 0, 10, 100, ( void * )s1 ) );
  ABORT_ON_ERROR( thread_create( &thread_1, 1, 50, 200, ( void * )s1 ) );
  ABORT_ON_ERROR( thread_create( &thread_2, 2, 75, 400, ( void * )s1 ) );

  printf( "Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return 0;
}
