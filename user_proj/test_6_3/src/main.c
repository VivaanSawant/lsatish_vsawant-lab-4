/**
 * @file   main.c
 *
 * @brief  Basic test for nested mutexes.
 *
 * t = 0 --- Task: 0, 0 locked
 * t = 8 --- Task: 0, 0 unlocked
 * t = 8 --- Task: 1, 0 locked
 * t = 8 --- Task: 1, 1 locked
 * t = 16 --- Task: 1, 0 unlocked
 * t = 16 --- Task: 1, 1 unlocked
 * t = 104 --- Task: 0, 0 locked
 * t = 112 --- Task: 0, 0 unlocked
 * t = 200 --- Task: 1, 0 locked
 * t = 200 --- Task: 1, 1 locked
 * t = 208 --- Task: 1, 0 unlocked
 * t = 208 --- Task: 0, 0 locked
 * t = 216 --- Task: 0, 0 unlocked
 * t = 216 --- Task: 1, 1 unlocked
 * t = 312 --- Task: 0, 0 locked
 * t = 320 --- Task: 0, 0 unlocked
 * t = 400 --- Task: 1, 0 locked
 * t = 400 --- Task: 1, 1 locked
 * t = 408 --- Task: 1, 0 unlocked
 * t = 408 --- Task: 1, 1 unlocked
 * t = 416 --- Task: 0, 0 locked
 * t = 424 --- Task: 0, 0 unlocked
 * t = 520 --- Task: 0, 0 locked
 * t = 528 --- Task: 0, 0 unlocked
 * t = 600 --- Task: 1, 0 locked
 * t = 600 --- Task: 1, 1 locked
 * t = 608 --- Task: 1, 0 unlocked
 * t = 608 --- Task: 1, 1 unlocked
 * t = 624 --- Task: 0, 0 locked
 * t = 632 --- Task: 0, 0 unlocked
 *
 */
#include <349_lib.h>
#include <349_threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/** @brief thread user space stack size - 1KB */
#define USR_STACK_WORDS 256
#define NUM_THREADS 2
#define NUM_MUTEXES 2
#define CLOCK_FREQUENCY 100

/** @brief How much to reduce spin wait */
#define REDUCE_SPIN_MS 2

typedef struct {
  mutex_t *mutex_0;
  mutex_t *mutex_1;
} arg_t;

/** @brief Basic thread which locks and unlocks a mutex
 *  T0 (10, 104)
 */
void thread_0( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;
  int cnt = 0;

  while( 1 ) {
    cnt++;
    mutex_lock( arg->mutex_0 );
    print_status( "Thread 0, 0 locked" );

    spin_wait( 10 - REDUCE_SPIN_MS );

    mutex_unlock( arg->mutex_0 );
    print_status( "Thread 0, 0 unlocked" );

    if ( cnt > 6 ) {
      break;
    }

    wait_until_next_period();
  }
}

/** @brief Basic thread which locks and unlocks a mutex
 *         T1 (30, 200)
 */
void thread_1( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;
  int cnt = 0;

  while( 1 ) {
    cnt++;
    mutex_lock( arg->mutex_0 );
    print_status( "Thread 1, 0 locked" );
    mutex_lock( arg->mutex_1 );
    print_status( "Thread 1, 1 locked" );

    spin_wait( 10 - REDUCE_SPIN_MS );

    print_status( "Thread 1, 0 unlocked" );
    mutex_unlock( arg->mutex_0 );
    print_status( "Thread 1, 1 unlocked" );
    mutex_unlock( arg->mutex_1 );

    if ( cnt > 3 ) {
      break;
    }

    wait_until_next_period();
  }
}

int main() {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ) );

  mutex_t *s0 = mutex_init( 0 );
  if ( s0 == NULL ) {
    printf( "Failed to create mutex 0\n" );
    return -1;
  }

  mutex_t *s1 = mutex_init( 1 );
  if ( s1 == NULL ) {
    printf( "Failed to create mutex 0\n" );
    return -1;
  }

  arg_t *arg = malloc( sizeof( arg_t ) );
  if ( arg == NULL ) {
    printf( "Malloc failed\n" );
    return -1;
  }
  arg->mutex_0 = s0;
  arg->mutex_1 = s1;

  ABORT_ON_ERROR( thread_create( &thread_0, 0, 20, 104, ( void * )arg ) );
  ABORT_ON_ERROR( thread_create( &thread_1, 1, 30, 200, ( void * )arg ) );

  printf( "Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return 0;
}
