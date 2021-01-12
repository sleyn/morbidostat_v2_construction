/* COPYRIGHT (c) 2019 - 2020 DIVO Systems, Inc.
 * Developed by DIVO Systems, Inc.   http://www.divo.com */

#define EXTERN
#include "common.h"

UINT32 chksum(UINT8 *data, UINT32 len)
{
  unsigned int i;
  UINT32 sum = 0;
  for (i = 0; i < len; i++) sum += ((UINT32)*data++) << (8 * (i & 3));
  return(sum);
}

void Reset(void)
{
  asm volatile ("jmp 0");
}