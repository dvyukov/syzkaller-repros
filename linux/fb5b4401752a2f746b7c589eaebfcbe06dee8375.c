// panic: invalid type: int
// https://syzkaller.appspot.com/bug?id=fb5b4401752a2f746b7c589eaebfcbe06dee8375
// status:fixed
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
  long res = 0;
  res = syscall(__NR_socket, 0x10, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000140 = 4;
  syscall(__NR_getsockopt, r[0], 1, 8, 0x20000040, 0x20000140);
  return 0;
}
