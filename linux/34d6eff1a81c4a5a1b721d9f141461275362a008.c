// panic: runtime error: index out of range (2)
// https://syzkaller.appspot.com/bug?id=34d6eff1a81c4a5a1b721d9f141461275362a008
// status:fixed
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

unsigned long long procid;

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

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
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
      reset_test();
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  memcpy((void*)0x200005c0, "/dev/ptmx\000", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x200005c0, 0x44001, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000600 = 0x108;
  *(uint8_t*)0x20000602 = 0;
  *(uint8_t*)0x20000603 = 9;
  *(uint32_t*)0x20000604 = 0x1dd;
  *(uint32_t*)0x20000608 = 0x96;
  *(uint32_t*)0x2000060c = 0x7ff;
  *(uint32_t*)0x20000610 = 0x66;
  *(uint32_t*)0x20000614 = 0xf74;
  *(uint32_t*)0x20000618 = 0;
  *(uint32_t*)0x2000061c = 0;
  *(uint64_t*)0x20000620 = 0;
  *(uint64_t*)0x20000628 = 0;
  *(uint64_t*)0x20000630 = 0;
  *(uint64_t*)0x20000638 = 0;
  *(uint64_t*)0x20000640 = 0;
  *(uint64_t*)0x20000648 = 0;
  *(uint64_t*)0x20000650 = 0;
  *(uint64_t*)0x20000658 = 0;
  *(uint64_t*)0x20000660 = 0;
  *(uint64_t*)0x20000668 = 0;
  *(uint64_t*)0x20000670 = 0;
  *(uint64_t*)0x20000678 = 0;
  *(uint64_t*)0x20000680 = 0;
  *(uint64_t*)0x20000688 = 0;
  *(uint64_t*)0x20000690 = 0;
  *(uint64_t*)0x20000698 = 0;
  *(uint64_t*)0x200006a0 = 0;
  *(uint64_t*)0x200006a8 = 0;
  *(uint64_t*)0x200006b0 = 0;
  *(uint64_t*)0x200006b8 = 0;
  *(uint64_t*)0x200006c0 = 0;
  *(uint64_t*)0x200006c8 = 0;
  *(uint64_t*)0x200006d0 = 0;
  *(uint64_t*)0x200006d8 = 0;
  *(uint64_t*)0x200006e0 = 0;
  *(uint64_t*)0x200006e8 = 0;
  *(uint64_t*)0x200006f0 = 0;
  *(uint64_t*)0x200006f8 = 0;
  *(uint64_t*)0x20000700 = 0;
  *(uint64_t*)0x20000708 = 0;
  *(uint64_t*)0x20000710 = 0;
  *(uint64_t*)0x20000718 = 0;
  *(uint64_t*)0x20000720 = 0;
  *(uint64_t*)0x20000728 = 0;
  *(uint64_t*)0x20000730 = 0;
  *(uint64_t*)0x20000738 = 0;
  *(uint64_t*)0x20000740 = 0;
  *(uint64_t*)0x20000748 = 0;
  *(uint64_t*)0x20000750 = 0;
  *(uint64_t*)0x20000758 = 0;
  *(uint64_t*)0x20000760 = 0;
  *(uint64_t*)0x20000768 = 0;
  *(uint64_t*)0x20000770 = 0;
  *(uint64_t*)0x20000778 = 0;
  *(uint64_t*)0x20000780 = 0;
  *(uint64_t*)0x20000788 = 0;
  *(uint64_t*)0x20000790 = 0;
  *(uint64_t*)0x20000798 = 0;
  *(uint64_t*)0x200007a0 = 0;
  *(uint64_t*)0x200007a8 = 0;
  *(uint64_t*)0x200007b0 = 0;
  *(uint64_t*)0x200007b8 = 0;
  *(uint64_t*)0x200007c0 = 0;
  *(uint64_t*)0x200007c8 = 0;
  *(uint64_t*)0x200007d0 = 0;
  *(uint64_t*)0x200007d8 = 0;
  *(uint64_t*)0x200007e0 = 0;
  *(uint64_t*)0x200007e8 = 0;
  *(uint64_t*)0x200007f0 = 0;
  *(uint64_t*)0x200007f8 = 0;
  *(uint64_t*)0x20000800 = 0;
  *(uint64_t*)0x20000808 = 0;
  *(uint64_t*)0x20000810 = 0;
  *(uint64_t*)0x20000818 = 0;
  *(uint64_t*)0x20000820 = 0;
  *(uint64_t*)0x20000828 = 0;
  *(uint64_t*)0x20000830 = 0;
  *(uint64_t*)0x20000838 = 0;
  *(uint64_t*)0x20000840 = 0;
  *(uint64_t*)0x20000848 = 0;
  *(uint64_t*)0x20000850 = 0;
  *(uint64_t*)0x20000858 = 0;
  *(uint64_t*)0x20000860 = 0;
  *(uint64_t*)0x20000868 = 0;
  *(uint64_t*)0x20000870 = 0;
  *(uint64_t*)0x20000878 = 0;
  *(uint64_t*)0x20000880 = 0;
  *(uint64_t*)0x20000888 = 0;
  *(uint64_t*)0x20000890 = 0;
  *(uint64_t*)0x20000898 = 0;
  *(uint64_t*)0x200008a0 = 0;
  *(uint64_t*)0x200008a8 = 0;
  *(uint64_t*)0x200008b0 = 0;
  *(uint64_t*)0x200008b8 = 0;
  *(uint64_t*)0x200008c0 = 0;
  *(uint64_t*)0x200008c8 = 0;
  *(uint64_t*)0x200008d0 = 0;
  *(uint64_t*)0x200008d8 = 0;
  *(uint64_t*)0x200008e0 = 0;
  *(uint64_t*)0x200008e8 = 0;
  *(uint64_t*)0x200008f0 = 0;
  *(uint64_t*)0x200008f8 = 0;
  *(uint64_t*)0x20000900 = 0;
  *(uint64_t*)0x20000908 = 0;
  *(uint64_t*)0x20000910 = 0;
  *(uint64_t*)0x20000918 = 0;
  *(uint64_t*)0x20000920 = 0;
  *(uint64_t*)0x20000928 = 0;
  *(uint64_t*)0x20000930 = 0;
  *(uint64_t*)0x20000938 = 0;
  *(uint64_t*)0x20000940 = 0;
  *(uint64_t*)0x20000948 = 0;
  *(uint64_t*)0x20000950 = 0;
  *(uint64_t*)0x20000958 = 0;
  *(uint64_t*)0x20000960 = 0;
  *(uint64_t*)0x20000968 = 0;
  *(uint64_t*)0x20000970 = 0;
  *(uint64_t*)0x20000978 = 0;
  *(uint64_t*)0x20000980 = 0;
  *(uint64_t*)0x20000988 = 0;
  *(uint64_t*)0x20000990 = 0;
  *(uint64_t*)0x20000998 = 0;
  *(uint64_t*)0x200009a0 = 0;
  *(uint64_t*)0x200009a8 = 0;
  *(uint64_t*)0x200009b0 = 0;
  *(uint64_t*)0x200009b8 = 0;
  *(uint64_t*)0x200009c0 = 0;
  *(uint64_t*)0x200009c8 = 0;
  *(uint64_t*)0x200009d0 = 0;
  *(uint64_t*)0x200009d8 = 0;
  *(uint64_t*)0x200009e0 = 0;
  *(uint64_t*)0x200009e8 = 0;
  *(uint64_t*)0x200009f0 = 0;
  *(uint64_t*)0x200009f8 = 0;
  *(uint64_t*)0x20000a00 = 0;
  *(uint64_t*)0x20000a08 = 0;
  *(uint64_t*)0x20000a10 = 0;
  *(uint64_t*)0x20000a18 = 0;
  *(uint64_t*)0x20000a20 = 0;
  *(uint64_t*)0x20000a28 = 0;
  *(uint64_t*)0x20000a30 = 0;
  *(uint64_t*)0x20000a38 = 0;
  *(uint64_t*)0x20000a40 = 0;
  *(uint64_t*)0x20000a48 = 0;
  *(uint64_t*)0x20000a50 = 0;
  *(uint64_t*)0x20000a58 = 0;
  *(uint64_t*)0x20000a60 = 0;
  *(uint64_t*)0x20000a68 = 0;
  *(uint64_t*)0x20000a70 = 0;
  *(uint64_t*)0x20000a78 = 0;
  *(uint64_t*)0x20000a80 = 0;
  *(uint64_t*)0x20000a88 = 0;
  *(uint64_t*)0x20000a90 = 0;
  *(uint64_t*)0x20000a98 = 0;
  *(uint64_t*)0x20000aa0 = 0;
  *(uint64_t*)0x20000aa8 = 0;
  *(uint64_t*)0x20000ab0 = 0;
  *(uint64_t*)0x20000ab8 = 0;
  *(uint64_t*)0x20000ac0 = 0;
  *(uint64_t*)0x20000ac8 = 0;
  *(uint64_t*)0x20000ad0 = 0;
  *(uint64_t*)0x20000ad8 = 0;
  *(uint64_t*)0x20000ae0 = 0;
  *(uint64_t*)0x20000ae8 = 0;
  *(uint64_t*)0x20000af0 = 0;
  *(uint64_t*)0x20000af8 = 0;
  *(uint64_t*)0x20000b00 = 0;
  *(uint64_t*)0x20000b08 = 0;
  *(uint64_t*)0x20000b10 = 0;
  *(uint64_t*)0x20000b18 = 0;
  *(uint64_t*)0x20000b20 = 0;
  *(uint64_t*)0x20000b28 = 0;
  *(uint64_t*)0x20000b30 = 0;
  *(uint64_t*)0x20000b38 = 0;
  *(uint64_t*)0x20000b40 = 0;
  *(uint64_t*)0x20000b48 = 0;
  *(uint64_t*)0x20000b50 = 0;
  *(uint64_t*)0x20000b58 = 0;
  *(uint64_t*)0x20000b60 = 0;
  *(uint64_t*)0x20000b68 = 0;
  *(uint64_t*)0x20000b70 = 0;
  *(uint64_t*)0x20000b78 = 0;
  *(uint64_t*)0x20000b80 = 0;
  *(uint64_t*)0x20000b88 = 0;
  *(uint64_t*)0x20000b90 = 0;
  *(uint64_t*)0x20000b98 = 0;
  *(uint64_t*)0x20000ba0 = 0;
  *(uint64_t*)0x20000ba8 = 0;
  *(uint64_t*)0x20000bb0 = 0;
  *(uint64_t*)0x20000bb8 = 0;
  *(uint64_t*)0x20000bc0 = 0;
  *(uint64_t*)0x20000bc8 = 0;
  *(uint64_t*)0x20000bd0 = 0;
  *(uint64_t*)0x20000bd8 = 0;
  *(uint64_t*)0x20000be0 = 0;
  *(uint64_t*)0x20000be8 = 0;
  *(uint64_t*)0x20000bf0 = 0;
  *(uint64_t*)0x20000bf8 = 0;
  *(uint64_t*)0x20000c00 = 0;
  *(uint64_t*)0x20000c08 = 0;
  *(uint64_t*)0x20000c10 = 0;
  *(uint64_t*)0x20000c18 = 0;
  *(uint64_t*)0x20000c20 = 0;
  *(uint64_t*)0x20000c28 = 0;
  *(uint64_t*)0x20000c30 = 0;
  *(uint64_t*)0x20000c38 = 0;
  *(uint64_t*)0x20000c40 = 0;
  *(uint64_t*)0x20000c48 = 0;
  *(uint64_t*)0x20000c50 = 0;
  *(uint64_t*)0x20000c58 = 0;
  *(uint64_t*)0x20000c60 = 0;
  *(uint64_t*)0x20000c68 = 0;
  *(uint64_t*)0x20000c70 = 0;
  *(uint64_t*)0x20000c78 = 0;
  *(uint64_t*)0x20000c80 = 0;
  *(uint64_t*)0x20000c88 = 0;
  *(uint64_t*)0x20000c90 = 0;
  *(uint64_t*)0x20000c98 = 0;
  *(uint64_t*)0x20000ca0 = 0;
  *(uint64_t*)0x20000ca8 = 0;
  *(uint64_t*)0x20000cb0 = 0;
  *(uint64_t*)0x20000cb8 = 0;
  *(uint64_t*)0x20000cc0 = 0;
  *(uint64_t*)0x20000cc8 = 0;
  *(uint64_t*)0x20000cd0 = 0;
  *(uint64_t*)0x20000cd8 = 0;
  *(uint64_t*)0x20000ce0 = 0;
  *(uint64_t*)0x20000ce8 = 0;
  *(uint64_t*)0x20000cf0 = 0;
  *(uint64_t*)0x20000cf8 = 0;
  *(uint64_t*)0x20000d00 = 0;
  *(uint64_t*)0x20000d08 = 0;
  *(uint64_t*)0x20000d10 = 0;
  *(uint64_t*)0x20000d18 = 0;
  syscall(__NR_write, r[0], 0x20000600, 0x720);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 4; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}