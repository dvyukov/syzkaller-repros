// KASAN: slab-out-of-bounds Read in bpf_skb_change_proto
// https://syzkaller.appspot.com/bug?id=e0e0bd1d9c362e44606d8c60d933d86806374a97
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

void loop(void)
{
  long res = 0;
  *(uint32_t*)0x20000200 = 4;
  *(uint32_t*)0x20000204 = 0xe;
  *(uint64_t*)0x20000208 = 0x20000000;
  memcpy((void*)0x20000000,
         "\xb7\x02\x00\x00\x00\x00\x00\x00\xbf\xa3\x00\x00\x00\x00\x00\x00\x07"
         "\x01\x00\x00\x00\xfe\xff\xff\x7a\x0a\xf0\xff\xf8\xff\xff\xff\x79\xa4"
         "\xf0\xff\x00\x00\x00\x00\xb7\x06\x00\x00\xff\xff\xff\xff\x2d\x64\x05"
         "\x00\x00\x00\x00\x00\x65\x04\x04\x00\x01\x00\x00\x00\x04\x04\x00\x00"
         "\x01\x00\x00\x20\xb7\x03\x00\x00\x00\x00\x00\x00\x6a\x0a\x00\xfe\x00"
         "\x00\x00\x00\x85\x00\x00\x00\x1f\x00\x00\x00\xb7\x00\x00\x00\x00\x00"
         "\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00",
         112);
  *(uint64_t*)0x20000210 = 0x20000340;
  memcpy((void*)0x20000340, "syzkaller", 10);
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  *(uint8_t*)0x20000230 = 0;
  *(uint8_t*)0x20000231 = 0;
  *(uint8_t*)0x20000232 = 0;
  *(uint8_t*)0x20000233 = 0;
  *(uint8_t*)0x20000234 = 0;
  *(uint8_t*)0x20000235 = 0;
  *(uint8_t*)0x20000236 = 0;
  *(uint8_t*)0x20000237 = 0;
  *(uint8_t*)0x20000238 = 0;
  *(uint8_t*)0x20000239 = 0;
  *(uint8_t*)0x2000023a = 0;
  *(uint8_t*)0x2000023b = 0;
  *(uint8_t*)0x2000023c = 0;
  *(uint8_t*)0x2000023d = 0;
  *(uint8_t*)0x2000023e = 0;
  *(uint8_t*)0x2000023f = 0;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  res = syscall(__NR_bpf, 5, 0x20000200, 0x48);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200001c0 = r[0];
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = 0xe;
  *(uint32_t*)0x200001cc = 0xd7;
  *(uint64_t*)0x200001d0 = 0x20000080;
  memcpy((void*)0x20000080,
         "\x3c\x08\x00\x00\x00\xf0\x8b\x41\x63\x7c\x2f\xee\x86\xdd", 14);
  *(uint64_t*)0x200001d8 = 0x20000380;
  *(uint32_t*)0x200001e0 = 0x7ff;
  *(uint32_t*)0x200001e4 = 0;
  syscall(__NR_bpf, 0xa, 0x200001c0, 0x28);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
