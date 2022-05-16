// BUG: unable to handle kernel paging request in bitfill_aligned (2)
// https://syzkaller.appspot.com/bug?id=a4edd73d589b0b7efbeb
// status:6
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20002540, "/dev/fb0\000", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20002540ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = 0xa0;
  *(uint32_t*)0x20000084 = 0x80;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0xa0000000;
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = 0xfffffffc;
  *(uint32_t*)0x20000098 = 8;
  *(uint32_t*)0x2000009c = 0;
  *(uint32_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a4 = 0;
  *(uint32_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000ac = 0;
  *(uint32_t*)0x200000b0 = 0;
  *(uint32_t*)0x200000b4 = 0;
  *(uint32_t*)0x200000b8 = 0;
  *(uint32_t*)0x200000bc = 0;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint32_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d4 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint32_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e4 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = 0;
  *(uint32_t*)0x200000f4 = 0;
  *(uint32_t*)0x200000f8 = 0;
  *(uint32_t*)0x200000fc = 0;
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0x100;
  *(uint32_t*)0x20000108 = 0;
  *(uint32_t*)0x2000010c = 0;
  memset((void*)0x20000110, 0, 16);
  syscall(__NR_ioctl, r[0], 0x4601, 0x20000080ul);
  res = -1;
  res = syz_open_dev(0xc, 4, 1);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000400 = 0;
  *(uint32_t*)0x20000404 = 0;
  *(uint32_t*)0x20000408 = 0x18;
  *(uint32_t*)0x2000040c = 0xa;
  *(uint32_t*)0x20000410 = 0x200;
  *(uint64_t*)0x20000418 = 0x20000000;
  memcpy(
      (void*)0x20000000,
      "\x41\xe2\x26\x66\x04\x78\x91\x2a\xe9\x68\xb4\xc5\xb4\x47\x40\xa2\x85\xaa"
      "\x2e\xe7\x83\xd6\x93\x2d\xfe\xfc\xb9\x95\xd2\x2d\xa2\xbf\xd4\x89\xd9\xd2"
      "\xe0\x5a\x66\x8b\x06\xca\x23\x12\xc9\x63\x07\x00\x00\x00\x6c\xb7\xa4\xf5"
      "\xb8\x26\x02\x8d\x3e\x1e\x26\xe1\x0c\xfb\x85\x83\x1a\x76\xe2\x52\x3e\xd6"
      "\x83\x01\xdb\x1b\xa2\x77\xe0\x72\x9c\xf9\x07\x93\x1e\x8e\x39\x26\xd7\x69"
      "\xb7\xc4\x31\xd0\x00\xb3\xaa\xa9\x62\x9f\x2a\xc1\xed\x6f\x62\xa8\x68\x43"
      "\x13\x33\xa7\x66\xe2\x99\x50\x44\x9e\x68\xb7\x67\x8b\x3c\xa8\x74\x5e\x22"
      "\x7b\x59\x04\x9d\x38\x21\x3c\x6b\x23\x4e\x29\x03\x87\x05\xae\x1e\xf6\x36"
      "\xe6\x56\x61\x11\xee\xef\xb1\xef\x80\x43\x01\x84\x6f\x16\x3d\x1b\x0a\xe8"
      "\xc5\x84\x95\x3f\x34\x97\x08\x87\x47\x27\xc7\x56\x59\x77\x6b\x20\x06\x59"
      "\xac\xab\x14\x44\x54\xca\x7d\x7b\x78\x60\x42\xc9\x6b\xfa\x40\xd5\xd5\xfc"
      "\xf2\x22\x6a\xad\x54\x61\xdd\x4e\x8c\xf0\x34\x82\x11\x46\x9e\x47\x99\x13"
      "\x50\xc0\xa4\xd6\xff\xf8\x5f\xcb\x52\xa1\xd8\x56\x00\xf4\xc1\x42\x79\x1a"
      "\xe6\x42\xec\x5b\x5c\x81\xec\x5f\xb9\x79\x6b\x61\xa6\xa2\xdc\x74\x77\x9a"
      "\x47\x50\x67\xfb\xd7\xd3\x48\x83\x66\x4a\xc3\x3f\xfd\x8c\x53\x21\x46\x3e"
      "\xab\x48\xa0\x44\xfc\x71\x9f\x94\x5d\xbb\xf5\x75\x56\xe7\x6d\xc7\x55\xd7"
      "\x5e\xcf\xab\x57\x30\x48\x21\xbc\xda\x8c\xd6\xbd\x90\xf2\x15\x7a\x56\xca"
      "\xe0\x41\x3a\x0a\x4d\x99\x72\x3e\x2a\x97\x01\x00\x1c\x4e\xff\xe8\xd7\xf5"
      "\x00\xb3\x0b\x4e\xd2\x72\x11\xef\xfc\x45\x59\x1d\xaa\x93\xaf\xba\x11\x5e"
      "\xcd\x9d\xf0\x82\x37\xb3\xde\x77\x9e\xe3\x2a\x6a\x95\xa6\x5a\x62\xce\x5c"
      "\xb7\xb0\x89\x69\x36\x82\xed\x18\x8e\x83\xfd\x88\x32\x89\xa6\x99\xdd\x44"
      "\xd4\xb5\x79\x69\x2b\x62\x44\x77\x9e\x43\x36\xf8\xfd\x7d\x3b\xbb\x91\x51"
      "\xfb\x45\x6a\x77\x5e\xfa\x1f\x2f\xac\x58\xfe\x8a\xf1\x91\x62\xa2\x94\x13"
      "\x10\x07\xd5\xe5\x41\xf3\xb9\x50\xdd\xbb\x75\x08\x59\x70\xee\xa8\xb3\x27"
      "\x0e\xe2\xe3\x17\x19\x3c\x47\xc7\x4f\xba\xfd\x55\xd9\xba\xb4\x7c\x82\xc2"
      "\x5d\xd6\xdf\xaf\x83\xe8\xf6\xd3\x3c\xce\x3a\x74\xcc\x86\x7d\x69\x9f\xde"
      "\x52\xb8\xc8\xd4\xb4\x89\x17\xbe\xdd\xbf\xf7\xff\xf8\x2a\xea\x3f\x4d\xcd"
      "\xb0\x24\x0c\xc3\x84\x0c\x1a\xca\x59\x9b\x1f\xa2\x67\xfa\xe8\xcc\xeb\xfe"
      "\x79\xa5\x0a\x91\x43\x3f\xc5\x17\x5e\x64\x8a\x50\x28\xcc\x48\x1e\x36\x79"
      "\x01\xb9\x14\x65\xbc\x1c\x7a\xc7\xbf\x33\x1d\x68\x77\xf7\x90\xa9\xe5\x18"
      "\x29\xc1\x51\x1a\x83\xaf\x1b\x7b\x6d\xdd\xf3\xd5\xef\x10\x1e\x2d\x7b\x04"
      "\xe5\x93\x83\x27\x4c\xc5\xa7\xf4\x8d\x9f\x85\xc4\xd4\x6b\x76\xa8\x5d\x2c"
      "\xe8\xb3\x4b\x29\x88\x95\xa7\xcf\xdb\x54\x65\x91\x9b\x7a\x92\x46\x37\x68"
      "\x9d\x9c\xaa\x48\xf3\xad\xd3\x7d\xa0\xf8\x72\xd2\xd8\x87\x1b\xa6\x4d\x44"
      "\x8d\x3e\x01\x8d\x71\x7a\x5b\x0a\x94\x39\xde\x2a\xe9\xf5\x8d\xa8\x70\xbe"
      "\x04\x60\x12\xfb\x1d\x41\x0e\x2b\x3b\x14\x1a\x1b\xc1\x1c\xae\x14\x96\xa2"
      "\x1f\x60\x5f\x4b\x97\xcc\xa8\x01\xe5\x46\x53\xba\xe5\xae\xd8\x70\x1a\x35"
      "\xc2\x24\x6f\x89\x7e\x91\x4f\xc8\xb2\xe4\x60\x7c\x59\xb2\x35\x9a\xf3\x01"
      "\xcc\x56\x00\x80\x5b\x06\x44\xa3\xef\xaf\x2b\xf5\x9d\xb9\x37\x96\xb6\xf4"
      "\x5e\x15\xcf\x77\x25\xf6\x4b\xc0\xa2\x29\x60\x9f\xff\x9b\xbe\x91\x22\x40"
      "\xb8\x2a\x37\xbb\xf8\x5b\x09\x87\x86\x2e\xf5\x79\x39\x5d\xc2\xf5\x8a\x04"
      "\x6f\x4d\xfd\x49\xee\xae\x2a\x16\x9b\x06\xae\x8e\xae\x77\x42\xf9\xae\x5a"
      "\x56\x8b\xe8\xbb\x0c\xd1\x48\x1a\x4b\x4e\x61\x61\x23\x66\xca\x2f\x51\x42"
      "\x43\x56\x1e\x80\xb1\x3e\xf6\xac\x3f\x82\xa0\x4e\xbe\x36\xfc\x3c\x11\x9d"
      "\xe9\xb3\xf4\x7a\x1a\x57\x2e\x28\x17\xbb\x9d\x94\x2f\x0f\x8a\xe1\xa5\xe4"
      "\x0b\xc3\x66\xb6\xb1\x86\x9d\x94\xf5\xb9\xc2\x06\xf0\x43\x95\x94\xe7\x1d"
      "\x25\xdb\xd0\x37\x34\x5c\x7d\x4c\xb9\x68\x36\x06\xb9\x6c\x07\xc0\xc6\xd2"
      "\x2a\x49\x8b\xa1\x2d\xb7\x8a\x2e\x37\x84\x44\x53\xef\xac\xe2\x03\x75\x69"
      "\x1a\x7f\xeb\x03\xe6\x6d\x87\xf1\x3a\x27\x9e\xe6\x37\x1e\x1b\x79\xcb\x18"
      "\x18\xec\xd9\xea\xfa\xf6\x03\xc8\x61\xc9\xd1\xe9\x11\x3d\xfa\x95\x4c\xbe"
      "\xf1\x12\x7b\xdf\xed\xe9\xb9\xad\x33\x24\xa6\x91\xde\xfa\x5d\xfb\x26\xfc"
      "\xae\x08\xe1\x49\xf8\x61\x4b\xb4\x1a\xa8\xea\xbc\x3b\x53\x6d\xf1\xfc\xab"
      "\xe2\x2c\x24\xbe\x6a\xc9\xa7\x28\x25\xfe\x83\xc4\x43\x2e\x1a\x4a\x34\x6d"
      "\x4e\xd2\x79\x6b\x39\x2b\xeb\xf1\xad\x9e\x06\xbd\x0b\xe3\xa6\x27\x23\x6e"
      "\x19\xa3\x0c\xed\x98\xcc\xff\x58\x80\xf2\x5a\x7f\x6e\x35\x55\x78\x05\xe9"
      "\xf3\x66\xb8\x57\x6e\x6e\xb3\xf6\xd8\x44\x42\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      1024);
  syscall(__NR_ioctl, r[1], 0x4b72, 0x20000400ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
