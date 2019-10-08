// KASAN: slab-out-of-bounds Read in __lock_acquire
// https://syzkaller.appspot.com/bug?id=05a9fbfbef5f1ba7b67d218e97d6eb912c368de3
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
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

long r[21];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    memcpy((void*)0x20048000, "\x2f\x64\x65\x76\x2f\x73\x67\x23\x00",
           9);
    r[2] = syz_open_dev(0x20048000ul, 0x0ul, 0x2ul);
    break;
  case 2:
    *(uint64_t*)0x208f4f80 = (uint64_t)0x20d2f000;
    *(uint64_t*)0x208f4f88 = (uint64_t)0x0;
    *(uint64_t*)0x208f4f90 = (uint64_t)0x20b43f42;
    *(uint64_t*)0x208f4f98 = (uint64_t)0x0;
    *(uint64_t*)0x208f4fa0 = (uint64_t)0x2006ef80;
    *(uint64_t*)0x208f4fa8 = (uint64_t)0x0;
    *(uint64_t*)0x208f4fb0 = (uint64_t)0x202aef6b;
    *(uint64_t*)0x208f4fb8 = (uint64_t)0x0;
    *(uint64_t*)0x208f4fc0 = (uint64_t)0x20a82f62;
    *(uint64_t*)0x208f4fc8 = (uint64_t)0x0;
    *(uint64_t*)0x208f4fd0 = (uint64_t)0x20b70fff;
    *(uint64_t*)0x208f4fd8 = (uint64_t)0x0;
    *(uint64_t*)0x208f4fe0 = (uint64_t)0x20f6a000;
    *(uint64_t*)0x208f4fe8 = (uint64_t)0x0;
    *(uint64_t*)0x208f4ff0 = (uint64_t)0x20c5a000;
    *(uint64_t*)0x208f4ff8 = (uint64_t)0x97;
    r[19] = syscall(__NR_readv, r[2], 0x208f4f80ul, 0x8ul);
    break;
  case 3:
    r[20] = syscall(__NR_fcntl, r[2], 0x4ul, 0x2000ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[8];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 4; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}