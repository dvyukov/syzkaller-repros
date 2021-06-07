// INFO: task hung in rsvp_delete_filter_work
// https://syzkaller.appspot.com/bug?id=a2ec7a7fb2331091aecf
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
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[6] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff,
                 0xffffffffffffffff, 0x0, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x11;
  memcpy((void*)0x20000082,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, r[0], 0x20000080ul, 0x80ul);
  *(uint32_t*)0x20000140 = 0x14;
  res = syscall(__NR_getsockname, r[0], 0x20000000ul, 0x20000140ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000004;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c8 = 0;
  *(uint64_t*)0x200007d0 = 0x20000100;
  *(uint64_t*)0x20000100 = 0x20000180;
  *(uint32_t*)0x20000180 = 0x40;
  *(uint16_t*)0x20000184 = 0x24;
  *(uint16_t*)0x20000186 = 0xf0b;
  *(uint32_t*)0x20000188 = 0;
  *(uint32_t*)0x2000018c = 0;
  *(uint8_t*)0x20000190 = 0;
  *(uint8_t*)0x20000191 = 0;
  *(uint16_t*)0x20000192 = 0;
  *(uint32_t*)0x20000194 = r[1];
  *(uint16_t*)0x20000198 = 0;
  *(uint16_t*)0x2000019a = 0;
  *(uint16_t*)0x2000019c = -1;
  *(uint16_t*)0x2000019e = -1;
  *(uint16_t*)0x200001a0 = 0;
  *(uint16_t*)0x200001a2 = 0;
  *(uint16_t*)0x200001a4 = 0xd;
  *(uint16_t*)0x200001a6 = 1;
  memcpy((void*)0x200001a8, "fq_codel\000", 9);
  *(uint16_t*)0x200001b4 = 0xc;
  *(uint16_t*)0x200001b6 = 2;
  *(uint16_t*)0x200001b8 = 8;
  *(uint16_t*)0x200001ba = 1;
  *(uint32_t*)0x200001bc = 0;
  *(uint64_t*)0x20000108 = 0x40;
  *(uint64_t*)0x200007d8 = 1;
  *(uint64_t*)0x200007e0 = 0;
  *(uint64_t*)0x200007e8 = 0;
  *(uint32_t*)0x200007f0 = 0;
  syscall(__NR_sendmsg, r[2], 0x200007c0ul, 0ul);
  res = syscall(__NR_socket, 2ul, 0x80803ul, 5);
  if (res != -1)
    r[3] = res;
  *(uint64_t*)0x200005c0 = -1;
  syscall(__NR_setsockopt, r[3], 0, 8, 0x200005c0ul, 1ul);
  *(uint32_t*)0x20000080 = 0xc;
  res = syscall(__NR_getsockopt, r[3], 0, 8, 0x20000040ul, 0x20000080ul);
  if (res != -1)
    r[4] = *(uint32_t*)0x20000040;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[5] = res;
  *(uint64_t*)0x20000000 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x20000680;
  *(uint32_t*)0x20000680 = 0x88;
  *(uint16_t*)0x20000684 = 0x2c;
  *(uint16_t*)0x20000686 = 0xd27;
  *(uint32_t*)0x20000688 = 0;
  *(uint32_t*)0x2000068c = 0;
  *(uint8_t*)0x20000690 = 0;
  *(uint8_t*)0x20000691 = 0;
  *(uint16_t*)0x20000692 = 0;
  *(uint32_t*)0x20000694 = r[4];
  *(uint16_t*)0x20000698 = 0;
  *(uint16_t*)0x2000069a = 0;
  *(uint16_t*)0x2000069c = 0;
  *(uint16_t*)0x2000069e = 0;
  *(uint16_t*)0x200006a0 = 6;
  *(uint16_t*)0x200006a2 = 0;
  *(uint16_t*)0x200006a4 = 0xa;
  *(uint16_t*)0x200006a6 = 1;
  memcpy((void*)0x200006a8, "rsvp6\000", 6);
  *(uint16_t*)0x200006b0 = 0x58;
  *(uint16_t*)0x200006b2 = 2;
  *(uint16_t*)0x200006b4 = 0x14;
  *(uint16_t*)0x200006b6 = 2;
  *(uint8_t*)0x200006b8 = -1;
  *(uint8_t*)0x200006b9 = 1;
  *(uint8_t*)0x200006ba = 0;
  *(uint8_t*)0x200006bb = 0;
  *(uint8_t*)0x200006bc = 0;
  *(uint8_t*)0x200006bd = 0;
  *(uint8_t*)0x200006be = 0;
  *(uint8_t*)0x200006bf = 0;
  *(uint8_t*)0x200006c0 = 0;
  *(uint8_t*)0x200006c1 = 0;
  *(uint8_t*)0x200006c2 = 0;
  *(uint8_t*)0x200006c3 = 0;
  *(uint8_t*)0x200006c4 = 0;
  *(uint8_t*)0x200006c5 = 0;
  *(uint8_t*)0x200006c6 = 0;
  *(uint8_t*)0x200006c7 = 1;
  *(uint16_t*)0x200006c8 = 0x40;
  *(uint16_t*)0x200006ca = 5;
  *(uint16_t*)0x200006cc = 0x3c;
  *(uint16_t*)0x200006ce = 1;
  *(uint32_t*)0x200006d0 = 9;
  *(uint32_t*)0x200006d4 = 0;
  *(uint32_t*)0x200006d8 = 0;
  *(uint32_t*)0x200006dc = 0;
  *(uint32_t*)0x200006e0 = 0;
  *(uint8_t*)0x200006e4 = 0;
  *(uint8_t*)0x200006e5 = 0;
  *(uint16_t*)0x200006e6 = 0;
  *(uint16_t*)0x200006e8 = 0;
  *(uint16_t*)0x200006ea = 0;
  *(uint32_t*)0x200006ec = 0;
  *(uint8_t*)0x200006f0 = 0;
  *(uint8_t*)0x200006f1 = 0;
  *(uint16_t*)0x200006f2 = 0;
  *(uint16_t*)0x200006f4 = 0;
  *(uint16_t*)0x200006f6 = 0;
  *(uint32_t*)0x200006f8 = 0;
  *(uint32_t*)0x200006fc = 0;
  *(uint32_t*)0x20000700 = 0;
  *(uint32_t*)0x20000704 = 0;
  *(uint64_t*)0x200002c8 = 0x88;
  *(uint64_t*)0x20000018 = 1;
  *(uint64_t*)0x20000020 = 0;
  *(uint64_t*)0x20000028 = 0;
  *(uint32_t*)0x20000030 = 0;
  syscall(__NR_sendmsg, r[5], 0x20000000ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}