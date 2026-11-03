/**
 * @file   main.c
 *
 * @brief  Test stack overflow detection and aborting.
 */

#include <349_lib.h>
#include <349_threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 1
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 200

#define UNUSED __attribute__((unused))

uint32_t multiply7( uint32_t n ) {
  if ( n < 1 ) {
    return 0;
  }
  else {
    return multiply7( n - 1 ) + 7;
  }
}

// Attempt stack overflow via recursion
void tayato_function( void *vargp ) {
  uint32_t end = *( uint32_t * )vargp;
  for (uint32_t i = 0; i < end; i++) {
    printf( "7 * %lu = %lu\n", i, multiply7( i ) );
  }
}

int main( int argc, char *const argv[] ) {
  int n = 0;
  int s = 256;
  int opt;

  while ( ( opt = getopt( argc, argv, "n:s:" ) ) != -1 ) {
    switch ( opt ) {
    case 'n':
      n = atoi( optarg );
      break;

    case 's':
      s = atoi( optarg );
      break;

    default:
      return -1;
    }
  }

  ABORT_ON_ERROR( thread_init( NUM_THREADS, s, NULL, NUM_MUTEXES ) );

  int vargp = n;
  ABORT_ON_ERROR( thread_create( &tayato_function, 0, 400, 500, &vargp ) );

  printf( "Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );


  return 0;
}
