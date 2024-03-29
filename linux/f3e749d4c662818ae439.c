// WARNING: kmalloc bug in bpf_check
// https://syzkaller.appspot.com/bug?id=f3e749d4c662818ae439
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint64_t*)0x20000080 = 0x200001c0;
  *(uint16_t*)0x200001c0 = 0xeb9f;
  *(uint8_t*)0x200001c2 = 1;
  *(uint8_t*)0x200001c3 = 0;
  *(uint32_t*)0x200001c4 = 0x18;
  *(uint32_t*)0x200001c8 = 0;
  *(uint32_t*)0x200001cc = 0xc;
  *(uint32_t*)0x200001d0 = 0xc;
  *(uint32_t*)0x200001d4 = 2;
  *(uint32_t*)0x200001d8 = 0;
  *(uint16_t*)0x200001dc = 0;
  *(uint8_t*)0x200001de = 0;
  *(uint8_t*)0x200001df = 0xd;
  *(uint32_t*)0x200001e0 = 0;
  *(uint8_t*)0x200001e4 = 0;
  *(uint8_t*)0x200001e5 = 0;
  *(uint64_t*)0x20000088 = 0x20000280;
  *(uint32_t*)0x20000090 = 0x26;
  *(uint32_t*)0x20000094 = 0xba;
  *(uint32_t*)0x20000098 = 8;
  res = syscall(__NR_bpf, 0x12ul, 0x20000080ul, 0x20ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000500 = 0x11;
  *(uint32_t*)0x20000504 = 3;
  *(uint64_t*)0x20000508 = 0x20000340;
  *(uint8_t*)0x20000340 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000341, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000341, 0, 4, 4);
  *(uint16_t*)0x20000342 = 0;
  *(uint32_t*)0x20000344 = 0;
  *(uint8_t*)0x20000348 = 0;
  *(uint8_t*)0x20000349 = 0;
  *(uint16_t*)0x2000034a = 0;
  *(uint32_t*)0x2000034c = 0;
  *(uint8_t*)0x20000350 = 0x95;
  *(uint8_t*)0x20000351 = 0;
  *(uint16_t*)0x20000352 = 0;
  *(uint32_t*)0x20000354 = 0;
  *(uint64_t*)0x20000510 = 0x20000380;
  memcpy((void*)0x20000380, "syzkaller\000", 10);
  *(uint32_t*)0x20000518 = 1;
  *(uint32_t*)0x2000051c = 0xba;
  *(uint64_t*)0x20000520 = 0x200003c0;
  *(uint32_t*)0x20000528 = 0;
  *(uint32_t*)0x2000052c = 0;
  memset((void*)0x20000530, 0, 16);
  *(uint32_t*)0x20000540 = 0;
  *(uint32_t*)0x20000544 = 0;
  *(uint32_t*)0x20000548 = r[0];
  *(uint32_t*)0x2000054c = 8;
  *(uint64_t*)0x20000550 = 0;
  *(uint32_t*)0x20000558 = 0;
  *(uint32_t*)0x2000055c = 0x10;
  *(uint64_t*)0x20000560 = 0x200004c0;
  *(uint32_t*)0x200004c0 = 0;
  *(uint32_t*)0x200004c4 = 0;
  *(uint32_t*)0x200004c8 = 0;
  *(uint32_t*)0x200004cc = 0;
  *(uint32_t*)0x20000568 = 0x200004d0;
  *(uint32_t*)0x2000056c = 0;
  *(uint32_t*)0x20000570 = 0;
  syscall(__NR_bpf, 5ul, 0x20000500ul, 0x78ul);
  return 0;
}
