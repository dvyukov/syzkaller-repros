// WARNING in snd_pcm_hw_params
// https://syzkaller.appspot.com/bug?id=5fae6cfbe79ca79164f070ce9b69045d05dc2c51
// status:fixed
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
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
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

static void exitf(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
}

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
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

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
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
  rv = system(command);
  if (panic && rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define TUN_IFACE "syz_tun"

#define LOCAL_MAC "aa:aa:aa:aa:aa:aa"
#define REMOTE_MAC "aa:aa:aa:aa:aa:bb"

#define LOCAL_IPV4 "172.20.20.170"
#define REMOTE_IPV4 "172.20.20.187"

#define LOCAL_IPV6 "fe80::aa"
#define REMOTE_IPV6 "fe80::bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(void)
{
  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }
  const int kTunFd = 252;
  if (dup2(tunfd, kTunFd) < 0)
    fail("dup2(tunfd, kTunFd) failed");
  close(tunfd);
  tunfd = kTunFd;

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  execute_command(1, "sysctl -w net.ipv6.conf.%s.accept_dad=0", TUN_IFACE);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  TUN_IFACE);

  execute_command(1, "ip link set dev %s address %s", TUN_IFACE, LOCAL_MAC);
  execute_command(1, "ip addr add %s/24 dev %s", LOCAL_IPV4, TUN_IFACE);
  execute_command(1, "ip -6 addr add %s/120 dev %s", LOCAL_IPV6, TUN_IFACE);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV4, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV6, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip link set dev %s up", TUN_IFACE);
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02hx"
#define DEV_MAC "aa:aa:aa:aa:aa:%02hx"

static void initialize_netdevices(void)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "veth"};
  const char* devnames[] = {"lo",       "sit0",    "bridge0", "vcan0",
                            "tunl0",    "gre0",    "gretap0", "ip_vti0",
                            "ip6_vti0", "ip6tnl0", "ip6gre0", "ip6gretap0",
                            "erspan0",  "bond0",   "veth0",   "veth1"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add dev veth1 type veth");
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

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
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
  initialize_tun();
  initialize_netdevices();

  loop();
  doexit(1);
}

static int inject_fault(int nth)
{
  int fd;
  char buf[16];

  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exitf("failed to open /proc/thread-self/fail-nth");
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exitf("failed to write /proc/thread-self/fail-nth");
  return fd;
}

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  memcpy((void*)0x20000100, "/dev/audio", 11);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000100, 0x40001, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20d2af88 = 2;
  *(uint32_t*)0x20d2af8c = 0x78;
  *(uint8_t*)0x20d2af90 = 0xe2;
  *(uint8_t*)0x20d2af91 = 0;
  *(uint8_t*)0x20d2af92 = 0;
  *(uint8_t*)0x20d2af93 = 0;
  *(uint32_t*)0x20d2af94 = 0;
  *(uint64_t*)0x20d2af98 = 0;
  *(uint64_t*)0x20d2afa0 = 0;
  *(uint64_t*)0x20d2afa8 = 0;
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x20d2afb0, 0, 29, 35);
  *(uint32_t*)0x20d2afb8 = 0;
  *(uint32_t*)0x20d2afbc = 0;
  *(uint64_t*)0x20d2afc0 = 0x20000000;
  *(uint64_t*)0x20d2afc8 = 0;
  *(uint64_t*)0x20d2afd0 = 0;
  *(uint64_t*)0x20d2afd8 = 0;
  *(uint32_t*)0x20d2afe0 = 0;
  *(uint32_t*)0x20d2afe4 = 0;
  *(uint64_t*)0x20d2afe8 = 0;
  *(uint32_t*)0x20d2aff0 = 0;
  *(uint16_t*)0x20d2aff4 = 0;
  *(uint16_t*)0x20d2aff6 = 0;
  syscall(__NR_perf_event_open, 0x20d2af88, 0, 0, -1, 0);
  memcpy((void*)0x20000180, "~", 1);
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(18);
  syscall(__NR_write, r[0], 0x20000180, 1);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  int pid = do_sandbox_none();
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}