// BUG: soft lockup in br_ip6_multicast_query_expired
// https://syzkaller.appspot.com/bug?id=f1358d82a4dee12ee4df
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000080 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0x20000500;
  *(uint64_t*)0x20000500 = 0x20000000;
  *(uint32_t*)0x20000000 = 0x50;
  *(uint16_t*)0x20000004 = 0x10;
  *(uint16_t*)0x20000006 = 0x439;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint8_t*)0x20000010 = 0;
  *(uint8_t*)0x20000011 = 0;
  *(uint16_t*)0x20000012 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint32_t*)0x20000018 = 0x542b;
  *(uint32_t*)0x2000001c = 0;
  *(uint16_t*)0x20000020 = 0x30;
  STORE_BY_BITMASK(uint16_t, , 0x20000022, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000023, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000023, 1, 7, 1);
  *(uint16_t*)0x20000024 = 0xb;
  *(uint16_t*)0x20000026 = 1;
  memcpy((void*)0x20000028, "bridge\000", 7);
  *(uint16_t*)0x20000030 = 0x20;
  STORE_BY_BITMASK(uint16_t, , 0x20000032, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000033, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000033, 1, 7, 1);
  *(uint16_t*)0x20000034 = 5;
  *(uint16_t*)0x20000036 = 0x19;
  *(uint8_t*)0x20000038 = 4;
  *(uint16_t*)0x2000003c = 0xc;
  *(uint16_t*)0x2000003e = 0x21;
  *(uint64_t*)0x20000040 = 0;
  *(uint16_t*)0x20000048 = 8;
  *(uint16_t*)0x2000004a = 5;
  *(uint32_t*)0x2000004c = 0;
  *(uint64_t*)0x20000508 = 0x50;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000b0 = 0x2400089c;
  syscall(__NR_sendmsg, r[0], 0x20000080ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
