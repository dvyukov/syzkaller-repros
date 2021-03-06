// WARNING in dma_buf_vunmap
// https://syzkaller.appspot.com/bug?id=e5aec54f6af0730f1ebfb6fbf4e53409ae0a5c87
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000140, "/dev/vbi#\000", 10);
  res = syz_open_dev(0x20000140, 3, 2);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = 8;
  *(uint32_t*)0x20000004 = 5;
  *(uint32_t*)0x20000008 = 4;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000010 = 0;
  syscall(__NR_ioctl, r[0], 0xc0145608, 0x20000000);
  *(uint32_t*)0x20000180 = 0;
  *(uint32_t*)0x20000184 = 6;
  *(uint32_t*)0x20000188 = 1;
  *(uint32_t*)0x20000190 = 5;
  memcpy((void*)0x20000198,
         "\xef\xaa\x91\xe8\x61\xaa\x94\x31\xed\x12\x53\x26\x34\x99\xa0\xb1\xb3"
         "\x70\xc4\x3f\x9f\xe8\xf2\x92\x73\x37\xa7\xd3\x98\xc6\x84\x5c\xb3\x81"
         "\x71\x58\x9b\x15\xe8\xd8\xb6\x1d\xc2\xe5\x42\x09\x99\x4f\xec\x06\x62"
         "\xed\x54\xb2\xba\xa4\x44\x04\xf4\x85\x09\x35\x6f\x73\x68\x9f\xfe\xe8"
         "\xd5\xb5\x90\x07\x44\x88\x9c\x50\x14\x20\x72\x4e\xa7\x2c\x48\x2a\x9e"
         "\x0d\xd0\xfa\x70\x16\x29\x10\x6a\xb5\x73\xc9\x4c\x82\xfa\x97\x4e\x2c"
         "\xc0\xb7\x4e\x5e\xb5\xd3\xdf\x1e\xda\x89\x58\x1c\x04\x5a\xe2\x8a\x41"
         "\xb0\x14\xd3\x28\xdb\x1e\xc6\x97\x73\x71\x7d\x52\xc3\x47\xe1\xab\x29"
         "\x30\x32\x68\x28\x11\xcc\x8c\x1a\x94\x3e\x4f\xf9\x9d\x64\x13\xe8\x57"
         "\x40\x95\x34\xcb\xeb\x3d\x5c\x2c\xef\x81\x43\xea\x24\x61\x30\x61\xac"
         "\xa7\xb8\x3d\x6a\x2d\xbb\x62\x3a\xa0\xcb\x70\x93\xe6\x81\x8c\x91\x77"
         "\x10\x01\x2a\x3b\xe5\x04\x41\x2c\xf3\xdc\x2d\xc0\x08",
         200);
  *(uint32_t*)0x20000260 = 0;
  *(uint32_t*)0x20000264 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint32_t*)0x2000026c = 0;
  *(uint32_t*)0x20000270 = 0;
  *(uint32_t*)0x20000274 = 0;
  *(uint32_t*)0x20000278 = 0;
  *(uint32_t*)0x2000027c = 0;
  syscall(__NR_ioctl, r[0], 0xc100565c, 0x20000180);
  return 0;
}
