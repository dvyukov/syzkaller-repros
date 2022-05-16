// INFO: task hung in usb_bulk_msg
// https://syzkaller.appspot.com/bug?id=e716aef347d472a5333a
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
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000f40, "/dev/bus/usb/00#/00#\000", 21);
  res = -1;
  res = syz_open_dev(0x20000f40, 0x200, 0x123002);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = 0;
  *(uint32_t*)0x20000084 = 0;
  memcpy(
      (void*)0x20000088,
      "\xe3\x8f\x9e\xa1\x48\xc1\x45\xd7\x45\x48\x98\x03\x7e\x06\xe2\x5c\xa2\xdc"
      "\x65\x67\x1b\xbb\xc6\x51\x97\x4f\xd3\x2a\x70\x3e\x86\x1f\x77\x20\x29\x0b"
      "\xd5\x09\xf1\xa4\xf4\xee\x95\x1b\x19\x07\x02\xa9\x30\xdd\xcc\xba\xaa\x5e"
      "\xfd\x43\x51\x32\x06\x1c\x1f\x7d\x47\x5a\x68\xe6\x32\x33\x2c\xaf\x6e\xa0"
      "\x95\x1b\xaa\xc6\xf8\x4b\x8b\xbc\x5b\xd1\xc6\x3c\x7e\x53\x0c\xb5\xf4\x01"
      "\xc9\x40\x38\xcb\xa4\x7c\x77\x47\xd8\x18\xe8\xb3\x10\xcd\x6a\x3f\x52\xde"
      "\x7e\x7d\x5a\xf1\xe8\xca\x3b\xfa\x04\x44\xc3\xff\x62\xbd\x75\x84\x0b\xee"
      "\x41\x23\xbe\xd9\x33\xbd\x12\x05\xda\x77\x89\x9a\x8d\x68\xcb\x40\xd2\x37"
      "\x7c\xd9\x65\x31\x24\x33\x78\xb5\xc2\x94\xdf\xd3\xc5\x37\xf2\xf4\xdb\x50"
      "\x00\x12\x74\x98\xc2\x80\x31\x75\xd2\xd8\xa5\xab\x4c\x3f\x83\xc3\xfd\x7a"
      "\xe9\xb2\xa8\x67\x4b\x10\x13\x5a\x2c\x23\x91\x45\xc9\x40\x37\x74\xac\x2d"
      "\xe7\x14\x0b\xa9\x09\x96\x83\x4b\x86\xc1\x39\xcf\x6a\x21\xb1\x79\x3e\xee"
      "\xfc\xfa\xd3\x06\x42\x22\x4e\xab\x9f\x1f\xf0\xca\x39\x76\x7c\xac\x24\x11"
      "\xf7\xd7\x25\xe6\xc9\xb5\x7e\x16\x63\xd8\x5a\x19\x82\x60\xf3\x9f\xcd\xe5"
      "\x80\x7a\x7d\x10",
      256);
  syscall(__NR_ioctl, r[0], 0x8108551b, 0x20000080ul);
  STORE_BY_BITMASK(uint8_t, , 0x200002c0, 1, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x200002c0, 1, 7, 1);
  *(uint32_t*)0x200002c4 = 0xcc;
  *(uint32_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0x200001c0;
  memcpy((void*)0x200001c0,
         "\x25\xab\x8a\xa2\xba\x41\x3b\xc5\x2e\xd2\x84\x0e\x95\x09\x55\x0d\x93"
         "\xbd\x10\xe1\x8c\x47\x62\x5e\x53\xed\xe7\xb4\x92\xac\x71\xda\x8c\xc0"
         "\xfd\xce\xfc\x12\x0a\x24\xd4\x15\xf7\x34\xca\x38\x30\x82\x44\x5d\x43"
         "\x21\xfb\x2b\x31\xce\x11\x0e\x5f\xb0\xa6\x55\xac\xbc\x9a\xc5\x0a\x09"
         "\xe7\xd5\xd3\xf7\xb0\xf0\x67\x52\x68\xd6\x3a\xc4\xc2\x23\x29\xeb\x73"
         "\xb1\xb5\x7c\x0c\xe6\x84\xdf\xeb\x78\xfb\x87\x5f\x42\xaf\x91\x96\x7e"
         "\xc7\xa8\xe4\x01\xc8\x2f\x1d\x0f\x2e\x59\x90\xbe\xec\x23\x6d\x91\x7e"
         "\x44\xbc\x77\xb4\xbf\xc5\x8b\x85\xaf\x66\x91\x70\xee\x38\x9f\x2a\x2b"
         "\x9c\x3b\xce\x4a\x0e\x25\xbd\xe8\x47\xcc\xd2\xf9\x50\x46\xf4\x82\x88"
         "\x43\x1f\x48\xb2\xbe\x8b\xf8\x61\x25\x3c\x30\xc5\x60\xa3\xc1\x8f\xe1"
         "\x6a\xb4\xf6\x2b\x73\x98\x0f\x08\xdd\x0b\x90\x93\xf4\x5d\xd2\x00\x7e"
         "\x1b\x2a\x31\xcb\x0a\x9a\xbf\x64\x07\xc3\xfe\x68\x48\x22\x3a\x2e\x63",
         204);
  syscall(__NR_ioctl, r[0], 0xc0185502, 0x200002c0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
