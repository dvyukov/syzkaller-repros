// KASAN: use-after-free Read in rdma_listen
// https://syzkaller.appspot.com/bug?id=5de8d431c3fd9cc8374d5b492cf06ffede4496af
// status:open
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
  int i;
  for (i = 0; i < 100; i++) {
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
  int iter;
  for (iter = 0;; iter++) {
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x200001c0, "/dev/infiniband/rdma_cm\000", 24);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x200001c0, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = 0;
  *(uint16_t*)0x20000084 = 0x18;
  *(uint16_t*)0x20000086 = 0xfa00;
  *(uint64_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0x20000000;
  *(uint16_t*)0x20000098 = 0x13f;
  *(uint8_t*)0x2000009a = 0;
  *(uint8_t*)0x2000009b = 0;
  *(uint8_t*)0x2000009c = 0;
  *(uint8_t*)0x2000009d = 0;
  *(uint8_t*)0x2000009e = 0;
  *(uint8_t*)0x2000009f = 0;
  res = syscall(__NR_write, r[0], 0x20000080, 0xffaf);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000000;
  *(uint32_t*)0x20000100 = 3;
  *(uint16_t*)0x20000104 = 0x40;
  *(uint16_t*)0x20000106 = 0xfa00;
  *(uint16_t*)0x20000108 = 0xa;
  *(uint16_t*)0x2000010a = htobe16(0x4e21);
  *(uint32_t*)0x2000010c = htobe32(0);
  memcpy((void*)0x20000110,
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
         16);
  *(uint32_t*)0x20000120 = 0;
  *(uint16_t*)0x20000124 = 0xa;
  *(uint16_t*)0x20000126 = htobe16(0);
  *(uint32_t*)0x20000128 = htobe32(0);
  *(uint8_t*)0x2000012c = -1;
  *(uint8_t*)0x2000012d = 1;
  *(uint8_t*)0x2000012e = 0;
  *(uint8_t*)0x2000012f = 0;
  *(uint8_t*)0x20000130 = 0;
  *(uint8_t*)0x20000131 = 0;
  *(uint8_t*)0x20000132 = 0;
  *(uint8_t*)0x20000133 = 0;
  *(uint8_t*)0x20000134 = 0;
  *(uint8_t*)0x20000135 = 0;
  *(uint8_t*)0x20000136 = 0;
  *(uint8_t*)0x20000137 = 0;
  *(uint8_t*)0x20000138 = 0;
  *(uint8_t*)0x20000139 = 0;
  *(uint8_t*)0x2000013a = 0;
  *(uint8_t*)0x2000013b = 1;
  *(uint32_t*)0x2000013c = 0;
  *(uint32_t*)0x20000140 = r[1];
  *(uint32_t*)0x20000144 = 0;
  syscall(__NR_write, r[0], 0x20000100, 0x48);
  *(uint32_t*)0x200002c0 = 7;
  *(uint16_t*)0x200002c4 = 8;
  *(uint16_t*)0x200002c6 = 0xfa00;
  *(uint32_t*)0x200002c8 = r[1];
  *(uint32_t*)0x200002cc = 0;
  syscall(__NR_write, r[0], 0x200002c0, 0x10);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
