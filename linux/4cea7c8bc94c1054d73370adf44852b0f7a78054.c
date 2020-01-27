// KASAN: null-ptr-deref Read in insert_char
// https://syzkaller.appspot.com/bug?id=4cea7c8bc94c1054d73370adf44852b0f7a78054
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  memcpy((void*)0x20000180, "/dev/fb0\000", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000180ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0x800;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint32_t*)0x20000018 = 8;
  *(uint32_t*)0x2000001c = 0;
  *(uint32_t*)0x20000020 = 0;
  *(uint32_t*)0x20000024 = 0;
  *(uint32_t*)0x20000028 = 0;
  *(uint32_t*)0x2000002c = 0;
  *(uint32_t*)0x20000030 = 0;
  *(uint32_t*)0x20000034 = 0;
  *(uint32_t*)0x20000038 = 0;
  *(uint32_t*)0x2000003c = 0;
  *(uint32_t*)0x20000040 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint32_t*)0x20000050 = 0;
  *(uint32_t*)0x20000054 = 0;
  *(uint32_t*)0x20000058 = 0;
  *(uint32_t*)0x2000005c = 0;
  *(uint32_t*)0x20000060 = 0;
  *(uint32_t*)0x20000064 = 0;
  *(uint32_t*)0x20000068 = 0;
  *(uint32_t*)0x2000006c = 0;
  *(uint32_t*)0x20000070 = 0;
  *(uint32_t*)0x20000074 = 0;
  *(uint32_t*)0x20000078 = 0;
  *(uint32_t*)0x2000007c = 0;
  *(uint32_t*)0x20000080 = 0;
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = 0;
  *(uint32_t*)0x20000098 = 0;
  *(uint32_t*)0x2000009c = 0;
  syscall(__NR_ioctl, r[0], 0x4601ul, 0x20000000ul);
  res = syz_open_dev(0xc, 4, 0x14);
  if (res != -1)
    r[1] = res;
  *(uint8_t*)0x20000000 = 0xe;
  *(uint8_t*)0x20000001 = 0x9b;
  *(uint8_t*)0x20000002 = 0x44;
  *(uint8_t*)0x20000003 = 0x9b;
  *(uint8_t*)0x20000004 = 0;
  *(uint8_t*)0x20000005 = 0;
  *(uint8_t*)0x20000006 = 0;
  *(uint8_t*)0x20000007 = 0;
  *(uint64_t*)0x20000008 = 0;
  *(uint16_t*)0x20000010 = 0;
  *(uint16_t*)0x20000012 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint64_t*)0x20000018 = 0;
  *(uint64_t*)0x20000020 = 0x40;
  *(uint64_t*)0x20000028 = 0;
  *(uint32_t*)0x20000030 = 0;
  *(uint16_t*)0x20000034 = 0;
  *(uint16_t*)0x20000036 = 0x38;
  *(uint16_t*)0x20000038 = 0;
  *(uint16_t*)0x2000003a = 0;
  *(uint16_t*)0x2000003c = 0;
  *(uint16_t*)0x2000003e = 0;
  *(uint32_t*)0x20000040 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint64_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint64_t*)0x20000070 = 0;
  syscall(__NR_write, r[1], 0x20000000ul, 0x78ul);
  return 0;
}