// WARNING: bad unlock balance in ovl_workdir_create (3)
// https://syzkaller.appspot.com/bug?id=48617d87ee45a631eb2e
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

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  memcpy((void*)0x20000000, "./file0\000", 8);
  syscall(__NR_mkdir, 0x20000000ul, 0ul);
  memcpy((void*)0x20000080, "./file0\000", 8);
  memcpy((void*)0x20000940, "tmpfs\000", 6);
  syscall(__NR_mount, 0ul, 0x20000080ul, 0x20000940ul, 0ul, 0ul);
  memcpy((void*)0x20000380, "./file0\000", 8);
  syscall(__NR_chdir, 0x20000380ul);
  memcpy((void*)0x20000400, "./file1\000", 8);
  syscall(__NR_mkdir, 0x20000400ul, 0ul);
  memcpy((void*)0x20000580, "./bus\000", 6);
  syscall(__NR_mkdir, 0x20000580ul, 0ul);
  memcpy((void*)0x20000300, ".\000", 2);
  syscall(__NR_mount, 0ul, 0x20000300ul, 0ul, 0x9029ul, 0ul);
  memcpy((void*)0x20000040, "./bus\000", 6);
  memcpy((void*)0x20000080, "overlay\000", 8);
  memcpy((void*)0x20000340, "upperdir=./bus,workdir=./file1,lowerdir=.", 41);
  syscall(__NR_mount, 0ul, 0x20000040ul, 0x20000080ul, 0ul, 0x20000340ul);
  return 0;
}
