// WARNING in blkdev_put
// https://syzkaller.appspot.com/bug?id=920b2a33b8b93976d1fed9da1724c185c55a1291
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/loop.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;

  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    fail("tun: snprintf failed");
  if ((size_t)rv >= size)
    fail("tun: string '%s...' doesn't fit into buffer", str);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                            \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(bool panic, const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
  va_end(args);
  rv = system(command);
  if (rv) {
    if (panic)
      fail("command '%s' failed: %d", &command[0], rv);
  }
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02hx"
#define DEV_MAC "aa:aa:aa:aa:aa:%02hx"

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

static void initialize_netdevices(void)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "team"};
  const char* devnames[] = {"lo",
                            "sit0",
                            "bridge0",
                            "vcan0",
                            "tunl0",
                            "gre0",
                            "gretap0",
                            "ip_vti0",
                            "ip6_vti0",
                            "ip6tnl0",
                            "ip6gre0",
                            "ip6gretap0",
                            "erspan0",
                            "bond0",
                            "veth0",
                            "veth1",
                            "team0",
                            "veth0_to_bridge",
                            "veth1_to_bridge",
                            "veth0_to_bond",
                            "veth1_to_bond",
                            "veth0_to_team",
                            "veth1_to_team"};
  const char* devmasters[] = {"bridge", "bond", "team"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add type veth");

  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    execute_command(
        0, "ip link add name %s_slave_0 type veth peer name veth0_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(
        0, "ip link add name %s_slave_1 type veth peer name veth1_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(0, "ip link set %s_slave_0 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set %s_slave_1 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set veth0_to_%s up", devmasters[i]);
    execute_command(0, "ip link set veth1_to_%s up", devmasters[i]);
  }
  execute_command(0, "ip link set bridge_slave_0 up");
  execute_command(0, "ip link set bridge_slave_1 up");

  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[32];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, i + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, i + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, i + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
  }
}

extern unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define SYZ_memfd_create 319

static uintptr_t syz_read_part_table(uintptr_t size, uintptr_t nsegs,
                                     uintptr_t segments)
{
  char loopname[64], linkname[64];
  int loopfd, err = 0, res = -1;
  uintptr_t i, j;
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;

  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (i = 0; i < nsegs; i++) {
    if (segs[i].size > IMAGE_MAX_SIZE)
      segs[i].size = IMAGE_MAX_SIZE;
    segs[i].offset %= IMAGE_MAX_SIZE;
    if (segs[i].offset > IMAGE_MAX_SIZE - segs[i].size)
      segs[i].offset = IMAGE_MAX_SIZE - segs[i].size;
    if (size < segs[i].offset + segs[i].offset)
      size = segs[i].offset + segs[i].offset;
  }
  if (size > IMAGE_MAX_SIZE)
    size = IMAGE_MAX_SIZE;
  int memfd = syscall(SYZ_memfd_create, "syz_read_part_table", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
  }
  snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
  loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    err = errno;
    goto error_close_memfd;
  }
  if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
    if (errno != EBUSY) {
      err = errno;
      goto error_close_loop;
    }
    ioctl(loopfd, LOOP_CLR_FD, 0);
    usleep(1000);
    if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
      err = errno;
      goto error_close_loop;
    }
  }
  struct loop_info64 info;
  if (ioctl(loopfd, LOOP_GET_STATUS64, &info)) {
    err = errno;
    goto error_clear_loop;
  }
  info.lo_flags |= LO_FLAGS_PARTSCAN;
  if (ioctl(loopfd, LOOP_SET_STATUS64, &info)) {
    err = errno;
    goto error_clear_loop;
  }
  res = 0;
  for (i = 1, j = 0; i < 8; i++) {
    snprintf(loopname, sizeof(loopname), "/dev/loop%llup%d", procid, (int)i);
    struct stat statbuf;
    if (stat(loopname, &statbuf) == 0) {
      snprintf(linkname, sizeof(linkname), "./file%d", (int)j++);
      if (symlink(loopname, linkname)) {
      }
    }
  }
error_clear_loop:
  ioctl(loopfd, LOOP_CLR_FD, 0);
error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return res;
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

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_netdevices();
  loop();
  doexit(1);
}

static void execute_one();
extern unsigned long long procid;

static void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    char buf[64];
    snprintf(buf, sizeof(buf), "/dev/loop%llu", procid);
    int loopfd = open(buf, O_RDWR);
    if (loopfd != -1) {
      ioctl(loopfd, LOOP_CLR_FD, 0);
      close(loopfd);
    }
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      execute_one();
      doexit(0);
    }

    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid) {
        break;
      }
      usleep(1000);
      if (current_time_ms() - start < 3 * 1000)
        continue;
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, __WALL) != pid) {
      }
      break;
    }
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

