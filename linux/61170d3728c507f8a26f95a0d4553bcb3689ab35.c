// general protection fault in reconfigure_super
// https://syzkaller.appspot.com/bug?id=61170d3728c507f8a26f95a0d4553bcb3689ab35
// status:invalid
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
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  syscall(__NR_flistxattr, -1, 0x20000240, 0xe3);
  memcpy((void*)0x200000c0, "./file0", 8);
  syscall(__NR_mkdir, 0x200000c0, 0);
  memcpy((void*)0x20026ff8, "./file0", 8);
  memcpy((void*)0x200013c0, "ramfs", 6);
  syscall(__NR_mount, 0x200002c0, 0x20026ff8, 0x200013c0, 0, 0x200002c0);
  memcpy((void*)0x20000280, "./file0", 8);
  syscall(__NR_chroot, 0x20000280);
  memcpy((void*)0x20000040, "./file0", 8);
  syscall(__NR_umount2, 0x20000040, 0);
  return 0;
}