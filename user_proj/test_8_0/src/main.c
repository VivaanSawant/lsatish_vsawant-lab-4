/**
 * @file   main.c
 *
 * @brief Written Question 1.
 * T0: (400, 1100), S1(200-400)
 * T1: (500, 1600)
 * T2: (300, 3100), S1(0-300)
 * Expected string: LABORTLABOLARTLABO
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

/** @brief thread user space stack size - 4KB */
#define USR_STACK_WORDS 256
#define NUM_THREADS 3
#define NUM_MUTEXES 1
#define CLOCK_FREQUENCY 200

/** @brief How much to reduce spin wait */
#define REDUCE_SPIN_MS 2

//#define TEST_DEBUG

/** @brief Debug print a single char
 *
 *  @param c   the character
 */
void print_char(int c) {
  putchar(c);
#ifdef TEST_DEBUG
  printf(" - Prio: %lu \t Time: %lu\n", get_priority(), get_time());
#endif
}

/** @brief T0: (400, 1100), S1(200-400)
 */
void thread_0(void* vargp) {
  mutex_t* m0 = (mutex_t*)vargp;
  int cnt = 0;
  while (cnt < 4) {
    print_char('L');
    spin_wait(200);
    mutex_lock(m0);
    print_char('A');
    spin_wait(190);
    mutex_unlock(m0);
    cnt++;
    wait_until_next_period();
  }
}

/** @brief T1: (500, 1600)
 */
void thread_1(UNUSED void* vargp) {
  int cnt = 0;

  while (cnt < 3) {
    print_char('B');
    spin_wait(490);
    print_char('O');
    cnt++;
    wait_until_next_period();

  }
  printf(" - Obtained\n");
  printf("LABORTLABOLARTLABO - Expected\n");
}

/** @brief T2: (300, 3100), S1(0-300)
 */
void thread_2(void* vargp) {
  mutex_t* m0 = (mutex_t*)vargp;
  int cnt = 0;
  while (cnt < 2) {
    mutex_lock(m0);
    print_char('R');
    spin_wait(285);
    print_char('T');
    mutex_unlock(m0);
    cnt++;
    wait_until_next_period();
  }
}

int main() {

  ABORT_ON_ERROR( thread_init( NUM_THREADS, USR_STACK_WORDS, NULL, NUM_MUTEXES ) );

  mutex_t* s0 = mutex_init(0);
  if (s0 == NULL){
    printf("Failed to create mutex 0\n");
    return -1;
  }

  ABORT_ON_ERROR( thread_create( &thread_0, 0, 400, 1100, ( void * )s0 ) );
  ABORT_ON_ERROR( thread_create( &thread_1, 1, 500, 1600, ( void * )s0 ) );
  ABORT_ON_ERROR( thread_create( &thread_2, 2, 300, 3100, ( void * )s0 ) );

  printf("Starting scheduler...\n");

  ABORT_ON_ERROR( scheduler_start( CLOCK_FREQUENCY ) );

  return 0;
}
