// general protection fault in qp_release_pages
// https://syzkaller.appspot.com/bug?id=de69b9118a1682380586
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000000, "/dev/vmci\000", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200002c0 = 0xb0000;
  syscall(__NR_ioctl, r[0], 0x7a7, 0x200002c0ul);
  *(uint32_t*)0x20000140 = 1;
  *(uint32_t*)0x20000144 = 0;
  syscall(__NR_ioctl, r[0], 0x7a0, 0x20000140ul);
  *(uint32_t*)0x20000080 = 1;
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = -1;
  *(uint32_t*)0x2000008c = 0xc;
  *(uint64_t*)0x20000090 = 0x100;
  *(uint64_t*)0x20000098 = 2;
  *(uint64_t*)0x200000a0 = 4;
  *(uint64_t*)0x200000a8 = 0x1000ff;
  *(uint32_t*)0x200000b0 = -1;
  *(uint32_t*)0x200000b4 = 0;
  syscall(__NR_ioctl, r[0], 0x7a8, 0x20000080ul);
  return 0;
}
