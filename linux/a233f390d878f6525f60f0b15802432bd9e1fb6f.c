// possible deadlock in blkdev_reread_part
// https://syzkaller.appspot.com/bug?id=a233f390d878f6525f60f0b15802432bd9e1fb6f
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

#ifndef __NR_memfd_create
#define __NR_memfd_create 356
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[2];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  memcpy((void*)0x202a0000, "/dev/loop#", 11);
  r[0] = syz_open_dev(0x202a0000, 0, 0);
  *(uint32_t*)0x2053ff74 = 0;
  *(uint32_t*)0x2053ff78 = 0;
  *(uint32_t*)0x2053ff7c = 0;
  *(uint32_t*)0x2053ff80 = 0;
  *(uint32_t*)0x2053ff84 = 0x80;
  *(uint32_t*)0x2053ff88 = 0;
  *(uint32_t*)0x2053ff8c = 0x16;
  *(uint32_t*)0x2053ff90 = 9;
  memcpy((void*)0x2053ff94,
         "\xb8\x85\xfc\x0c\x97\xf0\xaa\x14\x11\xf4\x81\xfd\xf3\xa9\xde\xe8\x4c"
         "\x1a\x3b\x42\xb8\xd7\xfb\x46\xa2\x6a\x7d\x2e\x87\x9c\xff\xca\x0c\xeb"
         "\x90\xd3\xe4\xe5\x80\xbe\x13\xb9\x80\xb0\x91\xf1\x41\xec\xfb\x95\x90"
         "\xe8\x82\x44\xf8\xc3\x14\x7f\x06\x7b\xb1\xf2\x8c\x28",
         64);
  memcpy((void*)0x2053ffd4, "\x8b\x5d\x39\xcb\xd9\x51\xbf\xdc\xb2\xdd\x2b\x0a"
                            "\x38\xfe\x9a\xb8\x81\x42\x4f\x09\xc1\x3f\x8b\x51"
                            "\x55\x45\x91\x33\x27\x35\x37\x8a",
         32);
  *(uint32_t*)0x2053fff4 = 3;
  *(uint32_t*)0x2053fff8 = 5;
  *(uint32_t*)0x2053fffc = 0;
  syscall(__NR_ioctl, r[0], 0x4c02, 0x2053ff74);
  memcpy((void*)0x20000000, "\xff\xf8", 2);
  r[1] = syscall(__NR_memfd_create, 0x20000000, 0);
  syscall(__NR_ioctl, r[0], 0x4c00, r[1]);
}

int main()
{
  for (;;) {
    loop();
  }
}
