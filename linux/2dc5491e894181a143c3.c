// BUG: stack guard page was hit in sys_mkdir (4)
// https://syzkaller.appspot.com/bug?id=2dc5491e894181a143c3
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

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  memcpy((void*)0x20000600, "./file0\000", 8);
  syscall(__NR_mkdir, 0x20000600ul, 0ul);
  memcpy((void*)0x20000080, "./file0\000", 8);
  memcpy((void*)0x20000040, "./file0\000", 8);
  memcpy((void*)0x20000200, "incremental-fs\000", 15);
  syscall(__NR_mount, 0x20000080ul, 0x20000040ul, 0x20000200ul, 0ul, 0ul);
  {
    int i;
    for (i = 0; i < 64; i++) {
      syscall(__NR_mount, 0x20000080ul, 0x20000040ul, 0x20000200ul, 0ul, 0ul);
    }
  }
  memcpy((void*)0x200000c0, "./file0/file0\000", 14);
  syscall(__NR_mkdir, 0x200000c0ul, 0ul);
  return 0;
}
