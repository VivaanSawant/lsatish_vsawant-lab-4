/** @file    mpu.h
 *
 *  @brief
 *
 *  @date
 *
 *  @author
 */
#ifndef _MPU_H_
#define _MPU_H_

#include <unistd.h>

/**
 * @brief  Returns ceiling (log_2 n).
 */
uint32_t mm_log2ceil_size(uint32_t n);

void mm_c_handler( void *psp );

int mm_region_enable(
  uint32_t region_number,
  void *base_address,
  uint8_t size_log2,
  int execute,
  int user_write_access
);
void mm_region_disable( uint32_t region_number );

void mm_init();

#endif /* _MPU_H_ */
