// KASAN: slab-out-of-bounds Read in cgroup_file_notify
// https://syzkaller.appspot.com/bug?id=cac0c4e204952cf449b1
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
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  syscall(__NR_unshare, 0x8000000ul);
  syscall(__NR_shmget, 0ul, 0xfffffffffefffffful, 0x4800ul, 0x20ffc000ul);
  return 0;
}