// BUG: unable to handle kernel paging request in dput (2)
// https://syzkaller.appspot.com/bug?id=b7758cb0e1d1856940c14a3bee1220be5a922120
// status:fixed
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/dev/md0\x00", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
  *(uint8_t*)0x200002c0 = 0;
  *(uint8_t*)0x200002c1 = 0;
  *(uint8_t*)0x200002c2 = 0;
  *(uint8_t*)0x200002c3 = 0;
  *(uint8_t*)0x200002c4 = 0;
  *(uint8_t*)0x200002c5 = 0;
  *(uint8_t*)0x200002c6 = 0;
  *(uint8_t*)0x200002c7 = 0;
  *(uint8_t*)0x200002c8 = 0;
  *(uint8_t*)0x200002c9 = 0;
  *(uint8_t*)0x200002ca = 0;
  *(uint8_t*)0x200002cb = 0;
  *(uint8_t*)0x200002cc = 0;
  *(uint8_t*)0x200002cd = 0;
  *(uint8_t*)0x200002ce = 0;
  *(uint8_t*)0x200002cf = 0;
  *(uint8_t*)0x200002d0 = 0;
  *(uint8_t*)0x200002d1 = 0;
  *(uint8_t*)0x200002d2 = 0;
  *(uint8_t*)0x200002d3 = 0;
  *(uint8_t*)0x200002d4 = 0;
  *(uint8_t*)0x200002d5 = 0;
  *(uint8_t*)0x200002d6 = 0;
  *(uint8_t*)0x200002d7 = 0;
  *(uint8_t*)0x200002d8 = 0;
  *(uint8_t*)0x200002d9 = 0;
  *(uint8_t*)0x200002da = 0;
  *(uint8_t*)0x200002db = 0;
  *(uint8_t*)0x200002dc = 0;
  *(uint8_t*)0x200002dd = 0;
  *(uint8_t*)0x200002de = 0;
  *(uint8_t*)0x200002df = 0;
  *(uint16_t*)0x200002e0 = 0;
  *(uint32_t*)0x200002e4 = 0xfffffcf0;
  *(uint32_t*)0x200002e8 = 0x96e;
  *(uint64_t*)0x200002f0 = 0;
  *(uint64_t*)0x200002f8 = 0;
  *(uint32_t*)0x20000300 = 0;
  syscall(__NR_ioctl, r[0], 0xc0481273, 0x200002c0);
  return 0;
}