/**
 * @file    main.c
 *
 * @brief   Two threads switching with each other.
 *
 * @author  Benjamin Huang <zemingbh@andrew.cmu.edu>
 */
#include <349_threads.h>
#include <349_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/** @brief thread user space stack size - 1KB */
#define USR_STACK_WORDS 256
#define NUM_THREADS 2
#define NUM_MUTEXES 0
#define CLOCK_FREQUENCY 1

/** @brief Computation time of the task */
#define THREAD_C_MS 1
/** @brief Period of the thread */
#define THREAD_T_MS 3

//int t0, t1;
void thread_0( UNUSED void *vargp ) {
  int cnt = 0;
  while( 1 ) print_status_cnt( "0", cnt++ ); //write(1,"hello0\n",8);//
}

void thread_1( UNUSED void *vargp ) {
  int cnt = 0;
  while( 1 ) print_status_cnt( "1", cnt-- );// write(1,"hello1\n",8);//
}

void idle_thread() {
  while( 1 );
}

int main( UNUSED int argc, UNUSED char *const argv[] ) {
  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, &idle_thread, NUM_MUTEXES ) );

  printf( "Successfully initialized threads...\n" );

  ABORT_ON_ERROR( thread_create( &thread_0, 0, THREAD_C_MS, THREAD_T_MS, NULL ) );

  ABORT_ON_ERROR( thread_create( &thread_1, 1, THREAD_C_MS, THREAD_T_MS, NULL ) );

  printf( "Successfully created threads! Starting scheduler...\n" );

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ), "Threads are unschedulable!\n" );

  while (1);

  return 0;
}
