// WARNING in smk_set_cipso
// https://syzkaller.appspot.com/bug?id=438aafc66532f0dba169
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000000, "/sys/fs/smackfs/cipso2\000", 23);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20014b00,
         "]\225\371\206P\306\323\210\027\036\200\355g\376\371/"
         "\'\2652\220\024!\352\263d\353\211u\276\334("
         "\277\2568\226\265\335\331\223\274\023\246I1\230}<"
         "\017\b\236v\252\202\307\364\366\236e\233\227\352\261v\251/P\313",
         67);
  *(uint8_t*)0x20014b43 = 0x20;
  sprintf((char*)0x20014b44, "%020llu", (long long)0);
  *(uint8_t*)0x20014b58 = 0x20;
  sprintf((char*)0x20014b59, "%020llu", (long long)0);
  *(uint8_t*)0x20014b6d = 0x20;
  *(uint8_t*)0x20014b6e = 0;
  syscall(__NR_write, r[0], 0x20014b00ul, 0x20014b6ful);
  return 0;
}
