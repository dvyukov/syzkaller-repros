// KASAN: slab-out-of-bounds Read in rhashtable_walk_enter
// https://syzkaller.appspot.com/bug?id=c7c08abc80d977ee1d2ca48828269ee08858a58c
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
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 4);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000080 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0x2000d379;
  *(uint64_t*)0x2000d379 = 0x20000200;
  *(uint32_t*)0x20000200 = 0x18;
  *(uint16_t*)0x20000204 = 0x14;
  *(uint16_t*)0x20000206 = 0x301;
  *(uint32_t*)0x20000208 = 0;
  *(uint32_t*)0x2000020c = 0;
  *(uint8_t*)0x20000210 = 0x1e;
  *(uint8_t*)0x20000211 = 0;
  *(uint16_t*)0x20000212 = 0;
  memcpy((void*)0x20000214, "\xc1", 1);
  *(uint64_t*)0x2000d381 = 0x18;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000080, 0);
  return 0;
}