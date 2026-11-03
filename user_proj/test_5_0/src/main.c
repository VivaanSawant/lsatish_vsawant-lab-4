/**
 * @file   main.c
 *
 * @brief  Test for mutex lock/unlock.
 *         T1 (500, 500)
 */

#include <349_lib.h>
#include <349_threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/** @brief thread user space stack size - 4KB */
#define USR_STACK_WORDS 256
#define NUM_THREADS 1
#define NUM_MUTEXES 1
#define CLOCK_FREQUENCY 1000



/** @brief Thread 1 (500, 500)
 */
void thread( void *vargp ) {
  int cnt = 0;
  mutex_t *s1 = ( mutex_t * )vargp;

  if ( s1 == NULL ) {
    printf( "Invalid mutex passed\n" );
    return;
  }

  while( 1 ) {
    print_num_status_cnt( 0, cnt++ );

    if ( cnt > 2 ) {
      printf( "Test complete.\n" );
      break;
    }

    printf( "Locking mutex..." );
    mutex_lock( s1 );
    printf( "Success!\n" );

    spin_wait( 10 );

    printf( "Unlocking mutex..." );
    mutex_unlock( s1 );
    printf( "Success!\n" );

    spin_wait( 10 );

    printf( "Unlocking mutex again. Should print warning!\n" );
    mutex_unlock( s1 );
    printf("unlocking \n");

    spin_wait( 10 );

    printf( "Locking mutex.\n" );
    mutex_lock( s1 );

    spin_wait( 10 );

    printf( "Locking mutex again. Should print warning!\n" );
    mutex_lock( s1 );

    spin_wait( 10 );

    printf( "Unlocking mutex..." );
    mutex_unlock( s1 );
    printf( "Success!\n" );

    wait_until_next_period();
  }

  return;
}

int main() {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ) );

  mutex_t *s1 = mutex_init( 0 );
  if ( s1 == NULL ) {
    printf("mutex_init failed.\n");
    return -1;
  }

  ABORT_ON_ERROR( thread_create( &thread, 0, 500, 500, ( void * )s1 ) );

  printf( "Starting scheduler...\n" );

	ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return 0;
}
