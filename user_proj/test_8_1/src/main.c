/**
 * @file   main.c
 *
 * @brief Written Question 2.
 * T1: (300, 700), S1(100-300)
 * T2: (100, 900), S2(0-100)
 * T3: (600, 2600), S1(0-400), S2(200-600)
 * Expected string: SCHEDUSCLHSCHSCEDUSCLSCH
 *
 * @author Benjamin Huang <zemingbh@andrew.cmu.edu>
 *
 * @author Harry Fernandez <hfernand@andrew.cmu.edu>
 */

#include <349_lib.h>
#include <349_threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define UNUSED __attribute__((unused))

#define USR_STACK_WORDS 256
#define NUM_THREADS 3
#define NUM_MUTEXES 2
#define CLOCK_FREQUENCY 200

/** @brief How much to reduce spin wait */
#define REDUCE_SPIN_MS 2

//#define TEST_DEBUG

typedef struct {
  mutex_t *mutex_0;
  mutex_t *mutex_1;
} arg_t;

/** @brief Debug print a single char
 *
 *  @param c   the character
 */
void print_char( int c ) {
  putchar( c );
#ifdef TEST_DEBUG
  printf( " - Prio: %lu \t Time: %lu\n", get_priority(), get_time() );
#endif
}

/** @brief T0: (300, 700), S1(100-300)
 */
void thread_0( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;
  int cnt = 0;

  while ( cnt < 6 ) {
    print_char( 'S' );
    spin_wait( 100 - REDUCE_SPIN_MS );
    mutex_lock( arg->mutex_0 );
    print_char( 'C' );
    spin_wait( 200 - REDUCE_SPIN_MS );
    mutex_unlock( arg->mutex_0 );
    cnt++;
    wait_until_next_period();
  }
}


/** @brief T1: (100, 900)
 */
void thread_1( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;
  int cnt = 0;

  while ( cnt < 4 ) {
    mutex_lock( arg->mutex_1 );
    spin_wait( 50 - REDUCE_SPIN_MS );
    print_char( 'H' );
    spin_wait( 50 - REDUCE_SPIN_MS );
    mutex_unlock( arg->mutex_1 );
    cnt++;
    wait_until_next_period();
  }

  printf( " - Obtained\n" );
  printf( "SCHEDUSCLHSCHSCEDUSCLSCH - Expected\n" );
}

/** @brief T2: (600, 2600), S1(0-400), S2(200-600)
 */
void thread_2( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;
  int cnt = 0;

  while ( cnt < 2 ) {
    mutex_lock( arg->mutex_0 );
    spin_wait( 100 - REDUCE_SPIN_MS );
    print_char( 'E' );
    spin_wait( 100 - REDUCE_SPIN_MS );
    mutex_lock( arg->mutex_1 );
    spin_wait( 50 - REDUCE_SPIN_MS );
    print_char( 'D' );
    spin_wait( 100 - REDUCE_SPIN_MS );
    print_char( 'U' );
    spin_wait( 50 - REDUCE_SPIN_MS );
    mutex_unlock( arg->mutex_0 );
    spin_wait( 150 - REDUCE_SPIN_MS );
    print_char( 'L' );
    spin_wait( 50 - REDUCE_SPIN_MS );
    mutex_unlock( arg->mutex_1 );
    cnt++;
    wait_until_next_period();
  }
}

int main() {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ) );

  mutex_t *mutex_0 = mutex_init( 0 );
  if ( mutex_0 == NULL ) {
    printf( "Failed to create mutex 0\n" );
    return -1;
  }

  mutex_t *mutex_1 = mutex_init( 1 );
  if ( mutex_1 == NULL ) {
    printf( "Failed to create mutex 1\n" );
    return -1;
  }

  arg_t *arg = malloc( sizeof( arg_t ) );
  if ( arg == NULL ) {
    printf( "Malloc on arg failed\n" );
    return -1;
  }
  arg->mutex_0 = mutex_0;
  arg->mutex_1 = mutex_1;

  ABORT_ON_ERROR( thread_create( &thread_0, 0, 300, 700, ( void * )arg ) );
  ABORT_ON_ERROR( thread_create( &thread_1, 1, 100, 900, ( void * )arg ) );
  ABORT_ON_ERROR( thread_create( &thread_2, 2, 600, 2600, ( void * )arg ) );

  printf( "Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return RET_0349;
}
