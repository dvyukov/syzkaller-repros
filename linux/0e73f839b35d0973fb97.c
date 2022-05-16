// WARNING: kmalloc bug in hash_ipportnet_create
// https://syzkaller.appspot.com/bug?id=0e73f839b35d0973fb97
// status:3
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0x20000100;
  *(uint64_t*)0x20000100 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x5c;
  *(uint8_t*)0x20000244 = 2;
  *(uint8_t*)0x20000245 = 6;
  *(uint16_t*)0x20000246 = 3;
  *(uint32_t*)0x20000248 = 0xe;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 0;
  *(uint8_t*)0x20000251 = 0;
  *(uint16_t*)0x20000252 = htobe16(0);
  *(uint16_t*)0x20000254 = 5;
  *(uint16_t*)0x20000256 = 5;
  *(uint8_t*)0x20000258 = 2;
  *(uint16_t*)0x2000025c = 5;
  *(uint16_t*)0x2000025e = 4;
  *(uint8_t*)0x20000260 = 0;
  *(uint16_t*)0x20000264 = 5;
  *(uint16_t*)0x20000266 = 1;
  *(uint8_t*)0x20000268 = 6;
  *(uint16_t*)0x2000026c = 0x15;
  *(uint16_t*)0x2000026e = 3;
  memcpy((void*)0x20000270, "hash:ip,port,net\000", 17);
  *(uint16_t*)0x20000284 = 9;
  *(uint16_t*)0x20000286 = 2;
  memcpy((void*)0x20000288, "syz2\000", 5);
  *(uint16_t*)0x20000290 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x20000292, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000293, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000293, 1, 7, 1);
  *(uint16_t*)0x20000294 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20000296, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000297, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000297, 0, 7, 1);
  *(uint32_t*)0x20000298 = htobe32(0x9000000);
  *(uint64_t*)0x20000108 = 0x5c;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200000c0ul, 0ul);
  return 0;
}
