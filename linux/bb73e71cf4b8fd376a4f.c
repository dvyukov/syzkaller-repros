// KASAN: vmalloc-out-of-bounds Read in bpf_prog_put
// https://syzkaller.appspot.com/bug?id=bb73e71cf4b8fd376a4f
// status:6
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static unsigned long long procid;

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  int skip = __atomic_load_n(&skip_segv, __ATOMIC_RELAXED) != 0;
  int valid = addr < prog_start || addr > prog_end;
  if (skip && valid) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  ({                                                                           \
    int ok = 1;                                                                \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    } else                                                                     \
      ok = 0;                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    ok;                                                                        \
  })

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

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

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  NONFAILING(*(uint32_t*)0x2000e000 = 0xe);
  NONFAILING(*(uint32_t*)0x2000e004 = 4);
  NONFAILING(*(uint64_t*)0x2000e008 = 0x200002c0);
  NONFAILING(memcpy(
      (void*)0x200002c0,
      "\xb4\x05\x00\x00\x00\x00\x00\x00\x61\x10\x58\x00\x00\x00\x00\x00\xc6\x00"
      "\x00\x00\x00\x00\x00\x00\x95\x00\x00\x02\x00\x00\x00\x00\x2a\xc7\x3f\xe4"
      "\x27\xab\xdf\x47\x8e\x03\x57\xf2\xd3\x29\x86\xf5\x0b\x89\x2b\x50\x07\x92"
      "\x1c\xd4\x8f\xe4\x3b\x23\xbe\x60\xa2\xdb\xc9\x77\x8b\xb6\x65\x51\x0e\x3e"
      "\x22\xfc\xb2\x6a\xdf\xdc\x05\xe5\x5a\x49\x33\xe6\x2a\x23\xab\xb4\xba\xca"
      "\x4e\xc8\x93\x23\x25\x7d\xb5\x9d\xb5\xee\xda\x38\x44\x59\xdd\x00\xd4\x1b"
      "\x6a\xab\x85\xdf\x5c\xab\x39\xe7\x26\x76\x9e\x22\x91\xb6\x26\xd1\x20\xa9"
      "\x24\x75\x79\xf0\xb8\xed\xd8\xf1\x66\x83\xe0\x62\x33\x66\xac\x63\x2c\x24"
      "\x92\x1d\x21\x53\x42\x37\x74\x33\x54\xf0\x9c\xc2\x9b\xe2\xf7\x2d\xfa\x8b"
      "\x05\x32\x01\x10\xbc\xea\x53\xbb\x03\x8c",
      172));
  NONFAILING(*(uint64_t*)0x2000e010 = 0x20003ff6);
  NONFAILING(memcpy((void*)0x20003ff6, "GPL\000", 4));
  NONFAILING(*(uint32_t*)0x2000e018 = 4);
  NONFAILING(*(uint32_t*)0x2000e01c = 0xfd90);
  NONFAILING(*(uint64_t*)0x2000e020 = 0x2000cf3d);
  NONFAILING(*(uint32_t*)0x2000e028 = 0);
  NONFAILING(*(uint32_t*)0x2000e02c = 0);
  NONFAILING(memset((void*)0x2000e030, 0, 16));
  NONFAILING(*(uint32_t*)0x2000e040 = 0);
  NONFAILING(*(uint32_t*)0x2000e044 = 0);
  NONFAILING(*(uint32_t*)0x2000e048 = -1);
  NONFAILING(*(uint32_t*)0x2000e04c = 8);
  NONFAILING(*(uint64_t*)0x2000e050 = 0x20000000);
  NONFAILING(*(uint32_t*)0x20000000 = 0);
  NONFAILING(*(uint32_t*)0x20000004 = 0);
  NONFAILING(*(uint32_t*)0x2000e058 = 0x366);
  NONFAILING(*(uint32_t*)0x2000e05c = 0x10);
  NONFAILING(*(uint64_t*)0x2000e060 = 0x20000000);
  NONFAILING(*(uint32_t*)0x20000000 = 0);
  NONFAILING(*(uint32_t*)0x20000004 = 0);
  NONFAILING(*(uint32_t*)0x20000008 = 0);
  NONFAILING(*(uint32_t*)0x2000000c = 0);
  NONFAILING(*(uint32_t*)0x2000e068 = 0x187);
  NONFAILING(*(uint32_t*)0x2000e06c = 0);
  NONFAILING(*(uint32_t*)0x2000e070 = -1);
  res = syscall(__NR_bpf, 5ul, 0x2000e000ul, 0x48ul);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint32_t*)0x20000100 = 0x12);
  NONFAILING(*(uint32_t*)0x20000104 = 4);
  NONFAILING(*(uint32_t*)0x20000108 = 8);
  NONFAILING(*(uint32_t*)0x2000010c = 4);
  NONFAILING(*(uint32_t*)0x20000110 = 0);
  NONFAILING(*(uint32_t*)0x20000114 = -1);
  NONFAILING(*(uint32_t*)0x20000118 = 0);
  NONFAILING(memset((void*)0x2000011c, 0, 16));
  NONFAILING(*(uint32_t*)0x2000012c = 0);
  NONFAILING(*(uint32_t*)0x20000130 = -1);
  NONFAILING(*(uint32_t*)0x20000134 = 0);
  NONFAILING(*(uint32_t*)0x20000138 = 0);
  NONFAILING(*(uint32_t*)0x2000013c = 0);
  res = syscall(__NR_bpf, 0x100000000000000ul, 0x20000100ul, 0x40ul);
  if (res != -1)
    r[1] = res;
  NONFAILING(syz_open_dev(0, 2, 0x64c540));
  NONFAILING(*(uint64_t*)0x20000000 = 0x20000700);
  NONFAILING(memcpy(
      (void*)0x20000700,
      "\x9f\xeb\x01\x00\x18\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x0c\x00"
      "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x04",
      33));
  NONFAILING(*(uint64_t*)0x20000008 = 0x200004c0);
  NONFAILING(*(uint32_t*)0x20000010 = 0x26);
  NONFAILING(*(uint32_t*)0x20000014 = 0x98);
  NONFAILING(*(uint32_t*)0x20000018 = 1);
  syscall(__NR_bpf, 0x12ul, 0x20000000ul, 0x20ul);
  {
    int i;
    for (i = 0; i < 64; i++) {
      syscall(__NR_bpf, 0x12ul, 0x20000000ul, 0x20ul);
    }
  }
  NONFAILING(*(uint32_t*)0x20000080 = r[1]);
  NONFAILING(*(uint32_t*)0x20000084 = r[0]);
  NONFAILING(*(uint32_t*)0x20000088 = 5);
  NONFAILING(*(uint32_t*)0x2000008c = 0);
  syscall(__NR_bpf, 8ul, 0x20000080ul, 0x10ul);
  res = syscall(__NR_socket, 1ul, 5ul, 0);
  if (res != -1)
    r[2] = res;
  NONFAILING(*(uint32_t*)0x20000200 = r[1]);
  NONFAILING(*(uint64_t*)0x20000208 = 0x20000140);
  NONFAILING(*(uint64_t*)0x20000210 = 0x200000c0);
  NONFAILING(*(uint32_t*)0x200000c0 = r[2]);
  NONFAILING(*(uint64_t*)0x20000218 = 0);
  syscall(__NR_bpf, 2ul, 0x20000200ul, 0x20ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  install_segv_handler();
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
