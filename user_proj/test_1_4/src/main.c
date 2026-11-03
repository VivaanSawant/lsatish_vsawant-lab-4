/**
 * @file   main.c
 *
 * @brief  Tests WUNP with two threads. Requires C and T to be implemented.
 *
 * @author  Benjamin Huang <zemingbh@andrew.cmu.edu>
 */

#include <349_lib.h>
#include <349_threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/** @brief thread user space stack size - 1KB */
#define USR_STACK_WORDS 256
#define NUM_THREADS 2
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 1000

/** @brief Computation time of the task */
#define THREAD0_C_MS 321
#define THREAD1_C_MS 123
/** @brief Period of the thread */
#define THREAD_T_MS 1024

/** @brief Give some leeway when testing */
#define SLACK_MILLISECONDS 5

void thread_0( void *vargp ) {
  uint32_t cnt = 0;
  uint32_t *ptr_time = ( uint32_t * )vargp;
  while( 1 ) {
    uint32_t t = get_time();

    if ( cnt > 20 ) {
      // printf( "Test passed!\n" );
      while ( 1 );
    }

    if ( t < THREAD_T_MS * cnt || t > THREAD_T_MS * cnt + SLACK_MILLISECONDS ) {
      printf( "Failed. Should have woken up at or after t = %lu\n", THREAD_T_MS * cnt );
      printf( "Woke up at t = %lu\n", t );
      while( 1 );
    }

    print_num_status_cnt( 0, cnt++ );
    *ptr_time = get_time();
    wait_until_next_period();
  }
}

void thread_1( void *vargp ) {
  uint32_t cnt = 0;
  uint32_t *ptr_time = ( uint32_t * )vargp;
  while ( 1 ) {
    uint32_t t = get_time();

    if ( t > *ptr_time + SLACK_MILLISECONDS ) {
      printf( "Failed. Should have woken up close to t = %lu\n", *ptr_time );
      printf( "Woke up at t = %lu\n", t );
      while( 1 );
    }

    print_num_status_cnt( 1, cnt++ );

    if ( cnt > 20 ) {
      printf( "Test passed!\n" );
      while ( 1 );
    }

    wait_until_next_period();
  }
}

void idle_thread() {
  while( 1 );
}

int main( UNUSED int argc, UNUSED char *const argv[] ) {
  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, &idle_thread, NUM_MUTEXES ) );

  printf( "Successfully initialized threads...\n" );

  uint32_t *ptr_time = malloc( sizeof( uint32_t ) );

  ABORT_ON_ERROR( thread_create( &thread_0, 0, THREAD0_C_MS, THREAD_T_MS, ptr_time ) );

  ABORT_ON_ERROR( thread_create( &thread_1, 1, THREAD1_C_MS, THREAD_T_MS, ptr_time ) );

  printf( "Successfully created threads! Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ), "Threads are unschedulable!\n" );

  while (1);

  return 0;
}
