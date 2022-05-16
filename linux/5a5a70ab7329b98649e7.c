// WARNING: kmalloc bug in hash_ipmark_create
// https://syzkaller.appspot.com/bug?id=5a5a70ab7329b98649e7
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
  *(uint64_t*)0x20000100 = 0x20000140;
  *(uint32_t*)0x20000140 = 0x58;
  *(uint8_t*)0x20000144 = 2;
  *(uint8_t*)0x20000145 = 6;
  *(uint16_t*)0x20000146 = 0x801;
  *(uint32_t*)0x20000148 = 0x6c;
  *(uint32_t*)0x2000014c = 0;
  *(uint8_t*)0x20000150 = 0;
  *(uint8_t*)0x20000151 = 0;
  *(uint16_t*)0x20000152 = htobe16(0);
  *(uint16_t*)0x20000154 = 5;
  *(uint16_t*)0x20000156 = 5;
  *(uint8_t*)0x20000158 = 0xa;
  *(uint16_t*)0x2000015c = 5;
  *(uint16_t*)0x2000015e = 1;
  *(uint8_t*)0x20000160 = 6;
  *(uint16_t*)0x20000164 = 5;
  *(uint16_t*)0x20000166 = 4;
  *(uint8_t*)0x20000168 = 0;
  *(uint16_t*)0x2000016c = 9;
  *(uint16_t*)0x2000016e = 2;
  memcpy((void*)0x20000170, "syz0\000", 5);
  *(uint16_t*)0x20000178 = 0x11;
  *(uint16_t*)0x2000017a = 3;
  memcpy((void*)0x2000017c, "hash:ip,mark\000", 13);
  *(uint16_t*)0x2000018c = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x2000018e, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000018f, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000018f, 1, 7, 1);
  *(uint16_t*)0x20000190 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20000192, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000193, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000193, 0, 7, 1);
  *(uint32_t*)0x20000194 = htobe32(0x233c0003);
  *(uint64_t*)0x20000108 = 0x58;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200000c0ul, 0ul);
  return 0;
}
