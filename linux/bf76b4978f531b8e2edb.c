// WARNING in smk_write_net4addr
// https://syzkaller.appspot.com/bug?id=bf76b4978f531b8e2edb
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
  memcpy((void*)0x20000140, "/sys/fs/smackfs/netlabel\000", 25);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000140ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_write, r[0], 0ul, 0x200001e2ul);
  return 0;
}
