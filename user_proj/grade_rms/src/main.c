/**
 * @file   main.c
 *
 * @brief Full RMS test.
 * T0: (30, 310)
 * T1: (20, 330)
 * T2: (40, 350)
 * T3: (40, 470)
 * T4: (50, 510)
 * T5: (40, 520)
 * T6: (60, 890)
 * T7: (50, 1020)
 *
 * @author Benjamin Huang <zemingbh@andrew.cmu.edu>
 */

#include <349_lib.h>
#include <349_threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define UNUSED __attribute__((unused))

/** @brief thread user space stack size - 4KB */
#define USR_STACK_WORDS 256
#define NUM_THREADS 8
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 1000

//#define RMS_DEBUG

/** @brief Computation time of the task */
static const int THREAD_C_MS[] = { 300, 200, 400, 400, 500, 400, 600, 500 };
/** @brief Period of the thread */
static const int THREAD_T_MS[] = { 3100, 3300, 3500, 4700, 5100, 5200, 8900, 10200 };

int idle_done = 0;

void printf_wrap( char *s ) {
#ifdef RMS_DEBUG
  printf( "t=%d, Thread %d: %s\n", ( int )getpid(), ( int )get_time(), s );
#else
  printf( s );
#endif
}

/** @brief Idle thread
 */
void idle_thread() {
  spin_until( 4100 );
  printf_wrap( " prova" );
  spin_until( 7100 );

  if ( get_time() > 7300 ) {
    printf( "Idle thread failed.\n" );
  }

  printf_wrap( "eduling algori" );
  idle_done = 1;

  while( 1 );
}

/** @brief T0: (300, 3100)
 */
void thread_0( void *vargp ) {
  uint32_t i = ( uint32_t )vargp;
  const unsigned int my_C = THREAD_C_MS[i];
  // const unsigned int my_T = 3100;

  // Period 0
  printf( "=== What do you think of RMS? ===\n" );
  printf_wrap( "I l" );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 1
  printf_wrap( "g! " );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 2
  printf_wrap( "tat" );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 3
  printf_wrap( " ED" );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  while ( !idle_done );

}

/** @brief T1: (200, 3300), doesn't call wait_until_next_period
 */
void thread_1( UNUSED void *vargp ) {
  uint32_t i = ( uint32_t )vargp;
  const unsigned int my_C = THREAD_C_MS[i];
  // const unsigned int my_T = 3300;

  // Period 0
  printf_wrap( "ov" );
  spin_wait( my_C + 2 );
  // Test miss deadline

  // Period 1
  printf_wrap( "It" );
  spin_wait( my_C - 6 );
  wait_until_next_period();

  // Period 2
  printf_wrap( "ic" );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 3
  printf_wrap( "s " );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  while ( !idle_done );

}


/** @brief T2: (400, 3500)
 */
void thread_2( UNUSED void *vargp ) {
  uint32_t i = ( uint32_t )vargp;
  const unsigned int my_C = THREAD_C_MS[i];
  // const unsigned int my_T = 3500;

  // Period 0
  printf_wrap( "e ra" );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 1
  printf_wrap( " is " );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 2
  printf_wrap( " sch" );
  wait_until_next_period();

  // Period 3
  printf_wrap( "er i" );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  while ( !idle_done );

}

/** @brief T3: (400, 4700)
 */
void thread_3( UNUSED void *vargp ) {
  uint32_t i = ( uint32_t )vargp;
  const unsigned int my_C = THREAD_C_MS[i];
  // const unsigned int my_T = 4700;

  // Period 0
  printf_wrap( "te m" );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 1
  printf_wrap( "bly " );
  spin_wait( my_C - 5 );
  wait_until_next_period();


  // Period 2
  printf_wrap( "F i" );
  spin_wait( my_C + 2 ); // Test spin wait spinning only when running
  printf_wrap( "==END==\n" );
  wait_until_next_period();

  while ( !idle_done );

}

/** @brief T4: (500, 5100)
 */
void thread_4( UNUSED void *vargp ) {
  uint32_t i = ( uint32_t )vargp;
  const unsigned int my_C = THREAD_C_MS[i];
  // const unsigned int my_T = 5100;

  // Period 0
  printf_wrap( "onoto" );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 1
  printf_wrap( "optim" );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 2
  printf_wrap( "bett" );
  spin_wait( 310 );
  printf_wrap( "f " );
  spin_wait( 180 );
  wait_until_next_period();

  while ( !idle_done );

}

/** @brief T5: (400, 5200)
 */
void thread_5( UNUSED void *vargp ) {
  uint32_t i = ( uint32_t )vargp;
  const unsigned int my_C = THREAD_C_MS[i];
  // const unsigned int my_T = 5200;

  // Period 0
  printf_wrap( "nic " );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 1
  printf_wrap( "al\ns" );
  spin_wait( my_C - 15 );
  wait_until_next_period();

  // Period 2
  printf_wrap( "you " );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  while ( !idle_done );

}

/** @brief T6: (600, 8900)
 */
void thread_6( UNUSED void *vargp ) {
  uint32_t i = ( uint32_t )vargp;
  const unsigned int my_C = THREAD_C_MS[i];
  // const unsigned int my_T = 8900;

  // Period 0
  printf_wrap( "schedu" );
  spin_wait( my_C - 5 );
  wait_until_next_period();

  // Period 1
  printf_wrap( "thm." );
  spin_wait( 410 );
  printf_wrap( "al" );
  spin_wait( 180 );
  wait_until_next_period();

  while ( !idle_done );

}

/** @brief T7: (500, 10200)
 */
void thread_7( UNUSED void *vargp ) {
  // const unsigned int my_C = 500;
  // const unsigned int my_T = 10200;

  // Period 0
  printf_wrap( "lin" );
  spin_wait( 480 );
  printf_wrap( "a" );
  wait_until_next_period();

  // Period 1
  printf_wrap( "low\ndynamic scheduling.\n" );
  wait_until_next_period();

  while ( !idle_done );

}

/** @brief Thread functions */
static const void *THREAD_FNS[] = {
  &thread_0,
  &thread_1,
  &thread_2,
  &thread_3,
  &thread_4,
  &thread_5,
  &thread_6,
  &thread_7
};

int main() {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, &idle_thread, NUM_MUTEXES ) );

  for ( int i = 0; i < NUM_THREADS; i++ ) {
    ABORT_ON_ERROR( thread_create( THREAD_FNS[i], i, THREAD_C_MS[i], THREAD_T_MS[i], ( void * )i ),
      "thread %d\n", i
    );
  }

  printf( "Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return RET_0349;
}
