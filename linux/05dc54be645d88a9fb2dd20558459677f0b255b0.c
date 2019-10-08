// general protection fault in tcf_action_destroy
// https://syzkaller.appspot.com/bug?id=05dc54be645d88a9fb2dd20558459677f0b255b0
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>

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
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
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
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
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

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
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
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
}

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
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

static void close_fds()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

static void setup_fault()
{
  static struct {
    const char* file;
    const char* val;
    bool fatal;
  } files[] = {
      {"/sys/kernel/debug/failslab/ignore-gfp-wait", "N", true},
      {"/sys/kernel/debug/fail_futex/ignore-private", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/min-order", "0", false},
  };
  unsigned i;
  for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].file, files[i].val)) {
      if (files[i].fatal)
        exit(1);
    }
  }
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
      close_fds();
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

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0x0, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10, 0x803, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, r[2], 0x200001c0, 0);
  *(uint32_t*)0x20000200 = 0x14;
  res = syscall(__NR_getsockname, r[2], 0x20000100, 0x20000200);
  if (res != -1)
    r[3] = *(uint32_t*)0x20000104;
  syscall(__NR_sendmsg, r[1], 0, 0);
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x200006c0;
  *(uint32_t*)0x200006c0 = 0x444;
  *(uint16_t*)0x200006c4 = 0x24;
  *(uint16_t*)0x200006c6 = 0x507;
  *(uint32_t*)0x200006c8 = 0;
  *(uint32_t*)0x200006cc = 0;
  *(uint8_t*)0x200006d0 = 0;
  *(uint32_t*)0x200006d4 = r[3];
  *(uint16_t*)0x200006d8 = 0;
  *(uint16_t*)0x200006da = 0;
  *(uint16_t*)0x200006dc = -1;
  *(uint16_t*)0x200006de = -1;
  *(uint16_t*)0x200006e0 = 0;
  *(uint16_t*)0x200006e2 = 0;
  *(uint16_t*)0x200006e4 = 8;
  *(uint16_t*)0x200006e6 = 1;
  memcpy((void*)0x200006e8, "cbq\000", 4);
  *(uint16_t*)0x200006ec = 0x418;
  *(uint16_t*)0x200006ee = 2;
  *(uint16_t*)0x200006f0 = 0x404;
  *(uint16_t*)0x200006f2 = 6;
  *(uint32_t*)0x200006f4 = 0;
  *(uint32_t*)0x200006f8 = 0;
  *(uint32_t*)0x200006fc = 0;
  *(uint32_t*)0x20000700 = 0;
  *(uint32_t*)0x20000704 = 0;
  *(uint32_t*)0x20000708 = 0;
  *(uint32_t*)0x2000070c = 0;
  *(uint32_t*)0x20000710 = 0;
  *(uint32_t*)0x20000714 = 0;
  *(uint32_t*)0x20000718 = 0;
  *(uint32_t*)0x2000071c = 0;
  *(uint32_t*)0x20000720 = 0;
  *(uint32_t*)0x20000724 = 0;
  *(uint32_t*)0x20000728 = 0;
  *(uint32_t*)0x2000072c = 0;
  *(uint32_t*)0x20000730 = 0;
  *(uint32_t*)0x20000734 = 0;
  *(uint32_t*)0x20000738 = 0;
  *(uint32_t*)0x2000073c = 0;
  *(uint32_t*)0x20000740 = 0;
  *(uint32_t*)0x20000744 = 0;
  *(uint32_t*)0x20000748 = 0;
  *(uint32_t*)0x2000074c = 0;
  *(uint32_t*)0x20000750 = 0;
  *(uint32_t*)0x20000754 = 0;
  *(uint32_t*)0x20000758 = 0;
  *(uint32_t*)0x2000075c = 0;
  *(uint32_t*)0x20000760 = 0;
  *(uint32_t*)0x20000764 = 0;
  *(uint32_t*)0x20000768 = 0;
  *(uint32_t*)0x2000076c = 0;
  *(uint32_t*)0x20000770 = 0;
  *(uint32_t*)0x20000774 = 0;
  *(uint32_t*)0x20000778 = 0;
  *(uint32_t*)0x2000077c = 0;
  *(uint32_t*)0x20000780 = 0;
  *(uint32_t*)0x20000784 = 0;
  *(uint32_t*)0x20000788 = 0;
  *(uint32_t*)0x2000078c = 0;
  *(uint32_t*)0x20000790 = 0;
  *(uint32_t*)0x20000794 = 0;
  *(uint32_t*)0x20000798 = 0;
  *(uint32_t*)0x2000079c = 0;
  *(uint32_t*)0x200007a0 = 0;
  *(uint32_t*)0x200007a4 = 0;
  *(uint32_t*)0x200007a8 = 0;
  *(uint32_t*)0x200007ac = 0;
  *(uint32_t*)0x200007b0 = 0;
  *(uint32_t*)0x200007b4 = 0;
  *(uint32_t*)0x200007b8 = 0;
  *(uint32_t*)0x200007bc = 0;
  *(uint32_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c4 = 0;
  *(uint32_t*)0x200007c8 = 0;
  *(uint32_t*)0x200007cc = 0;
  *(uint32_t*)0x200007d0 = 0;
  *(uint32_t*)0x200007d4 = 0;
  *(uint32_t*)0x200007d8 = 0;
  *(uint32_t*)0x200007dc = 0;
  *(uint32_t*)0x200007e0 = 0;
  *(uint32_t*)0x200007e4 = 0;
  *(uint32_t*)0x200007e8 = 0;
  *(uint32_t*)0x200007ec = 0;
  *(uint32_t*)0x200007f0 = 0;
  *(uint32_t*)0x200007f4 = 0;
  *(uint32_t*)0x200007f8 = 0;
  *(uint32_t*)0x200007fc = 0;
  *(uint32_t*)0x20000800 = 0;
  *(uint32_t*)0x20000804 = 0;
  *(uint32_t*)0x20000808 = 0;
  *(uint32_t*)0x2000080c = 0;
  *(uint32_t*)0x20000810 = 0;
  *(uint32_t*)0x20000814 = 0;
  *(uint32_t*)0x20000818 = 0;
  *(uint32_t*)0x2000081c = 0;
  *(uint32_t*)0x20000820 = 0;
  *(uint32_t*)0x20000824 = 0;
  *(uint32_t*)0x20000828 = 0;
  *(uint32_t*)0x2000082c = 0;
  *(uint32_t*)0x20000830 = 0;
  *(uint32_t*)0x20000834 = 0;
  *(uint32_t*)0x20000838 = 0;
  *(uint32_t*)0x2000083c = 0;
  *(uint32_t*)0x20000840 = 0;
  *(uint32_t*)0x20000844 = 0;
  *(uint32_t*)0x20000848 = 0;
  *(uint32_t*)0x2000084c = 0;
  *(uint32_t*)0x20000850 = 0;
  *(uint32_t*)0x20000854 = 0;
  *(uint32_t*)0x20000858 = 0;
  *(uint32_t*)0x2000085c = 0;
  *(uint32_t*)0x20000860 = 0;
  *(uint32_t*)0x20000864 = 0;
  *(uint32_t*)0x20000868 = 0;
  *(uint32_t*)0x2000086c = 0;
  *(uint32_t*)0x20000870 = 0;
  *(uint32_t*)0x20000874 = 0;
  *(uint32_t*)0x20000878 = 0;
  *(uint32_t*)0x2000087c = 0;
  *(uint32_t*)0x20000880 = 0;
  *(uint32_t*)0x20000884 = 0;
  *(uint32_t*)0x20000888 = 0;
  *(uint32_t*)0x2000088c = 0;
  *(uint32_t*)0x20000890 = 0;
  *(uint32_t*)0x20000894 = 0;
  *(uint32_t*)0x20000898 = 0;
  *(uint32_t*)0x2000089c = 0;
  *(uint32_t*)0x200008a0 = 0;
  *(uint32_t*)0x200008a4 = 0;
  *(uint32_t*)0x200008a8 = 0;
  *(uint32_t*)0x200008ac = 0;
  *(uint32_t*)0x200008b0 = 0;
  *(uint32_t*)0x200008b4 = 0;
  *(uint32_t*)0x200008b8 = 0;
  *(uint32_t*)0x200008bc = 0;
  *(uint32_t*)0x200008c0 = 0;
  *(uint32_t*)0x200008c4 = 0;
  *(uint32_t*)0x200008c8 = 0;
  *(uint32_t*)0x200008cc = 0;
  *(uint32_t*)0x200008d0 = 0;
  *(uint32_t*)0x200008d4 = 0;
  *(uint32_t*)0x200008d8 = 0;
  *(uint32_t*)0x200008dc = 0;
  *(uint32_t*)0x200008e0 = 0;
  *(uint32_t*)0x200008e4 = 0;
  *(uint32_t*)0x200008e8 = 0;
  *(uint32_t*)0x200008ec = 0;
  *(uint32_t*)0x200008f0 = 0;
  *(uint32_t*)0x200008f4 = 0;
  *(uint32_t*)0x200008f8 = 0;
  *(uint32_t*)0x200008fc = 0;
  *(uint32_t*)0x20000900 = 0;
  *(uint32_t*)0x20000904 = 0;
  *(uint32_t*)0x20000908 = 0;
  *(uint32_t*)0x2000090c = 0;
  *(uint32_t*)0x20000910 = 0;
  *(uint32_t*)0x20000914 = 0;
  *(uint32_t*)0x20000918 = 0;
  *(uint32_t*)0x2000091c = 0;
  *(uint32_t*)0x20000920 = 0;
  *(uint32_t*)0x20000924 = 0;
  *(uint32_t*)0x20000928 = 0;
  *(uint32_t*)0x2000092c = 0;
  *(uint32_t*)0x20000930 = 0;
  *(uint32_t*)0x20000934 = 0;
  *(uint32_t*)0x20000938 = 0;
  *(uint32_t*)0x2000093c = 0;
  *(uint32_t*)0x20000940 = 0;
  *(uint32_t*)0x20000944 = 0;
  *(uint32_t*)0x20000948 = 0;
  *(uint32_t*)0x2000094c = 0;
  *(uint32_t*)0x20000950 = 0;
  *(uint32_t*)0x20000954 = 0;
  *(uint32_t*)0x20000958 = 0;
  *(uint32_t*)0x2000095c = 0;
  *(uint32_t*)0x20000960 = 0;
  *(uint32_t*)0x20000964 = 0;
  *(uint32_t*)0x20000968 = 0;
  *(uint32_t*)0x2000096c = 0;
  *(uint32_t*)0x20000970 = 0;
  *(uint32_t*)0x20000974 = 0;
  *(uint32_t*)0x20000978 = 0;
  *(uint32_t*)0x2000097c = 0;
  *(uint32_t*)0x20000980 = 0;
  *(uint32_t*)0x20000984 = 0;
  *(uint32_t*)0x20000988 = 0;
  *(uint32_t*)0x2000098c = 0;
  *(uint32_t*)0x20000990 = 0;
  *(uint32_t*)0x20000994 = 0;
  *(uint32_t*)0x20000998 = 0;
  *(uint32_t*)0x2000099c = 0;
  *(uint32_t*)0x200009a0 = 0;
  *(uint32_t*)0x200009a4 = 0;
  *(uint32_t*)0x200009a8 = 0;
  *(uint32_t*)0x200009ac = 0;
  *(uint32_t*)0x200009b0 = 0;
  *(uint32_t*)0x200009b4 = 0;
  *(uint32_t*)0x200009b8 = 0;
  *(uint32_t*)0x200009bc = 0;
  *(uint32_t*)0x200009c0 = 0;
  *(uint32_t*)0x200009c4 = 0;
  *(uint32_t*)0x200009c8 = 0;
  *(uint32_t*)0x200009cc = 0;
  *(uint32_t*)0x200009d0 = 0;
  *(uint32_t*)0x200009d4 = 0;
  *(uint32_t*)0x200009d8 = 0;
  *(uint32_t*)0x200009dc = 0;
  *(uint32_t*)0x200009e0 = 0;
  *(uint32_t*)0x200009e4 = 0;
  *(uint32_t*)0x200009e8 = 0;
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = 0;
  *(uint32_t*)0x200009f4 = 0;
  *(uint32_t*)0x200009f8 = 0;
  *(uint32_t*)0x200009fc = 0;
  *(uint32_t*)0x20000a00 = 0;
  *(uint32_t*)0x20000a04 = 0;
  *(uint32_t*)0x20000a08 = 0;
  *(uint32_t*)0x20000a0c = 0;
  *(uint32_t*)0x20000a10 = 0;
  *(uint32_t*)0x20000a14 = 0;
  *(uint32_t*)0x20000a18 = 0;
  *(uint32_t*)0x20000a1c = 0;
  *(uint32_t*)0x20000a20 = 0;
  *(uint32_t*)0x20000a24 = 0;
  *(uint32_t*)0x20000a28 = 0;
  *(uint32_t*)0x20000a2c = 0;
  *(uint32_t*)0x20000a30 = 0;
  *(uint32_t*)0x20000a34 = 0;
  *(uint32_t*)0x20000a38 = 0;
  *(uint32_t*)0x20000a3c = 0;
  *(uint32_t*)0x20000a40 = 0;
  *(uint32_t*)0x20000a44 = 0;
  *(uint32_t*)0x20000a48 = 0;
  *(uint32_t*)0x20000a4c = 0;
  *(uint32_t*)0x20000a50 = 0;
  *(uint32_t*)0x20000a54 = 0;
  *(uint32_t*)0x20000a58 = 0;
  *(uint32_t*)0x20000a5c = 0;
  *(uint32_t*)0x20000a60 = 0;
  *(uint32_t*)0x20000a64 = 0;
  *(uint32_t*)0x20000a68 = 0;
  *(uint32_t*)0x20000a6c = 0;
  *(uint32_t*)0x20000a70 = 0;
  *(uint32_t*)0x20000a74 = 0;
  *(uint32_t*)0x20000a78 = 0;
  *(uint32_t*)0x20000a7c = 0;
  *(uint32_t*)0x20000a80 = 0;
  *(uint32_t*)0x20000a84 = 0;
  *(uint32_t*)0x20000a88 = 0;
  *(uint32_t*)0x20000a8c = 0;
  *(uint32_t*)0x20000a90 = 0;
  *(uint32_t*)0x20000a94 = 0;
  *(uint32_t*)0x20000a98 = 0;
  *(uint32_t*)0x20000a9c = 0;
  *(uint32_t*)0x20000aa0 = 0;
  *(uint32_t*)0x20000aa4 = 0;
  *(uint32_t*)0x20000aa8 = 0;
  *(uint32_t*)0x20000aac = 0;
  *(uint32_t*)0x20000ab0 = 0;
  *(uint32_t*)0x20000ab4 = 0;
  *(uint32_t*)0x20000ab8 = 0;
  *(uint32_t*)0x20000abc = 0;
  *(uint32_t*)0x20000ac0 = 0;
  *(uint32_t*)0x20000ac4 = 0;
  *(uint32_t*)0x20000ac8 = 0;
  *(uint32_t*)0x20000acc = 0;
  *(uint32_t*)0x20000ad0 = 0;
  *(uint32_t*)0x20000ad4 = 0;
  *(uint32_t*)0x20000ad8 = 0;
  *(uint32_t*)0x20000adc = 0;
  *(uint32_t*)0x20000ae0 = 0;
  *(uint32_t*)0x20000ae4 = 0;
  *(uint32_t*)0x20000ae8 = 0;
  *(uint32_t*)0x20000aec = 0;
  *(uint32_t*)0x20000af0 = 0;
  *(uint16_t*)0x20000af4 = 0x10;
  *(uint16_t*)0x20000af6 = 5;
  *(uint8_t*)0x20000af8 = 0x22;
  *(uint8_t*)0x20000af9 = 0;
  *(uint16_t*)0x20000afa = 0;
  *(uint16_t*)0x20000afc = 0;
  *(uint16_t*)0x20000afe = 0;
  *(uint32_t*)0x20000b00 = 0x7f;
  *(uint64_t*)0x20000088 = 0x444;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000240, 0);
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint64_t*)0x20000150 = 0x200000c0;
  *(uint64_t*)0x200000c0 = 0x20000100;
  *(uint32_t*)0x20000100 = 0x34;
  *(uint16_t*)0x20000104 = 0x2c;
  *(uint16_t*)0x20000106 = 0x701;
  *(uint32_t*)0x20000108 = 0;
  *(uint32_t*)0x2000010c = 0;
  *(uint8_t*)0x20000110 = 0;
  *(uint32_t*)0x20000114 = r[3];
  *(uint16_t*)0x20000118 = 0;
  *(uint16_t*)0x2000011a = 0;
  *(uint16_t*)0x2000011c = 0;
  *(uint16_t*)0x2000011e = 0;
  *(uint16_t*)0x20000120 = 0xfff6;
  *(uint16_t*)0x20000122 = 0xfff1;
  *(uint16_t*)0x20000124 = 0xc;
  *(uint16_t*)0x20000126 = 1;
  memcpy((void*)0x20000128, "tcindex\000", 8);
  *(uint16_t*)0x20000130 = 4;
  *(uint16_t*)0x20000132 = 2;
  *(uint64_t*)0x200000c8 = 0x34;
  *(uint64_t*)0x20000158 = 1;
  *(uint64_t*)0x20000160 = 0;
  *(uint64_t*)0x20000168 = 0;
  *(uint32_t*)0x20000170 = 0;
  syscall(__NR_sendmsg, -1, 0x20000140, 0);
  res = syscall(__NR_socket, 0x10, 0x80002, 0);
  if (res != -1)
    r[4] = res;
  inject_fault(11);
  syscall(__NR_sendmmsg, r[4], 0x20000140, 0x332, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  setup_fault();
  do_sandbox_none();
  return 0;
}