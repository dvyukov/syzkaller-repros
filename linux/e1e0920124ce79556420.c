// kernel BUG in notify_change
// https://syzkaller.appspot.com/bug?id=e1e0920124ce79556420
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

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  memcpy((void*)0x20000100, "./file0\000", 8);
  syscall(__NR_mkdir, 0x20000100ul, 0ul);
  memcpy((void*)0x20000300, "./file0/file0\000", 14);
  syscall(__NR_creat, 0x20000300ul, 0xd929c48218614f6dul);
  memcpy((void*)0x20000240, "./file0\000", 8);
  memcpy((void*)0x20000040, "./file0\000", 8);
  memcpy((void*)0x20000200, "incremental-fs\000", 15);
  syscall(__NR_mount, 0x20000240ul, 0x20000040ul, 0x20000200ul, 0ul, 0ul);
  memcpy((void*)0x20000000, "./file0/file0\000", 14);
  syscall(__NR_lchown, 0x20000000ul, 0, 0);
  return 0;
}
