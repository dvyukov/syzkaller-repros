// WARNING in xfrm_policy_insert
// https://syzkaller.appspot.com/bug?id=4851459211256681c25aea897c487cf2a643ec96
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 6);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20001a40;
  *(uint64_t*)0x20001a40 = 0x20000040;
  memcpy((void*)0x20000040,
         "\xc4\x00\x00\x00\x19\x00\x25\x0a\x00\x00\x00\x00\x00\x00\x00\x00\xfe"
         "\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xaa\xfe\x80"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint32_t*)0x20000080 = 0;
  *(uint32_t*)0x20000084 = 0;
  memcpy((void*)0x20000088,
         "\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf4\xff\xff\xff"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0c\xe8\x10\x92\x22\x3d\xd6"
         "\xd8\xc9\xc2\x4f\xeb\x0f\x27\x00\x15\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00",
         137);
  *(uint64_t*)0x20001a48 = 0xc4;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000280, 0);
  res = syscall(__NR_socket, 0x10, 3, 6);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10, 3, 6);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20001a40;
  *(uint64_t*)0x20001a40 = 0x200073c0;
  *(uint32_t*)0x200073c0 = 0xc4;
  *(uint16_t*)0x200073c4 = 0x19;
  *(uint16_t*)0x200073c6 = 0xa25;
  *(uint32_t*)0x200073c8 = 0;
  *(uint32_t*)0x200073cc = 0;
  *(uint8_t*)0x200073d0 = 0xfe;
  *(uint8_t*)0x200073d1 = 0x80;
  *(uint8_t*)0x200073d2 = 0;
  *(uint8_t*)0x200073d3 = 0;
  *(uint8_t*)0x200073d4 = 0;
  *(uint8_t*)0x200073d5 = 0;
  *(uint8_t*)0x200073d6 = 0;
  *(uint8_t*)0x200073d7 = 0;
  *(uint8_t*)0x200073d8 = 0;
  *(uint8_t*)0x200073d9 = 0;
  *(uint8_t*)0x200073da = 0;
  *(uint8_t*)0x200073db = 0;
  *(uint8_t*)0x200073dc = 0;
  *(uint8_t*)0x200073dd = 0;
  *(uint8_t*)0x200073de = 0;
  *(uint8_t*)0x200073df = 0xaa;
  *(uint8_t*)0x200073e0 = 0xfe;
  *(uint8_t*)0x200073e1 = 0x80;
  *(uint8_t*)0x200073e2 = 0;
  *(uint8_t*)0x200073e3 = 0;
  *(uint8_t*)0x200073e4 = 0;
  *(uint8_t*)0x200073e5 = 0;
  *(uint8_t*)0x200073e6 = 0;
  *(uint8_t*)0x200073e7 = 0;
  *(uint8_t*)0x200073e8 = 0;
  *(uint8_t*)0x200073e9 = 0;
  *(uint8_t*)0x200073ea = 0;
  *(uint8_t*)0x200073eb = 0;
  *(uint8_t*)0x200073ec = 0;
  *(uint8_t*)0x200073ed = 0;
  *(uint8_t*)0x200073ee = 0;
  *(uint8_t*)0x200073ef = 0;
  *(uint16_t*)0x200073f0 = htobe16(0);
  *(uint16_t*)0x200073f2 = htobe16(0);
  *(uint16_t*)0x200073f4 = htobe16(0);
  *(uint16_t*)0x200073f6 = htobe16(0);
  *(uint16_t*)0x200073f8 = 0xa;
  *(uint8_t*)0x200073fa = 0;
  *(uint8_t*)0x200073fb = 0;
  *(uint8_t*)0x200073fc = 0;
  *(uint32_t*)0x20007400 = 0;
  *(uint32_t*)0x20007404 = 0;
  *(uint64_t*)0x20007408 = 0;
  *(uint64_t*)0x20007410 = 0;
  *(uint64_t*)0x20007418 = 0;
  *(uint64_t*)0x20007420 = 0;
  *(uint64_t*)0x20007428 = 0;
  *(uint64_t*)0x20007430 = 0;
  *(uint64_t*)0x20007438 = 0;
  *(uint64_t*)0x20007440 = 0;
  *(uint64_t*)0x20007448 = 0;
  *(uint64_t*)0x20007450 = 0;
  *(uint64_t*)0x20007458 = 0;
  *(uint64_t*)0x20007460 = 0;
  *(uint32_t*)0x20007468 = 0;
  *(uint32_t*)0x2000746c = 0;
  *(uint8_t*)0x20007470 = 0;
  *(uint8_t*)0x20007471 = 0;
  *(uint8_t*)0x20007472 = 0;
  *(uint8_t*)0x20007473 = 0;
  *(uint16_t*)0x20007478 = 0xc;
  *(uint16_t*)0x2000747a = 0x15;
  *(uint32_t*)0x2000747c = 0;
  *(uint32_t*)0x20007480 = 1;
  *(uint64_t*)0x20001a48 = 0xc4;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, r[2], 0x20000280, 0);
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20001a40;
  *(uint64_t*)0x20001a40 = 0x200073c0;
  *(uint32_t*)0x200073c0 = 0xb8;
  *(uint16_t*)0x200073c4 = 0x19;
  *(uint16_t*)0x200073c6 = 0xa25;
  *(uint32_t*)0x200073c8 = 0;
  *(uint32_t*)0x200073cc = 0;
  *(uint8_t*)0x200073d0 = 0xfe;
  *(uint8_t*)0x200073d1 = 0x80;
  *(uint8_t*)0x200073d2 = 0;
  *(uint8_t*)0x200073d3 = 0;
  *(uint8_t*)0x200073d4 = 0;
  *(uint8_t*)0x200073d5 = 0;
  *(uint8_t*)0x200073d6 = 0;
  *(uint8_t*)0x200073d7 = 0;
  *(uint8_t*)0x200073d8 = 0;
  *(uint8_t*)0x200073d9 = 0;
  *(uint8_t*)0x200073da = 0;
  *(uint8_t*)0x200073db = 0;
  *(uint8_t*)0x200073dc = 0;
  *(uint8_t*)0x200073dd = 0;
  *(uint8_t*)0x200073de = 0;
  *(uint8_t*)0x200073df = 0xaa;
  *(uint8_t*)0x200073e0 = 0xfe;
  *(uint8_t*)0x200073e1 = 0x80;
  *(uint8_t*)0x200073e2 = 0;
  *(uint8_t*)0x200073e3 = 0;
  *(uint8_t*)0x200073e4 = 0;
  *(uint8_t*)0x200073e5 = 0;
  *(uint8_t*)0x200073e6 = 0;
  *(uint8_t*)0x200073e7 = 0;
  *(uint8_t*)0x200073e8 = 0;
  *(uint8_t*)0x200073e9 = 0;
  *(uint8_t*)0x200073ea = 0;
  *(uint8_t*)0x200073eb = 0;
  *(uint8_t*)0x200073ec = 0;
  *(uint8_t*)0x200073ed = 0;
  *(uint8_t*)0x200073ee = 0;
  *(uint8_t*)0x200073ef = 0;
  *(uint16_t*)0x200073f0 = htobe16(0);
  *(uint16_t*)0x200073f2 = htobe16(0);
  *(uint16_t*)0x200073f4 = htobe16(0);
  *(uint16_t*)0x200073f6 = htobe16(0);
  *(uint16_t*)0x200073f8 = 0xa;
  *(uint8_t*)0x200073fa = 0;
  *(uint8_t*)0x200073fb = 0;
  *(uint8_t*)0x200073fc = 0;
  *(uint32_t*)0x20007400 = 0;
  *(uint32_t*)0x20007404 = 0;
  *(uint64_t*)0x20007408 = 0;
  *(uint64_t*)0x20007410 = 0;
  *(uint64_t*)0x20007418 = 0;
  *(uint64_t*)0x20007420 = 0;
  *(uint64_t*)0x20007428 = 0x10000;
  *(uint64_t*)0x20007430 = 0;
  *(uint64_t*)0x20007438 = 0;
  *(uint64_t*)0x20007440 = 0;
  *(uint64_t*)0x20007448 = 0xfffffffffffffffd;
  *(uint64_t*)0x20007450 = 0;
  *(uint64_t*)0x20007458 = 0;
  *(uint64_t*)0x20007460 = 0;
  *(uint32_t*)0x20007468 = 0;
  *(uint32_t*)0x2000746c = 0;
  *(uint8_t*)0x20007470 = 0;
  *(uint8_t*)0x20007471 = 0;
  *(uint8_t*)0x20007472 = 0;
  *(uint8_t*)0x20007473 = 0;
  *(uint64_t*)0x20001a48 = 0xb8;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0x20008946;
  syscall(__NR_sendmsg, r[1], 0x20000280, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
