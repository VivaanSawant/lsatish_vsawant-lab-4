/**
 * @file   main.c
 *
 * @brief  Test for nested mutex implementation.
 *
 * @note expected output:
 *
 * t=0     Thread 1 locked                                                         
 * t=88    Thread 1 unlocked                                                       
 * t=88    Thread 2a locked                                                        
 * t=88    Thread 2b locked                                                        
 * t=500   Thread 1 locked                                                         
 * t=588   Thread 1 unlocked                                                       
 * t=774   Thread 2c unlocked                                                      
 * t=774   Thread 2d unlocked                                                      
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
  int done;
} arg_t;

/** @brief Basic thread which locks and unlocks a mutex
 *  T0 (100, 500)
 */
void thread_0( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;

  while( 1 ) {
    if ( arg->done < 2 ) {
      mutex_lock( arg->mutex_0 );
      print_status( "1 locked" );

      spin_wait( 90 - REDUCE_SPIN_MS );

      print_status( "1 unlocked" );
      mutex_unlock( arg->mutex_0 );
      arg->done++;

      wait_until_next_period();
    }
    else {
      exit( -1 );
    }
  }
}

/** @brief Basic thread which locks and unlocks a mutex
 *         T1 (150, 1000)
 */
void thread_1( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;

  while( 1 ) {
    mutex_lock( arg->mutex_1 );
    print_status( "2a locked" );

    mutex_lock( arg->mutex_0 );
    print_status( "2b locked" );

    mutex_unlock( arg->mutex_0 );

    // we should be preempted here by task 1...
    spin_wait( 600 - REDUCE_SPIN_MS );

    print_status( "2c unlocked" );
    mutex_unlock( arg->mutex_1 );
    print_status( "2d unlocked" );
    exit( 0 );
  }
}

int main( void ) {
  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ) );

  mutex_t *s0 = mutex_init( 0 );
  if ( s0 == NULL ) {
    printf( "Failed to create mutex 0\n" );
    return -1;
  }

  mutex_t *s1 = mutex_init( 1 );
  if ( s1 == NULL ) {
    printf( "Failed to create mutex 1\n" );
    return -1;
  }

  arg_t *arg = malloc( sizeof( arg_t ) );
  if ( arg == NULL ) {
    printf( "Malloc failed\n" );
    return -1;
  }
  arg->mutex_0 = s0;
  arg->mutex_1 = s1;
  arg->done = 0;

  ABORT_ON_ERROR( thread_create( &thread_0, 0, 100, 500, ( void * )arg ) );
  ABORT_ON_ERROR( thread_create( &thread_1, 1, 10000, 1000000, ( void * )arg ) );

  printf( "Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return -1;
}
