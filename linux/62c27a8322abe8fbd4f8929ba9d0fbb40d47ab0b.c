// INFO: rcu detected stall in sys_sendfile64
// https://syzkaller.appspot.com/bug?id=62c27a8322abe8fbd4f8929ba9d0fbb40d47ab0b
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

#ifndef __NR_sched_setattr
#define __NR_sched_setattr 314
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 6;
  *(uint64_t*)0x20000008 = 0;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint64_t*)0x20000018 = 0x8000000009917;
  *(uint64_t*)0x20000020 = 0x400000000000fffd;
  *(uint64_t*)0x20000028 = 0;
  syscall(__NR_sched_setattr, 0, 0x20000000, 0);
  memcpy((void*)0x20000080, "./bus\x00", 6);
  res = syscall(__NR_open, 0x20000080, 0x1fe, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20006c40, "\xff\x07\x00\xc1\x34", 5);
  syscall(__NR_write, r[0], 0x20006c40, 5);
  *(uint64_t*)0x20000000 = 0;
  syscall(__NR_sendfile, r[0], r[0], 0x20000000, 0x8080fffffffe);
  return 0;
}