unsigned long long procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    memcpy((void*)0x20000100, "/dev/vga_arbiter", 17);
    syscall(__NR_openat, 0xffffffffffffff9c, 0x20000100, 0x400, 0);
    break;
  case 1:
    syscall(__NR_socket, 0xa, 2, 0);
    break;
  case 2:
    syscall(__NR_pwritev, -1, 0x20001340, 0, 0);
    break;
  case 3:
    *(uint64_t*)0x20001a00 = 0x20000600;
    memcpy(
        (void*)0x20000600,
        "\x6c\x5e\x88\xcd\x2c\xdc\x9a\x41\x65\x6a\x9b\xbe\xa4\x92\x01\x3a\x97"
        "\xdd\x5c\xcd\xac\x19\x69\x40\x81\xc0\x56\x4e\xec\x9f\xa0\x61\xac\x9e"
        "\xe0\xd3\x64\x65\x20\x3a\x18\xc1\xe4\xf7\x23\x90\x12\xaa\x5d\x16\xf8"
        "\x76\x6a\x15\x6f\xe7\xd5\x26\x5e\x8a\xfa\xe3\x40\x22\x40\xe5\xf4\x4e"
        "\xec\xcf\xd4\x3d\x3b\xdc\x48\x3c\x7e\x1a\x67\xfb\x2b\xed\x67\xc7\x69"
        "\x09\xb6\x6a\x7d\x3c\xed\xff\x64\xc7\x04\xc2\xb4\x78\xcc\x18\x37\x66"
        "\xa6\xb6\xbf\x17\x71\x31\x61\xa9\xb1\x8d\x48\xe4\x99\xce\x95\x56\x60"
        "\xed\x00\x31\x1e\xe7\x10\x29\xca\x90\x83\xdf\x16\x5f\xd6\x10\x20\x4c"
        "\x2d\xb7\x97\xf5\xb9\x70\x32\x45\x03\x0a\x84\x39\x8b\x80\xf6\x41\x26"
        "\x24\xc0\xb6\x51\xd4\x47\xb9\x9b\x4e\x8b\x70\xa4\xb8\xf3\x4e\xd0\x73"
        "\xf5\x09\xc7\x8d\x20\x9f\xaf\x52\x3b\x58\x7a\x87\x43\x74\xac\x25\x16"
        "\x5c\x7e\x18\xee\x61\xf1\x61\x2f\xb1\x11\xf8\xc0\xec\x42\xc4\x9e\xe3"
        "\xc8\x32\x81\xc5\x63\xb9\xb9\xb6\x31\x20\x6c\x28\xbd\x80\xdd\xc7\xda"
        "\x59\x4b\xf3\xd2\x79\xbb\xe5\x54\x56\xd8\x25\xf1\x28\x0a\xab\x4c\xad"
        "\xce\x01\x0c\xf3\x6b\x44\xb9\x78\xeb\x5b\xe5\x5f\xb5\xf5\x39\x6c\x78"
        "\x7c\x7d\xa2\xee\x7a\xc1\xb9\xec\xa7\xa6\xc6\x84\xdd\x6b\x8c\xae\xfd"
        "\x7a\xdd\xb9\xb6\x27\x79\xce\xa5\xaf\xca\x2f\x19\x18\x4e\x67\x7d\x99"
        "\x0f\xd0\x6a\x05\xfe\x3f\xbf\xc8\x99\x91\xd9\x2f\xb8\xef\xd1\x38\xe9"
        "\xda\x24\x2f\x8f\x97\x25\xa1\xba\x55\x8e\x08\x6b\x56\xa8\x17\x7d\xe8"
        "\x9d\xdb\xc2\x59\x87\x09\x73\x0c\xa0\xbe\x57\x44\xe8\xb4\x23\xec\x53"
        "\xd4\x49\xb3\x21\x65\xa3\x81\x2c\x76\x12\x33\xc3\x9d\x18\x37\xae\xff"
        "\xd7\xca\xb6\xe4\x49\xa0\x90\x57\x34\x71\xcf\x4c\x48\xab\x40\xd1\xcc"
        "\x3f\x40\x6a\xc1\x27\x75\x42\x02\x8d\x70\x68\x40\x1e\x0f\x99\xce\x2c"
        "\x89\xf2\x1a\x31\xa5\x39\x05\xf8\x42\x97\x7c\x6f\x63\x26\x62\x05\x5b"
        "\x84\xd9\x56\x09\xf7\xd1\x15\x27\x3a\x9f\xc5\x59\x10\x69\x6d\xf2\x4f"
        "\xdc\x60\x70\x62\x9a\x84\xc2\x8a\xa1\x2c\x7f\x72\x3e\x46\xd2\x8c\xcd"
        "\x63\x0f\x7f\x9e\x91\x40\x33\xf0\xeb\xdc\x62\x03\x99\x77\xfe\x82\x19"
        "\x84\xb8\x54\x9f\x94\x3b\xfb\x11\x2f\x55\xc0\xf5\xf4\x1f\x9d\x53\xba"
        "\x0b\x15\xd6\x84\x45\x05\xa4\x10\xdd\x92\x16\x0d\x3a\xdc\xb3\x18\x59"
        "\xcf\x2b\xdd\x9d\x65\xeb\xfe\x7f\x8d\x37\xbd\xc5\xd0\x24\xee\x48\x0d"
        "\x18\x73\xd4\x92\x24\xde\xf6\xdb\x9d\x62\xca\xcf\xd6\xa0\x43\x12\x2c"
        "\xbe\xef\x30\xd8\xd9\x3f\xa2\xe0\x71\xef\x7c\x4e\xb2\x2e\xe4\xd9\xe6"
        "\xcc\x79\xf8\xfa\x84\xbf\x22\x3c\x92\x82\xf9\x81\xe2\x04\xc8\x1b\xe7"
        "\xb4\x53\x89\x31\x6a\x47\xc3\x6f\x6c\x84\x82\x60\xf4\xc4\x09\xba\x17"
        "\x1b\xff\x24\x94\xe8\x2f\x25\x14\x87\xe5\x26\xdb\xc2\x55\x8a\x5f\x73"
        "\x1d\x6c\xf3\x2f\x7f\xd5\x18\xb3\xb8\x3c\xfb\x32\xb0\x47\x3c\xf0\x19"
        "\x30\x6f\x8b\x19\x8b\x00\x61\x26\xbf\x77\x4c\xcb\xa0\xfa\xe5\xe4\xc7"
        "\x68\x17\x4b\x77\x3a\x97\x8c\x2f\xf8\x95\x1d\x6d\x2d\x1c\x0f\xa5\xf2"
        "\x06\x58\x01\x4e\xb8\x4b\x05\x55\xac\xea\x86\x60\xb6\x89\x24\x08\xb5"
        "\x02\xb0\x50\xa5\x2b\x88\x7a\x28\x38\x27\x8b\xca\x5c\x33\xfb\x9f\x41"
        "\x38\xcd\x15\xd4\x23\x5e\xcc\x41\x14\xa1\x67\x2f\x62\xc7\xd4\xeb\x2f"
        "\x3d\xdb\xc4\x38\x67\x5d\x38\x3a\x57\xb9\x64\xe5\x18\xf5\x05\xe9\xfd"
        "\xfe\xc4\xb9\x30\x2c\xd9\x25\xcc\xbf\x81\x32\xc2\xf1\x9a\xa9\x1d\x6a"
        "\x29\xad\x82\xe5\x23\x73\x51\x5a\xe7\x40\x75\x0d\x23\xac\xb7\x92\x0d"
        "\x2b\x3d\x5b\xd9\x8f\xc9\x3e\x30\x91\xd3\xd4\x3d\x31\xfa\x92\x04\xec"
        "\xa0\x88\xc6\x2c\x28\xd6\x90\xb7\x72\xdc\x08\x30\xba\xd9\xeb\x7b\x68"
        "\xac\xc8\xc6\x10\xea\x9f\x65\x39\x4f\x49\x88\xe1\x2c\x55\xb9\x32\x73"
        "\x1e\x1f\x1e\xe2\xe9\xd1\xff\x8f\x22\x36\x8e\x5f\x17\xe5\xaf\xaa\x3c"
        "\xa0\x85\x30\x2a\x1b\xf0\xe8\xdf\xbd\x6f\x4a\xd3\xdb\x2e\xf8\x9a\x95"
        "\xa2\x99\xa8\xdf\x0a\x90\x39\xeb\x7a\x17\xd0\x30\x2e\xf6\xd0\xfc\x25"
        "\x5b\xd4\x77\x0f\xef\xcd\x8f\xdb\x0a\xc0\x52\x9f\x58\x40\xf7\x79\xd9"
        "\x35\xd7\x58\x9a\xd6\x45\x59\xf6\x1f\x44\xea\xf8\xac\x8e\x00\xf3\x18"
        "\x92\xff\x15\x3b\xe4\xba\x1a\x79\x74\x26\xf4\xe5\xda\x70\x64\xea\xe3"
        "\x07\x28\xe0\xe4\x09\x14\xc7\x45\x1f\xc9\xcd\x07\x48\x51\x4b\x9f\x3d"
        "\x17\xa8\x13\x57\xdd\xc0\xb7\x5b\xfd\xc4\x68\x6e\xfe\x91\x4c\x7d\x90"
        "\xbe\x75\xbc\xf1\x63\x83\xcd\x63\xdc\x5c\x14\xdf\x9f\xa6\xe6\x1c\x04"
        "\xe4\xb9\xf9\x8b\xe7\x57\x22\x6a\x59\x4c\x4f\x67\x06\x2c\x7d\xb8\x35"
        "\xcf\xcd\x33\xf8\xa9\x36\xaa\x81\xe5\x97\x8b\x30\x25\x59\x5f\xc6\x11"
        "\x72\x41\xc7\x46\xa0\x95\x19\x43\xdb\x58\x4f\x5e\x4b\xe3\xb4\xdb\xc3"
        "\xa3\x2f\xa6\xc4\x7c\x9f\xe3\xb5\x81\x49\x45\xd1\x92\xfc\x02\xc0\x89"
        "\xde\x41\x65\x95\x79\x55\x22\x4f\x99\xfe\xc5\x23\x19\x91\x6b\xf5\x9e"
        "\x70\x62\x12\x68\x59\xa8\x48\xcf\xfe\x44\x68\xd5\x93\xca\x59\x8a\x0d"
        "\xa9\xdf\xb9\xb1\xec\x4c\xa3\xfe\xb1\x4e\xcc\xa1\x65\xe4\xaa\x58\xc6"
        "\xae\x1a\x30\x4e\x38\xaa\xc6\xf4\xae\xc0\x8a\x49\x6b\x2c\xbb\x33\xf0"
        "\x41\x45\x96\x0d\xbc\x31\xfd\x22\xbb\x70\x87\xa3\xa3\x06\x0b\x95\x00"
        "\x81\xa1\x19\x36\x17\xc7\x56\x09\x10\x68\x60\xdd\x48\xa2\xca\x62\x21"
        "\x04\x51\xaf\x14\x72\xba\x9c\x65\x7b\x4e\xd2\x7f\x5d\x48\x70\xd0\x52"
        "\x18\xd7\xb7\xca\x8a\x53\x94\x0b\x24\x04\x1e\xd1\xc4\x26\x87\xfa\x78"
        "\xd7\xb4\x3b\xa8\xbe\xde\x3c\xfd\x7e\x3c\x42\xdc\x9a\x2e\xfa\x96\x55"
        "\x2a\xcb\x98\x4c\x69\x8c\x74\xbb\x09\x28\x83\xe4\x23\x47\x76\xe0\x87"
        "\x6d\x3d\xbc\x9a\x3d\x03\x9c\x79\xb0\x87\x23\xdb\x8c\x6a\x85\x7b\xfc"
        "\x65\xd7\x29\x74\x2e\x5e\xb6\x7e\xaa\x19\x4c\x2d\x0f\x0e\x32\x00\xa8"
        "\x05\x45\xe0\xb8\x02\x33\x22\xc2\x64\x3f\x31\x17\xd0\x1d\x78\xf5\xf2"
        "\xcc\x5b\xfd\x2b\x8d\xc7\xa4\xd7\x9d\xf5\x3d\x7d\x37\x7e\x5b\xae\x7f"
        "\x5c\x77\x96\x74\x31\x27\xf6\xd0\xcc\xff\x60\x04\x60\xfc\x34\x5a\x07"
        "\x4b\x1e\x57\xdf\xec\x08\x69\x3f\xe8\x7c\xe8\x86\x09\x5f\x11\xd7\xe3"
        "\xe8\xab\x82\x7a\x37\xe1\x84\x85\x16\x6d\xea\xf4\x60\xf5\x55\xa3\x5c"
        "\xfc\xa9\x9c\x69\x21\xbd\x8d\x61\xc4\x03\x2e\x66\xd4\x58\x83\x89\xb6"
        "\xe6\xce\xab\x78\x33\x1e\x0f\x94\x98\x5b\x6b\x36\xca\x13\x66\x8f\x7b"
        "\x3a\x00\x74\x18\x8e\xad\xb7\xe9\xa0\x65\x53\x97\x74\xa9\x87\x95\xc3"
        "\xb9\x80\x76\x27\xe0\x90\x6e\x2f\x1d\x4e\x74\x74\x43\x2b\xcd\x38\xbd"
        "\xd6\x11\x59\x8b\x12\x1f\xf8\x11\xe3\xfc\xe4\xc9\x8a\x10\x69\xda\x4d"
        "\xc0\x6a\x8b\xff\x5e\xfa\x9b\xc9\xbb\x34\x80\x46\x9e\x7d\x67\x52\xb9"
        "\x23\x18\x66\xda\x00\x18\xb3\xab\xf1\xd4\xae\x5b\x72\x4a\x56\xb1\x15"
        "\x69\xbf\x94\xf9\x17\x08\xb2\x17\x2b\x76\xd7\xd7\x7f\x34\xd6\xda\xa1"
        "\xe5\x64\xb7\x12\x64\x3a\xbc\xa2\xb1\xda\x24\xa9\x40\xf7\xbf\x86\xcf"
        "\x55\xfc\x18\xc7\x5b\x8f\xa2\xf5\xb9\xa8\x01\xfd\x10\x63\xc3\x20\x52"
        "\x42\xbe\xce\x48\xb9\x19\x57\x63\x0c\xaa\x0c\x64\xc0\x0e\x24\x6e\xe4"
        "\x62\x1f\xbf\x55\xde\xf7\x19\xfd\x33\xc9\x2a\x14\x1e\xf8\xfd\x5d\xfe"
        "\x06\x94\xa1\x1c\xf5\x5b\xce\xec\x29\xe9\x95\x7a\x95\xdf\x92\xb2\x8b"
        "\x3f\x14\xd3\x93\x38\x99\x95\xf5\x62\x18\x7d\xf1\xc3\xd6\xb8\x26\xbd"
        "\x8a\x19\x81\x33\x3b\x00\xe7\x08\xe6\xcf\x99\xed\x6a\xc3\x59\x9d\xbe"
        "\x3a\x0a\x15\xdc\xc7\x87\x48\x8a\xb2\x94\x25\x1f\xc4\x2e\x29\x4e\x4c"
        "\x93\xa6\x2a\x82\x01\xac\xee\xb3\xba\x23\xca\x3f\xd8\x16\x20\xc9\x2a"
        "\x4c\x00\x11\x87\xd1\x82\x7d\x5b\x77\x4d\x36\x2f\xda\xfa\xb9\x2a\x0b"
        "\x1a\x1d\xb5\xf5\x61\xd4\x7f\x4a\xe6\xa3\x26\xdc\x75\xb6\x0f\x1d\x0f"
        "\x18\xdc\xcf\x0f\x26\xa6\xb4\x88\xc0\x88\x43\x13\xe4\xa5\x3e\x35\x92"
        "\x66\x1f\xe0\x57\xd0\x7b\xf5\x8c\xdb\x3c\x86\xf1\x72\x69\xc9\x1b\xc6"
        "\x9a\x63\x69\x3d\x4e\x2a\x9c\x01\xad\x65\x8d\x47\xf4\xb8\xae\xb6\x8b"
        "\x63\xe9\x0d\x15\x9a\x8a\x61\x1e\xd8\xac\x08\x9e\x9c\xb7\xe1\x63\x6e"
        "\x47\x67\xe2\x50\x69\x9f\xab\x55\x1e\x37\x0c\xd7\x58\x21\x4d\x0c\xf1"
        "\x69\x3b\x6d\xc7\x7b\xc4\x18\x04\x45\x7c\x3c\x45\xae\x92\xa7\xd4\xf8"
        "\x89\xf1\x62\x94\x15\x2e\x0f\x00\x3d\x2a\x78\x53\x16\xe8\x1b\x8d\xaa"
        "\xa5\xd6\xca\xf3\x4c\xd7\xea\x1c\x89\x93\xa5\x84\x3c\x75\x4a\xcc\x2c"
        "\xc2\x9e\x1c\x3d\x91\x21\xd4\xb4\x2b\x2d\x5f\x7f\x3f\xe9\x91\x4a\xc6"
        "\x8b\x7a\x50\xab\x44\x79\x3f\xf1\x34\x9f\xc9\x1c\xe8\xd2\x9c\x2d\xf0"
        "\x8f\xdc\x8d\x84\x1b\x5c\x75\x22\x5f\xe7\x94\x79\x56\x9b\xff\x2c\x54"
        "\x1b\x40\x97\x2b\x4d\x87\x2e\xf7\x17\x18\xcf\x41\xf2\xb0\xa4\x11\xa0"
        "\xf2\x5d\xcd\x79\x1c\xf9\x9e\x19\xa8\x2a\x8a\x18\xec\x1f\x2b\x55\xdb"
        "\x13\x37\x7d\xc7\x78\x3e\x2a\x25\x32\x15\xd1\x49\x9c\x50\xac\x94\xfb"
        "\x51\x42\xd0\xde\x87\x32\x87\xde\x57\xab\xa7\x3e\x7a\xf8\x52\x98\x17"
        "\x13\x84\x5c\x9e\x72\x4a\x5a\x89\xa9\xf8\x7a\x0a\xc5\x20\x39\x89\x54"
        "\xf1\x9c\x62\xe6\x7d\x11\x9b\x15\x7e\x16\x2b\xa0\x0f\x8d\x42\xa8\x57"
        "\x4f\xf3\x80\xa4\x5b\xfe\xe1\xca\xe5\x5f\xc6\x3d\xd8\x40\x15\x42\x4e"
        "\xe5\xa9\xf7\x02\x98\xba\x61\x67\x14\x13\xf4\x09\x81\xf4\x2c\xc6\x44"
        "\x38\x1b\xd3\xad\x7b\xed\x21\xd6\xc8\x60\x42\xcb\x58\x1d\x2d\x08\x44"
        "\xff\xb5\x96\xff\x9e\xde\x13\xef\x81\xeb\x70\xca\xae\x69\x7b\xd9\x1f"
        "\x11\x61\x4d\x51\xc3\x5f\x8e\xa2\x15\xae\x56\x8a\x06\x18\x1d\xa7\x32"
        "\xf3\x25\xbb\x1e\xc9\x96\x74\xad\x7e\xa5\x7d\x32\x76\x2f\x4e\xf6\x8d"
        "\xc3\x58\x81\xbb\xc7\xfa\x32\x5c\x66\xdf\x45\xba\xee\xfb\x09\x43\xe3"
        "\x35\x74\xb6\x47\xb3\x69\xf8\x54\xe8\x02\x07\x00\x65\xa3\xa0\xa3\xae"
        "\x01\x47\x1d\x02\x85\x69\x23\x80\xe9\xf4\x25\x99\x71\xe5\xea\x63\x88"
        "\x7f\x32\xd2\x47\x2c\x1c\x5a\x86\x86\x23\x07\xe7\xbb\x14\x69\x48\xe6"
        "\x6d\x73\xa3\xa4\xd5\x59\xec\xee\x05\x3b\x86\xb3\xa3\x12\x6f\xf9\xf0"
        "\x97\x0c\x5b\x00\x43\x46\x0e\xbd\x11\xa6\x87\x89\x1f\x80\xc1\x48\x4a"
        "\x37\xe2\x24\xa6\xb1\x0d\xb9\xbf\x04\xc4\xc8\x5d\x89\xb0\x24\x40\xcc"
        "\x94\xe7\x37\x33\xf6\x04\x23\x4a\xb4\x0f\xd1\x65\xc8\x93\x7a\x86\xd4"
        "\x18\xfb\x8c\x43\x6d\x0b\x38\xc5\x4d\x02\x95\x38\x45\xd0\x36\x15\x51"
        "\xdc\x39\x27\x14\x7a\xa3\x30\x5f\x43\xa7\x64\x78\x33\xb6\xb8\xc5\xa9"
        "\x04\x3d\xa7\x3b\xc8\x25\x3e\xb9\xe0\x05\x28\xf0\x89\xd9\x52\x9c\x13"
        "\xd0\x62\x04\x6e\x4f\xcf\xe9\x3c\x61\xce\x3e\x00\x23\x9f\xaa\x22\x28"
        "\x19\xb8\x65\xba\x50\xa1\xca\x97\x8d\xd7\x7d\x52\xe3\x35\xed\x3d\x2f"
        "\xeb\xb3\x3d\x56\x88\xe2\xf2\xf7\x94\xbf\x21\x02\x30\xcd\x49\xd2\xa9"
        "\x9a\x07\xed\x28\x8d\x49\x51\x00\x0c\xdf\xc9\x1d\x24\x69\x82\x5a\xf5"
        "\x5c\x8f\x26\x68\x83\x56\xdd\x68\x66\x2c\x5b\x08\x96\x60\x3c\x64\x91"
        "\x18\x25\x77\x59\x68\xd0\x99\xcf\x14\x48\x43\xb4\x01\x55\xdd\x59\xad"
        "\x82\xe1\xc9\xd4\x7f\xc0\x41\x50\x75\x35\x0f\xf9\x69\xa1\x11\x68\x1e"
        "\x73\x19\x4c\x62\x7e\xc2\xaf\xd1\x12\x91\xc8\x1e\x23\x63\x47\x7e\x78"
        "\xb6\x8f\x33\x10\x72\xa5\x70\x70\x6e\xa2\xb0\xb2\x8c\x91\xde\x5d\x94"
        "\xbd\x57\x8f\x2d\x37\x7b\xde\x28\x63\x50\x07\xb3\x75\x42\x2e\xac\x8a"
        "\x11\x6c\xc0\x87\x2d\xa2\xc5\x4d\xa6\xa2\xf1\x4e\x14\x99\xd1\x6d\x8d"
        "\xc4\x77\xd4\x59\x46\x7a\x2a\xa1\x91\xa1\xf2\xb8\xed\x65\x3e\xa8\x5c"
        "\x29\x09\x0a\x97\x1a\x8b\x7d\x79\xe6\xb4\x3c\x26\x80\x09\x2b\xa5\xd3"
        "\x5b\x46\xf2\xa3\x5b\x3d\x4d\x2b\x76\xf1\x14\x35\x2a\xa3\x9e\xb0\x32"
        "\x4a\x7a\x52\xd0\x3f\x04\xbb\x1d\x7f\x6f\xfd\x40\x8c\x6b\x33\x80\x3d"
        "\xe7\x2e\x63\xda\x51\x9a\x6e\xda\x0a\x8d\x99\x24\x41\xcc\xaa\x30\x23"
        "\x46\x57\x40\x5c\x1a\xf7\x0d\x64\x35\xb3\x67\x55\x14\xa7\xfc\x48\x94"
        "\x73\x73\x42\x0f\xb2\xca\xab\x90\xb8\xc6\x61\xd2\x43\x80\x97\xd3\xba"
        "\x40\x87\xc9\xbb\x6d\xb3\x82\x02\x85\xd7\x67\x79\x51\x24\x4e\xee\x2e"
        "\xa7\x7b\xd1\xd2\x87\x53\x87\x2a\x41\x21\x22\x17\x02\xce\x60\x84\xf3"
        "\xb0\xbe\xf6\x3b\x7b\x77\x5f\x5f\x5d\x93\xf3\xa0\x41\xb1\x34\xea\xdf"
        "\x7f\x3a\xfb\x3f\xb9\xaa\x9b\xf1\xed\x41\x51\x87\xb3\xb7\xeb\xdd\xf7"
        "\x8c\x96\xd0\x4a\xc0\xef\x8a\xef\xba\xf0\x0e\x2e\x0b\xcb\xee\x63\x2b"
        "\x7e\x01\xbc\x62\x11\xd5\x4c\xb5\xea\x30\x39\xfa\x11\x48\x1e\x66\x47"
        "\x0e\x2b\x3d\xb1\xb5\xef\xd7\xe7\xa0\xbd\x98\x3d\x75\xe5\x13\x9a\x25"
        "\x8f\xf0\xb4\xf6\x0c\x7d\x36\xcc\x24\x6d\x94\xeb\xe8\xea\xf8\x76\x60"
        "\xf4\x0a\xc0\xa8\x89\x35\xa3\x2b\xf1\x47\x46\x9f\x50\x5b\xb7\xd3\xf2"
        "\xf7\xe7\xf7\xbe\x4d\x6b\x13\x9c\x86\x7c\x04\x7b\x38\x1f\x38\x86\xd5"
        "\x79\x83\x8f\x90\xba\x22\x72\x39\x8e\x4f\xfc\x22\x93\xf3\xad\xcf\x7b"
        "\x2c\x48\x35\xf8\xde\xe2\xf0\xf1\x3d\xe5\xea\x05\xcd\x36\xfe\x9e\xf3"
        "\x78\x58\x38\x64\x77\xc6\x8d\xdb\x9e\x41\xb7\x48\x77\x6f\x90\xda\x31"
        "\x4a\xa1\xee\x68\x9e\x19\xf7\xf8\x8b\x2e\xce\x66\xb8\x29\x04\xdd\x27"
        "\x28\x69\xa2\xf5\x6c\xec\x5c\xd6\xf8\xaf\x39\x3b\x6e\x0f\x34\x3f\xb6"
        "\x9f\xab\x39\x8f\x12\x47\x03\xf1\x61\xee\xdc\xb3\xaf\x7c\xa7\xab\x4d"
        "\x5e\x71\xe7\x0c\x63\xd1\x2c\xb6\xb8\x3e\x9c\x10\x7d\x8f\xcb\x89\xb6"
        "\x53\xe6\x35\xb2\xd8\xb0\x44\x11\x03\x93\xbe\x36\xe8\x64\xd3\xcf\xd4"
        "\xc2\x99\xfa\x58\x09\xba\x78\x83\xfe\x08\xb5\x1b\xb9\xf9\x17\xee\x3a"
        "\x81\x4a\xd2\x88\x17\x37\xfd\x7f\x26\xda\x7e\x80\x73\x67\xf5\x41\x65"
        "\xd9\x7e\x8e\x65\x0a\xe2\xf1\xba\xbd\x04\xc3\xb8\xc9\x12\xb8\x16\xf0"
        "\x75\xd0\x01\xc8\x41\xda\xf2\x71\x1f\x6c\x8b\x2f\x57\xa1\x6e\x86\xe9"
        "\x9e\xec\x94\xa3\xdd\x11\xf7\x6d\x47\xdc\x21\x29\x56\x9b\x5c\x6d\x02"
        "\xf5\x28\xc0\xc8\x3e\x4f\x31\x38\x21\x01\xc6\x42\x6e\xc9\xb3\xcc\xf5"
        "\x20\x35\x5a\x35\xde\x99\x2f\x0c\xcc\x03\xaf\x65\x13\x6a\xa7\x1a\x51"
        "\xb5\xa6\xb9\x09\x4f\x16\x8a\x36\xff\xce\xfb\x7d\x32\xc7\x52\x48\xf8"
        "\x7b\xea\xf3\xfe\x32\x6a\xd5\xfe\xcb\x6c\xad\x91\xe0\xeb\x83\xdb\x07"
        "\xf6\x0b\x70\x99\x49\xe6\x96\x66\x33\x6c\x92\x75\x17\x39\xf7\x85\xcc"
        "\x95\x3e\xa6\xde\x07\x68\x15\xa1\x75\x6f\xae\x45\x0c\xef\x43\xe9\x20"
        "\xac\x75\x43\x80\x09\x91\xd2\xe5\x5e\xc9\xd7\x2b\x8f\xc2\x33\x9c\xf7"
        "\xb2\xa5\xe2\x88\xaa\xa8\x12\xdd\xe0\x23\x55\x0c\x97\xb4\x02\xd8\x9b"
        "\xf3\xec\xd6\xaf\x80\x27\x7d\x2e\x94\xff\x9e\x0e\xfe\x2a\xcf\x77\xea"
        "\x52\x1a\x8e\x67\x7c\x61\x12\xb8\x21\xec\xbe\xda\xf8\xab\xf3\xa5\xdb"
        "\x50\xc2\xc0\x40\x40\x71\xd4\x85\xdd\xad\xc3\xe2\x7c\x3f\xa5\x2f\xea"
        "\xcc\xd0\xfd\x30\x0b\x71\x0f\x8c\x2b\xfa\x50\x49\x8e\x2d\x3b\x27\xa0"
        "\x05\x2d\xdb\x8a\x4f\x79\x0a\x40\xff\xbd\xad\x39\xc1\xfa\x30\xb1\xae"
        "\x19\x5c\x80\x14\x3c\x71\x5b\x43\xde\x0c\xf2\xf1\x60\x45\xe4\x8b\x9e"
        "\x44\xfd\xfa\x43\xea\x64\xf2\xf4\x3f\x7f\xf1\x7e\xdf\x16\x98\xa9\x02"
        "\x61\xfd\x4c\xf1\x36\xd0\xb1\x38\x1f\x7c\xc6\x01\xa9\x90\x96\x9c\x26"
        "\xad\x90\x1a\xad\x0a\x8f\xd7\xe5\xef\x0c\xdf\xbe\xce\x04\x9a\xfe\x8d"
        "\xe5\xaa\x25\x15\x26\xed\xc7\x3a\xd8\xba\x66\xdb\xe4\x14\x26\x09\x2b"
        "\x27\x01\x5c\xa3\x3a\x88\x10\x9c\x7f\x1e\x34\x43\x8c\x5d\x05\x04\xfd"
        "\xe7\x52\xd1\x94\xbb\xcb\xa8\x4b\xe4\xdf\x07\xd2\x21\xd1\x9f\x79\xe7"
        "\x84\x2c\xd8\x94\x06\xdd\xd1\x7e\x51\x2c\xe2\xf2\x69\x21\xf9\x9f\x0f"
        "\x02\x38\x69\x98\x26\x4e\x08\x66\xfd\x2c\x42\xa1\x3c\xf4\xe6\x01\x37"
        "\x8c\xc0\x11\xc9\xfc\x0e\x0c\x0b\x05\x96\x50\xac\xa4\x12\x51\xc3\x56"
        "\x1a\x24\xdf\x70\xd8\xa7\xcb\xa5\xd1\x8e\xf2\xd0\x39\xe0\xea\xfa\xb7"
        "\x7d\xe4\xdd\x14\x95\x6a\x2c\x4a\x65\x92\x9e\x92\x86\x57\x62\x34\xef"
        "\xb1\xe0\xf5\x2d\xa8\x33\xa9\x8a\x28\xb1\xc6\xf0\x25\x9a\x9a\x5b\x44"
        "\xf5\x48\xba\x4b\x6f\x0e\xd7\x8d\xaf\xe2\x8a\xea\x0a\xe5\xe2\x9b\x9b"
        "\xd6\x29\xa7\xf5\xdf\x44\xad\x7e\x9c\x5f\x3a\x06\xe9\x3a\x67\xcb\x39"
        "\xe8\x09\x1e\x3c\x79\x2b\x2c\xb9\xbd\xa6\x69\xfb\x97\x33\xe1\x3d\x15"
        "\x80\x45\x12\x39\x52\xd9\x82\xd8\xd3\xaa\x96\xa9\x45\x46\x7b\x96\x63"
        "\x4e\x88\x0e\xfd\xd6\xa9\x9a\xda\x59\x12\x4f\xc5\x41\x79\x11\xf0\xb3"
        "\xbc\xd9\x81\x4a\xfa\xec\xec\x6b\x32\x37\x6b\x01\x82\xf3\x62\x1f\xb4"
        "\x5e\xaa\x0f\x41\x88\x89\x10\x4a\x4d\xe4\x4e\x44\x97\x4c\x92\xff\x08"
        "\x6f\xd6\x64\xe3\x19\xea\xa5\x90\x4d\x90\xd1\x56\x51\xb4\x16\xef\x4b"
        "\x06\x19\x73\x7b\x10\xc5\xb8\x68\x3d\xed\xa5\x1d\xe1\x78\xf1\xa8\xe1"
        "\x0e\xdf\xc0\x19\x70\xdc\xf5\xff\xe6\x41\x4c\x09\xc8\x4a\x91\x68\xb7"
        "\x37\xb6\x64\xfa\x96\xcc\x20\x3f\x48\x05\xf9\xd8\x13\x9f\x7c\x2f\x7e"
        "\xa8\x31\x45\x35\x9b\xd6\xb4\x1b\xd0\xa4\x47\x94\x56\xe2\xad\xf0\x51"
        "\x4d\x16\xf6\x10\x98\xf8\x8b\x3e\xd3\x58\xde\x48\x27\x8e\x06\x4f\xf6"
        "\xc1\xef\x23\xab\xdd\x85\x38\xab\x90\x54\x69\xfa\xc7\x45",
        3584);
    *(uint64_t*)0x20001a08 = 0xe00;
    *(uint64_t*)0x20001a10 = 0;
    syz_read_part_table(0, 1, 0x20001a00);
    break;
  case 4:
    *(uint64_t*)0x200004c0 = 0x200001c0;
    *(uint16_t*)0x200001c0 = 3;
    memcpy((void*)0x200001c2, "\xd2\x66\x11\x2d\x87\x96\xb9", 7);
    *(uint32_t*)0x200001cc = 3;
    memcpy((void*)0x200001d0, "\x7e\xe9\xf1\x53\xca\xb3\x5e", 7);
    memcpy((void*)0x200001d7, "\xe6\x21\x98\x45\x08\x54\x8f", 7);
    memcpy((void*)0x200001de, "\x53\x37\xd7\x69\x11\xa7\xdf", 7);
    memcpy((void*)0x200001e5, "\xd2\xca\xb2\x14\xb9\x42\x8d", 7);
    memcpy((void*)0x200001ec, "\x5d\x9b\x4d\xd2\x93\xad\x7f", 7);
    memcpy((void*)0x200001f3, "\x16\x86\xf6\x22\x11\xeb\x8f", 7);
    memcpy((void*)0x200001fa, "\xc1\x7e\x64\x25\x28\x48\x8a", 7);
    memcpy((void*)0x20000201, "\xd4\xaa\xf5\x86\x72\xa5\x32", 7);
    *(uint32_t*)0x200004c8 = 0x48;
    *(uint64_t*)0x200004d0 = 0x20000240;
    *(uint64_t*)0x20000240 = 0x200002c0;
    *(uint64_t*)0x20000248 = 0;
    *(uint64_t*)0x20000250 = 0x20000400;
    *(uint64_t*)0x20000258 = 0;
    *(uint64_t*)0x20000260 = 0x20000540;
    *(uint64_t*)0x20000268 = 0;
    *(uint64_t*)0x200004d8 = 3;
    *(uint64_t*)0x200004e0 = 0;
    *(uint64_t*)0x200004e8 = 0;
    *(uint32_t*)0x200004f0 = 0x8040;
    syscall(__NR_recvmsg, -1, 0x200004c0, 0x100);
    break;
  case 5:
    memcpy((void*)0x20000140, "./file0", 8);
    syscall(__NR_openat, 0xffffffffffffff9c, 0x20000140, 0x101080, 1);
    break;
  }
}

void execute_one()
{
  execute(6);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        int pid = do_sandbox_none();
        int status = 0;
        while (waitpid(pid, &status, __WALL) != pid) {
        }
      }
    }
  }
  sleep(1000000);
  return 0;
}
