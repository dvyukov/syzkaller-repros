// general protection fault in j1939_priv_get_by_ndev_locked
// https://syzkaller.appspot.com/bug?id=145aa40955f1b9c9aa8598fe15bf3f7eca4d4ee9
// status:open
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
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  memcpy((void*)0x20000140, "/dev/net/tun\000", 13);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000140, 0x400000001, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000,
         "nr0\001\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint16_t*)0x20000010 = 0x7132;
  syscall(__NR_ioctl, r[0], 0x400454ca, 0x20000000);
  syscall(__NR_ioctl, r[0], 0x400454cd, 0x118);
  return 0;
}