/**
 * @file   main.c
 *
 * @brief Full PCP test.
 * T0: (500, 2500), S1(100-500)
 * T1: (200, 3000)
 * T2: (500, 3300), S1(0-300), S2(200-500)
 * T3: (500, 4000), S3(100-400)
 * T4: (500, 6300), S2(200-500)
 * T5: (700, 8500), S3(0-700)
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
#define NUM_THREADS 6
#define NUM_MUTEXES 3
#define CLOCK_FREQUENCY 500

/** @brief Computation time of the task */
static const int THREAD_C_MS[] = { 500, 200, 500, 500, 500, 700 };
/** @brief Period of the thread */
static const int THREAD_T_MS[] = { 2500, 3000, 3300, 4000, 6300, 8500 };

#define REDUCE_SPIN_MS 2

typedef struct {
  mutex_t *mutex_0;
  mutex_t *mutex_2;
  mutex_t *mutex_3;
} arg_t;

/** @brief Debug print a single char
 *
 *  @param c   the character
 */
void print_char( int c ) {
  putchar( c );
#ifdef TEST_DEBUG
  printf( "--Prio %d, t=%d\n", ( int )get_priority(), ( int )get_time() );
#endif
}

/** @brief Debug print a single char
 *
 *  @param c   the character
 */
void print_wrap( char *s ) {
  printf( s );
#ifdef TEST_DEBUG
  printf( "--Prio %d, t=%d\n", ( int )get_priority(), ( int )get_time() );
#endif
}

/** @brief Default idle thread which just loops infinitely
 */
void idle_thread() {
  spin_until( 12100 );

  while ( 1 );
}

/** @brief T0: (500, 2500), S1(100-500)
 */
void thread_0( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;

  // Period 0

  printf( "==== Lab 3 PCP Test ====\n" );
  print_wrap( "Y" ); 
  spin_wait( 100 - REDUCE_SPIN_MS ); 
  mutex_lock( arg->mutex_0 );
  print_wrap( "ou " );
  spin_wait( 400 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_0 );
  print_char( 'n' );
  wait_until_next_period();

  // Period 1

  print_wrap( "a" );
  spin_wait( 100 - REDUCE_SPIN_MS ); 
  mutex_lock( arg->mutex_0 );
  print_wrap( "wli" );
  spin_wait( 400 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_0 );
  print_char( 'n' );
  spin_wait( REDUCE_SPIN_MS * 3 );

  // Period 2

  print_wrap( "s" ); 
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_0 );
  print_wrap( "had" );
  spin_wait( 400 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_0 );
  print_char( 'o' );
  wait_until_next_period();

  // Period 3

  print_wrap( "a" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_0 );
  print_wrap( "ppr" );
  spin_wait( 400 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_0 );
  print_char( 'e' );
  wait_until_next_period();

  // Period 4

  print_wrap( "t" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_0 );
  print_wrap( "o s" );
  spin_wait( 400 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_0 );
  print_char( 't' );
  wait_until_next_period();
}

/** @brief T1: (200, 3000)
 */
void thread_1( UNUSED void *vargp ) {
  const unsigned int my_C = THREAD_C_MS[1];
  // Period 0

  print_wrap( "ee" );
  spin_wait( my_C - REDUCE_SPIN_MS );
  wait_until_next_period();

  // Period 1

  print_wrap( "g " );
  spin_wait( my_C + REDUCE_SPIN_MS );

  // Period 2

  print_wrap( "ws" );
  spin_wait( my_C - 2 * REDUCE_SPIN_MS );
  wait_until_next_period();

  // Period 3

  print_wrap( " i" );
  spin_wait( my_C - REDUCE_SPIN_MS );
  wait_until_next_period();

  printf( "d in the sun.\n" );
}


/** @brief T2: (500, 3300), S1(0-300), S2(200-500)
 */
void thread_2( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;

  // Period 0
  mutex_lock( arg->mutex_0 );
  print_wrap( "d " );
  spin_wait( 200 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_2 );
  print_wrap( "t" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_0 );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( "o " );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_2 );
  wait_until_next_period();

  // Period 1
  mutex_lock( arg->mutex_0 );
  print_wrap( "lo" );
  spin_wait( 200 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_2 );
  print_wrap( "n" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_0 );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( "e " );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_2 );
  wait_until_next_period();

  // Period 2
  mutex_lock( arg->mutex_0 );
  print_wrap( "ru" );
  spin_wait( 200 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_2 );
  print_wrap( "l" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_0 );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( "y " );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_2 );
  wait_until_next_period();

  // Period 3
  mutex_lock( arg->mutex_0 );
  print_wrap( "i" );
  spin_wait( 200 - REDUCE_SPIN_MS );
  print_wrap( "s" );
  spin_wait( REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_2 );
  print_wrap( " " );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_0 );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( "an" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_2 );
  wait_until_next_period();
}

