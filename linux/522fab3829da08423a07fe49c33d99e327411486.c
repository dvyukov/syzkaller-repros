// KMSAN: kernel-infoleak in copy_siginfo_to_user
// https://syzkaller.appspot.com/bug?id=522fab3829da08423a07fe49c33d99e327411486
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0x0};
void execute_one()
{
  long res = 0;
  res = syscall(__NR_inotify_init1, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_fcntl, r[0], 8, -1);
  res = syscall(__NR_fcntl, r[0], 0x10, 0x20000280);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000284;
  syscall(__NR_ptrace, 0x4206, r[1], 0, 0);
  syscall(__NR_ptrace, 0x4207, r[1]);
  memcpy((void*)0x20000000,
         "\x79\x61\x6d\x30\x00\x01\x17\x8b\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint16_t*)0x20000010 = 0x4013;
  syscall(__NR_ioctl, -1, 0x400454ca, 0x20000000);
  *(uint64_t*)0x20000100 = 0x20000040;
  *(uint64_t*)0x20000108 = 0;
  syscall(__NR_ptrace, 0x4209, r[1], 0x20000004, 0x20000100);
  *(uint32_t*)0x20000340 = 0xc1;
  *(uint64_t*)0x20000348 = 0;
  *(uint64_t*)0x20000350 = 0;
  *(uint8_t*)0x20000358 = 0;
  *(uint8_t*)0x20000359 = 0;
  *(uint8_t*)0x2000035a = 0;
  *(uint8_t*)0x2000035b = 0;
  *(uint32_t*)0x2000035c = 0;
  *(uint8_t*)0x20000360 = 0;
  *(uint32_t*)0x20000364 = 0;
  *(uint32_t*)0x20000368 = 0;
  *(uint32_t*)0x2000036c = 0;
  *(uint32_t*)0x20000370 = 0;
  *(uint32_t*)0x20000374 = 0;
  *(uint32_t*)0x20000378 = 0;
  *(uint32_t*)0x2000037c = 0;
  *(uint32_t*)0x20000380 = 0;
  *(uint32_t*)0x20000384 = 0;
  *(uint32_t*)0x20000388 = 0;
  syscall(__NR_ioctl, -1, 0x4040534e, 0x20000340);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
