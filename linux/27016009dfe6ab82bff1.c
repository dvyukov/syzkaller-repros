// general protection fault in __xfrm6_tunnel_spi_lookup
// https://syzkaller.appspot.com/bug?id=27016009dfe6ab82bff1
// status:0
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
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint64_t*)0x20000180 = 0;
  *(uint32_t*)0x20000188 = 0;
  *(uint64_t*)0x20000190 = 0x20000040;
  *(uint64_t*)0x20000040 = 0;
  *(uint64_t*)0x20000048 = 0xfffffffffffffe83;
  *(uint64_t*)0x20000198 = 1;
  *(uint64_t*)0x200001a0 = 0;
  *(uint64_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001b0 = 0;
  syscall(__NR_sendmsg, -1, 0x20000180ul, 0ul);
  *(uint64_t*)0x200001c0 = 0x40000000;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000040;
  *(uint64_t*)0x20000040 = 0x20000080;
  memcpy((void*)0x20000080,
         "\x02\x02\x06\x09\x10\x00\x00\x00\x00\x00\x00\x4c\x9e\x00\x00\x00\x02"
         "\x00\x13\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x41\x05\x00"
         "\x06\x00\x20\x00\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00"
         "\xe5\x00\x08\x07\xff\x00\x00\x1f\x00\x00\x00\x00\x00\x00\x09\x20\x00"
         "\x00\x00\x00\x00\x02\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00"
         "\x00\x98\xa8\x05\x00\x05\x00\x00\x00\x00\x00\x0a",
         97);
  *(uint64_t*)0x20000048 = 0x80;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, -1, 0x200001c0ul, 0ul);
  res = syscall(__NR_socket, 0xful, 3ul, 2);
  if (res != -1)
    r[0] = res;
  syscall(__NR_sendmmsg, r[0], 0x20000180ul, 0x393ul, 0ul);
  return 0;
}