/** @brief T3: (500, 4000), S3(100-400)
 */
void thread_3( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;

  // Period 0
  print_wrap( "s" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_3 );
  print_wrap( "pen" );
  spin_wait( 300 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_3 );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( "d" );
  wait_until_next_period();

  // Period 1
  print_wrap( "o" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_3 );
  print_wrap( "ugh" );
  spin_wait( 300 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_3 );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( " " );
  wait_until_next_period();

  // Period 2
  print_wrap( "c" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_3 );
  print_wrap( "iat" );
  spin_wait( 300 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_3 );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( "e" );
  wait_until_next_period();
}

/** @brief T4: (500, 6300), S2(200-500)
 */
void thread_4( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;

  // Period 0

  print_wrap( " t" );
  spin_wait( 200 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_2 );
  print_wrap( "i" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( "m" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( "e" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_2 );
  wait_until_next_period();

  // Period 1

  print_wrap( "\nt" );
  spin_wait( 200 - REDUCE_SPIN_MS );
  mutex_lock( arg->mutex_2 );
  print_wrap( "o" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( " " );
  spin_wait( 100 - REDUCE_SPIN_MS ); 
  print_wrap( "t" ); 
  spin_wait( 100 - REDUCE_SPIN_MS ); 
  mutex_unlock( arg->mutex_2 ); 
  wait_until_next_period();
}

/** @brief T5: (700, 8500), S3(0-700)
 */
void thread_5( void *vargp ) {
  arg_t *arg = ( arg_t * )vargp;

  // Period 0

  mutex_lock( arg->mutex_3 );
  print_wrap( " cr" ); 
  spin_wait( 350 - REDUCE_SPIN_MS );
  print_wrap( "a" ); 
  spin_wait( 100 - REDUCE_SPIN_MS ); 
  print_wrap( "t" ); 
  spin_wait( 100 - REDUCE_SPIN_MS ); 
  print_wrap( "h" );
  spin_wait( 100 - REDUCE_SPIN_MS ); 
  print_wrap( "r" );
  spin_wait( 50 - REDUCE_SPIN_MS ); 
  mutex_unlock( arg->mutex_3 );
  wait_until_next_period();

  // Period 1
  mutex_lock( arg->mutex_3 );
  print_wrap( " wh" );
  spin_wait( 350 - REDUCE_SPIN_MS );
  print_wrap( "a" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( "t" );
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( "t" ); 
  spin_wait( 100 - REDUCE_SPIN_MS );
  print_wrap( " " );
  spin_wait( 50 - REDUCE_SPIN_MS );
  mutex_unlock( arg->mutex_3 );
  wait_until_next_period();
}

/** @brief Thread functions */
static const void *THREAD_FNS[] = {
  &thread_0,
  &thread_1,
  &thread_2,
  &thread_3,
  &thread_4,
  &thread_5
};

int main() {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, &idle_thread, NUM_MUTEXES ) );

  mutex_t *mutex_0 = mutex_init( 0 );
  if ( mutex_0 == NULL ) {
    printf( "Failed to create mutex\n" );
    return -1;
  }

  mutex_t *mutex_2 = mutex_init( 2 );
  if ( mutex_2 == NULL ) {
    printf( "Failed to create mutex\n" );
    return -1;
  }

  mutex_t *mutex_3 = mutex_init( 3 );
  if ( mutex_3 == NULL ) {
    printf( "Failed to create mutex\n" );
    return -1;
  }

  arg_t *arg = malloc( sizeof( arg_t ) );
  if ( arg == NULL ) {
    printf( "Malloc on arg failed\n" );
    return -1;
  }
  arg->mutex_0 = mutex_0;
  arg->mutex_2 = mutex_2;
  arg->mutex_3 = mutex_3;

  for ( int i = 0; i < NUM_THREADS; i++ ) {
    ABORT_ON_ERROR( thread_create( THREAD_FNS[i], i, THREAD_C_MS[i], THREAD_T_MS[i], ( void * )arg ),
      "thread %d\n", i
    );
  }

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return RET_0349;
}
