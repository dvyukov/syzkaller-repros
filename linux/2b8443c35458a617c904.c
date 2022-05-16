// WARNING: kmalloc bug in hash_net_create
// https://syzkaller.appspot.com/bug?id=2b8443c35458a617c904
// status:6
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
  *(uint32_t*)0x200000c8 = 0x6000c0;
  *(uint64_t*)0x200000d0 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x20000040;
  *(uint32_t*)0x20000040 = 0x54;
  *(uint8_t*)0x20000044 = 2;
  *(uint8_t*)0x20000045 = 6;
  *(uint16_t*)0x20000046 = 0x101;
  *(uint32_t*)0x20000048 = 0x300;
  *(uint32_t*)0x2000004c = 0;
  *(uint8_t*)0x20000050 = 0;
  *(uint8_t*)0x20000051 = 0;
  *(uint16_t*)0x20000052 = htobe16(0);
  *(uint16_t*)0x20000054 = 5;
  *(uint16_t*)0x20000056 = 5;
  *(uint8_t*)0x20000058 = 0xa;
  *(uint16_t*)0x2000005c = 9;
  *(uint16_t*)0x2000005e = 2;
  memcpy((void*)0x20000060, "syz0\000", 5);
  *(uint16_t*)0x20000068 = 5;
  *(uint16_t*)0x2000006a = 4;
  *(uint8_t*)0x2000006c = 0;
  *(uint16_t*)0x20000070 = 5;
  *(uint16_t*)0x20000072 = 1;
  *(uint8_t*)0x20000074 = 6;
  *(uint16_t*)0x20000078 = 0xd;
  *(uint16_t*)0x2000007a = 3;
  memcpy((void*)0x2000007c, "hash:net\000", 9);
  *(uint16_t*)0x20000088 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x2000008a, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000008b, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000008b, 1, 7, 1);
  *(uint16_t*)0x2000008c = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000008e, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000008f, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000008f, 0, 7, 1);
  *(uint32_t*)0x20000090 = htobe32(0x580c02a0);
  *(uint64_t*)0x20000148 = 0x54;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200000c0ul, 0ul);
  return 0;
}