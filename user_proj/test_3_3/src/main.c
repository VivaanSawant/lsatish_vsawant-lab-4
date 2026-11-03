/**
 * @file   main.c
 *
 * @brief  Test for rate-monotonic scheduling, with preemption
 *         T0 (40, 410); T1 (300, 800); T2 (350, 1200)
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
#define NUM_THREADS 3
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 1000

/** @brief Give some leeway when testing */
#define SLACK_MS 5

/** @brief Computation time of the task */
static const int THREAD_C_MS[] = { 40, 300, 350 };
/** @brief Period of the thread */
static const int THREAD_T_MS[] = { 410, 800, 1200 };

volatile int done_flag;

/** @brief T0 (20, 410)
 */
void thread_0( UNUSED void *vargp ) {
  static const int NUM = 0;
  int cnt = 0;

  while( 1 ) {

    int t = get_time();

    if ( t < THREAD_T_MS[NUM] * cnt || t > THREAD_T_MS[NUM] * cnt + SLACK_MS ) {
      print_num_status_cnt( NUM, cnt );
      printf( "Thread %d should only wake up at t = %d.\n", NUM, THREAD_T_MS[NUM] * cnt );
      printf( "Test failed.\n" );
      break;
    }

    print_num_status_cnt( NUM, cnt++ );

    if ( cnt > 15 ) {
      printf( "Test passed!\n" );
      break;
    }

    spin_wait( THREAD_C_MS[NUM] - SLACK_MS );
    wait_until_next_period();

  }

  done_flag = 1;
  while ( 1 ) wait_until_next_period();

}

void thread_fn( void *vargp ) {
  int cnt = 0;
  int num = ( int )vargp;

  while( 1 ) {
    if ( done_flag ) {
      while ( 1 ) wait_until_next_period();
    }

    print_num_status_cnt( num, cnt++ );
    spin_wait( THREAD_C_MS[num] - SLACK_MS );
    wait_until_next_period();
  }

}

int main( void ) {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ) );

  ABORT_ON_ERROR( thread_create( &thread_0, 0, THREAD_C_MS[0], THREAD_T_MS[0], NULL ) );

  for ( int i = 1; i < NUM_THREADS; i++ ) {
    ABORT_ON_ERROR( thread_create( &thread_fn, i, THREAD_C_MS[i], THREAD_T_MS[i], ( void * )i ),
      "Failed to create thread %d\n", i
    );
  }

  printf( "Successfully created threads! Starting scheduler...\n" );
  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return 0;
}
