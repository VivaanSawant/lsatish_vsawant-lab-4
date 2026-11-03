/**
 * @file 
 *
 * @brief      
 *
 * @date       
 *
 * @author     
 */

#include <unistd.h>
#include <syscall.h>

#define UNUSED __attribute__((unused))

void *sys_sbrk(UNUSED int incr){
  return NULL;
}

int sys_write(UNUSED int file, UNUSED char *ptr, UNUSED int len){
  return -1;
}

int sys_read(UNUSED int file, UNUSED char *ptr, UNUSED int len){
  return -1;
}

void sys_exit(UNUSED int status){
}