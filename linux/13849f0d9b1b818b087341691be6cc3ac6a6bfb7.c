// KASAN: use-after-free Read in relay_switch_subbuf
// https://syzkaller.appspot.com/bug?id=13849f0d9b1b818b087341691be6cc3ac6a6bfb7
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/if_tun.h>
#include <linux/in6.h>
#include <linux/ip.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/tcp.h>
#include <linux/veth.h>

unsigned long long procid;

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

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

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
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

static struct {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[1024];
} nlmsg;

static void netlink_init(int typ, int flags, const void* data, int size)
{
  memset(&nlmsg, 0, sizeof(nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg.buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg.pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(int typ, const void* data, int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg.pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  memcpy(attr + 1, data, size);
  nlmsg.pos += NLMSG_ALIGN(attr->nla_len);
}

static int netlink_send_ext(int sock, uint16_t reply_type, int* reply_len)
{
  if (nlmsg.pos > nlmsg.buf + sizeof(nlmsg.buf) || nlmsg.nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg.buf;
  hdr->nlmsg_len = nlmsg.pos - nlmsg.buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg.buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg.buf, sizeof(nlmsg.buf), 0);
  if (n < sizeof(struct nlmsghdr))
    exit(1);
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_send(int sock)
{
  return netlink_send_ext(sock, 0, NULL);
}

static void netlink_device_change(int sock, const char* name, bool up,
                                  const char* master, const void* mac,
                                  int macsize)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  netlink_init(RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  netlink_attr(IFLA_IFNAME, name, strlen(name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(sock);
  (void)err;
}

static int netlink_add_addr(int sock, const char* dev, const void* addr,
                            int addrsize)
{
  struct ifaddrmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ifa_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ifa_prefixlen = addrsize == 4 ? 24 : 120;
  hdr.ifa_scope = RT_SCOPE_UNIVERSE;
  hdr.ifa_index = if_nametoindex(dev);
  netlink_init(RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE, &hdr, sizeof(hdr));
  netlink_attr(IFA_LOCAL, addr, addrsize);
  netlink_attr(IFA_ADDRESS, addr, addrsize);
  return netlink_send(sock);
}

static void netlink_add_addr4(int sock, const char* dev, const char* addr)
{
  struct in_addr in_addr;
  inet_pton(AF_INET, addr, &in_addr);
  int err = netlink_add_addr(sock, dev, &in_addr, sizeof(in_addr));
  (void)err;
}

static void netlink_add_addr6(int sock, const char* dev, const char* addr)
{
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, addr, &in6_addr);
  int err = netlink_add_addr(sock, dev, &in6_addr, sizeof(in6_addr));
  (void)err;
}

static void netlink_add_neigh(int sock, const char* name, const void* addr,
                              int addrsize, const void* mac, int macsize)
{
  struct ndmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ndm_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ndm_ifindex = if_nametoindex(name);
  hdr.ndm_state = NUD_PERMANENT;
  netlink_init(RTM_NEWNEIGH, NLM_F_EXCL | NLM_F_CREATE, &hdr, sizeof(hdr));
  netlink_attr(NDA_DST, addr, addrsize);
  netlink_attr(NDA_LLADDR, mac, macsize);
  int err = netlink_send(sock);
  (void)err;
}

static int tunfd = -1;
static int tun_frags_enabled;
#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define TUN_IFACE "syz_tun"

#define LOCAL_MAC 0xaaaaaaaaaaaa
#define REMOTE_MAC 0xaaaaaaaaaabb

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
  const int kTunFd = 240;
  if (dup2(tunfd, kTunFd) < 0)
    exit(1);
  close(tunfd);
  tunfd = kTunFd;
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      exit(1);
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    exit(1);
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;
  char sysctl[64];
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/accept_dad", TUN_IFACE);
  write_file(sysctl, "0");
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/router_solicitations", TUN_IFACE);
  write_file(sysctl, "0");
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  netlink_add_addr4(sock, TUN_IFACE, LOCAL_IPV4);
  netlink_add_addr6(sock, TUN_IFACE, LOCAL_IPV6);
  uint64_t macaddr = REMOTE_MAC;
  struct in_addr in_addr;
  inet_pton(AF_INET, REMOTE_IPV4, &in_addr);
  netlink_add_neigh(sock, TUN_IFACE, &in_addr, sizeof(in_addr), &macaddr,
                    ETH_ALEN);
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, REMOTE_IPV6, &in6_addr);
  netlink_add_neigh(sock, TUN_IFACE, &in6_addr, sizeof(in6_addr), &macaddr,
                    ETH_ALEN);
  macaddr = LOCAL_MAC;
  netlink_device_change(sock, TUN_IFACE, true, 0, &macaddr, ETH_ALEN);
  close(sock);
}

const int kInitNetNsFd = 239;

#define DEVLINK_FAMILY_NAME "devlink"

#define DEVLINK_CMD_RELOAD 37
#define DEVLINK_ATTR_BUS_NAME 1
#define DEVLINK_ATTR_DEV_NAME 2
#define DEVLINK_ATTR_NETNS_FD 137

static void netlink_devlink_netns_move(const char* bus_name,
                                       const char* dev_name, int netns_fd)
{
  struct genlmsghdr genlhdr;
  struct nlattr* attr;
  int sock, err, n;
  uint16_t id = 0;
  sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(CTRL_ATTR_FAMILY_NAME, DEVLINK_FAMILY_NAME,
               strlen(DEVLINK_FAMILY_NAME) + 1);
  err = netlink_send_ext(sock, GENL_ID_CTRL, &n);
  if (err) {
    goto error;
  }
  attr =
      (struct nlattr*)(nlmsg.buf + NLMSG_HDRLEN + NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg.buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    goto error;
  }
  recv(sock, nlmsg.buf, sizeof(nlmsg.buf), 0); /* recv ack */
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_RELOAD;
  netlink_init(id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  netlink_attr(DEVLINK_ATTR_NETNS_FD, &netns_fd, sizeof(netns_fd));
  netlink_send(sock);
error:
  close(sock);
}

static void initialize_devlink_pci(void)
{
  int netns = open("/proc/self/ns/net", O_RDONLY);
  if (netns == -1)
    exit(1);
  int ret = setns(kInitNetNsFd, 0);
  if (ret == -1)
    exit(1);
  netlink_devlink_netns_move("pci", "0000:00:10.0", netns);
  ret = setns(netns, 0);
  if (ret == -1)
    exit(1);
  close(netns);
}

static int read_tun(char* data, int size)
{
  if (tunfd < 0)
    return -1;
  int rv = read(tunfd, data, size);
  if (rv < 0) {
    if (errno == EAGAIN)
      return -1;
    if (errno == EBADFD)
      return -1;
    exit(1);
  }
  return rv;
}

static void flush_tun()
{
  char data[SYZ_TUN_MAX_PACKET_SIZE];
  while (read_tun(&data[0], sizeof(data)) != -1) {
  }
}

#define MAX_FDS 30

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
    NONFAILING(strncpy(buf, (char*)a0, sizeof(buf) - 1));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

#define XT_TABLE_SIZE 1536
#define XT_MAX_ENTRIES 10

struct xt_counters {
  uint64_t pcnt, bcnt;
};

struct ipt_getinfo {
  char name[32];
  unsigned int valid_hooks;
  unsigned int hook_entry[5];
  unsigned int underflow[5];
  unsigned int num_entries;
  unsigned int size;
};

struct ipt_get_entries {
  char name[32];
  unsigned int size;
  void* entrytable[XT_TABLE_SIZE / sizeof(void*)];
};

struct ipt_replace {
  char name[32];
  unsigned int valid_hooks;
  unsigned int num_entries;
  unsigned int size;
  unsigned int hook_entry[5];
  unsigned int underflow[5];
  unsigned int num_counters;
  struct xt_counters* counters;
  char entrytable[XT_TABLE_SIZE];
};

struct ipt_table_desc {
  const char* name;
  struct ipt_getinfo info;
  struct ipt_replace replace;
};

static struct ipt_table_desc ipv4_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

static struct ipt_table_desc ipv6_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

#define IPT_BASE_CTL 64
#define IPT_SO_SET_REPLACE (IPT_BASE_CTL)
#define IPT_SO_GET_INFO (IPT_BASE_CTL)
#define IPT_SO_GET_ENTRIES (IPT_BASE_CTL + 1)

struct arpt_getinfo {
  char name[32];
  unsigned int valid_hooks;
  unsigned int hook_entry[3];
  unsigned int underflow[3];
  unsigned int num_entries;
  unsigned int size;
};

struct arpt_get_entries {
  char name[32];
  unsigned int size;
  void* entrytable[XT_TABLE_SIZE / sizeof(void*)];
};

struct arpt_replace {
  char name[32];
  unsigned int valid_hooks;
  unsigned int num_entries;
  unsigned int size;
  unsigned int hook_entry[3];
  unsigned int underflow[3];
  unsigned int num_counters;
  struct xt_counters* counters;
  char entrytable[XT_TABLE_SIZE];
};

struct arpt_table_desc {
  const char* name;
  struct arpt_getinfo info;
  struct arpt_replace replace;
};

static struct arpt_table_desc arpt_tables[] = {
    {.name = "filter"},
};

#define ARPT_BASE_CTL 96
#define ARPT_SO_SET_REPLACE (ARPT_BASE_CTL)
#define ARPT_SO_GET_INFO (ARPT_BASE_CTL)
#define ARPT_SO_GET_ENTRIES (ARPT_BASE_CTL + 1)

static void checkpoint_iptables(struct ipt_table_desc* tables, int num_tables,
                                int family, int level)
{
  struct ipt_get_entries entries;
  socklen_t optlen;
  int fd, i;
  fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      exit(1);
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      exit(1);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      exit(1);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
      exit(1);
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, entries.entrytable, table->info.size);
  }
  close(fd);
}

static void reset_iptables(struct ipt_table_desc* tables, int num_tables,
                           int family, int level)
{
  struct xt_counters counters[XT_MAX_ENTRIES];
  struct ipt_get_entries entries;
  struct ipt_getinfo info;
  socklen_t optlen;
  int fd, i;
  fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &info, &optlen))
      exit(1);
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
        exit(1);
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, level, IPT_SO_SET_REPLACE, &table->replace, optlen))
      exit(1);
  }
  close(fd);
}

static void checkpoint_arptables(void)
{
  struct arpt_get_entries entries;
  socklen_t optlen;
  unsigned i;
  int fd;
  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      exit(1);
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      exit(1);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      exit(1);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
      exit(1);
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, entries.entrytable, table->info.size);
  }
  close(fd);
}

static void reset_arptables()
{
  struct xt_counters counters[XT_MAX_ENTRIES];
  struct arpt_get_entries entries;
  struct arpt_getinfo info;
  socklen_t optlen;
  unsigned i;
  int fd;
  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &info, &optlen))
      exit(1);
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
        exit(1);
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    } else {
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, SOL_IP, ARPT_SO_SET_REPLACE, &table->replace, optlen))
      exit(1);
  }
  close(fd);
}

#define NF_BR_NUMHOOKS 6
#define EBT_TABLE_MAXNAMELEN 32
#define EBT_CHAIN_MAXNAMELEN 32
#define EBT_BASE_CTL 128
#define EBT_SO_SET_ENTRIES (EBT_BASE_CTL)
#define EBT_SO_GET_INFO (EBT_BASE_CTL)
#define EBT_SO_GET_ENTRIES (EBT_SO_GET_INFO + 1)
#define EBT_SO_GET_INIT_INFO (EBT_SO_GET_ENTRIES + 1)
#define EBT_SO_GET_INIT_ENTRIES (EBT_SO_GET_INIT_INFO + 1)

struct ebt_replace {
  char name[EBT_TABLE_MAXNAMELEN];
  unsigned int valid_hooks;
  unsigned int nentries;
  unsigned int entries_size;
  struct ebt_entries* hook_entry[NF_BR_NUMHOOKS];
  unsigned int num_counters;
  struct ebt_counter* counters;
  char* entries;
};

struct ebt_entries {
  unsigned int distinguisher;
  char name[EBT_CHAIN_MAXNAMELEN];
  unsigned int counter_offset;
  int policy;
  unsigned int nentries;
  char data[0] __attribute__((aligned(__alignof__(struct ebt_replace))));
};

struct ebt_table_desc {
  const char* name;
  struct ebt_replace replace;
  char entrytable[XT_TABLE_SIZE];
};

static struct ebt_table_desc ebt_tables[] = {
    {.name = "filter"}, {.name = "nat"}, {.name = "broute"},
};

static void checkpoint_ebtables(void)
{
  socklen_t optlen;
  unsigned i;
  int fd;
  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
  for (i = 0; i < sizeof(ebt_tables) / sizeof(ebt_tables[0]); i++) {
    struct ebt_table_desc* table = &ebt_tables[i];
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->replace);
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INIT_INFO, &table->replace,
                   &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      exit(1);
    }
    if (table->replace.entries_size > sizeof(table->entrytable))
      exit(1);
    table->replace.num_counters = 0;
    table->replace.entries = table->entrytable;
    optlen = sizeof(table->replace) + table->replace.entries_size;
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INIT_ENTRIES, &table->replace,
                   &optlen))
      exit(1);
  }
  close(fd);
}

static void reset_ebtables()
{
  struct ebt_replace replace;
  char entrytable[XT_TABLE_SIZE];
  socklen_t optlen;
  unsigned i, j, h;
  int fd;
  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
  for (i = 0; i < sizeof(ebt_tables) / sizeof(ebt_tables[0]); i++) {
    struct ebt_table_desc* table = &ebt_tables[i];
    if (table->replace.valid_hooks == 0)
      continue;
    memset(&replace, 0, sizeof(replace));
    strcpy(replace.name, table->name);
    optlen = sizeof(replace);
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INFO, &replace, &optlen))
      exit(1);
    replace.num_counters = 0;
    table->replace.entries = 0;
    for (h = 0; h < NF_BR_NUMHOOKS; h++)
      table->replace.hook_entry[h] = 0;
    if (memcmp(&table->replace, &replace, sizeof(table->replace)) == 0) {
      memset(&entrytable, 0, sizeof(entrytable));
      replace.entries = entrytable;
      optlen = sizeof(replace) + replace.entries_size;
      if (getsockopt(fd, SOL_IP, EBT_SO_GET_ENTRIES, &replace, &optlen))
        exit(1);
      if (memcmp(table->entrytable, entrytable, replace.entries_size) == 0)
        continue;
    }
    for (j = 0, h = 0; h < NF_BR_NUMHOOKS; h++) {
      if (table->replace.valid_hooks & (1 << h)) {
        table->replace.hook_entry[h] =
            (struct ebt_entries*)table->entrytable + j;
        j++;
      }
    }
    table->replace.entries = table->entrytable;
    optlen = sizeof(table->replace) + table->replace.entries_size;
    if (setsockopt(fd, SOL_IP, EBT_SO_SET_ENTRIES, &table->replace, optlen))
      exit(1);
  }
  close(fd);
}

static void checkpoint_net_namespace(void)
{
  checkpoint_ebtables();
  checkpoint_arptables();
  checkpoint_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                      AF_INET, SOL_IP);
  checkpoint_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                      AF_INET6, SOL_IPV6);
}

static void reset_net_namespace(void)
{
  reset_ebtables();
  reset_arptables();
  reset_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                 AF_INET, SOL_IP);
  reset_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                 AF_INET6, SOL_IPV6);
}

static void setup_cgroups()
{
  if (mkdir("/syzcgroup", 0777)) {
  }
  if (mkdir("/syzcgroup/unified", 0777)) {
  }
  if (mount("none", "/syzcgroup/unified", "cgroup2", 0, NULL)) {
  }
  if (chmod("/syzcgroup/unified", 0777)) {
  }
  write_file("/syzcgroup/unified/cgroup.subtree_control",
             "+cpu +memory +io +pids +rdma");
  if (mkdir("/syzcgroup/cpu", 0777)) {
  }
  if (mount("none", "/syzcgroup/cpu", "cgroup", 0,
            "cpuset,cpuacct,perf_event,hugetlb")) {
  }
  write_file("/syzcgroup/cpu/cgroup.clone_children", "1");
  if (chmod("/syzcgroup/cpu", 0777)) {
  }
  if (mkdir("/syzcgroup/net", 0777)) {
  }
  if (mount("none", "/syzcgroup/net", "cgroup", 0,
            "net_cls,net_prio,devices,freezer")) {
  }
  if (chmod("/syzcgroup/net", 0777)) {
  }
}

static void setup_cgroups_loop()
{
  int pid = getpid();
  char file[128];
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/pids.max", cgroupdir);
  write_file(file, "32");
  snprintf(file, sizeof(file), "%s/memory.low", cgroupdir);
  write_file(file, "%d", 298 << 20);
  snprintf(file, sizeof(file), "%s/memory.high", cgroupdir);
  write_file(file, "%d", 299 << 20);
  snprintf(file, sizeof(file), "%s/memory.max", cgroupdir);
  write_file(file, "%d", 300 << 20);
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
}

static void setup_cgroups_test()
{
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup")) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup.cpu")) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup.net")) {
  }
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
  setup_cgroups();
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  int netns = open("/proc/self/ns/net", O_RDONLY);
  if (netns == -1)
    exit(1);
  if (dup2(netns, kInitNetNsFd) < 0)
    exit(1);
  close(netns);
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
  initialize_devlink_pci();
  initialize_tun();
  loop();
  exit(1);
}

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  while (umount2(dir, MNT_DETACH) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0)
            close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH))
        exit(1);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0)
            close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
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

static void setup_loop()
{
  setup_cgroups_loop();
  checkpoint_net_namespace();
}

static void reset_loop()
{
  reset_net_namespace();
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setup_cgroups_test();
  write_file("/proc/self/oom_score_adj", "1000");
  flush_tun();
}

static void close_fds()
{
  int fd;
  for (fd = 3; fd < MAX_FDS; fd++)
    close(fd);
}

static void setup_binfmt_misc()
{
  if (mount(0, "/proc/sys/fs/binfmt_misc", "binfmt_misc", 0, 0)) {
  }
  write_file("/proc/sys/fs/binfmt_misc/register", ":syz0:M:0:\x01::./file0:");
  write_file("/proc/sys/fs/binfmt_misc/register",
             ":syz1:M:1:\x02::./file0:POC");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  setup_loop();
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    reset_loop();
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
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
    remove_dir(cwdbuf);
  }
}

uint64_t r[4] = {0x0, 0xffffffffffffffff, 0x0, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  NONFAILING(memcpy((void*)0x20000300, "./file0\000", 8));
  syscall(__NR_creat, 0x20000300, 0);
  syscall(__NR_clone, 0x2000100, 0, 0x9999999999999999, 0, -1);
  NONFAILING(memcpy((void*)0x20000140, "./file0\000", 8));
  NONFAILING(memcpy((void*)0x20000180, "devpts\000", 7));
  NONFAILING(memcpy((void*)0x20000200, "devpts\000", 7));
  syscall(__NR_mount, 0, 0x20000140, 0x20000180, 0, 0x20000200);
  res = syscall(__NR_gettid);
  if (res != -1)
    r[0] = res;
  syscall(__NR_tkill, r[0], 0x3c);
  syscall(__NR_getpid);
  syscall(__NR_write, -1, 0, 0);
  NONFAILING(memcpy((void*)0x200009c0, "/dev/loop-control\000", 18));
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x200009c0, 0, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_ioctl, r[1], 0x4c82, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_ioctl, r[1], 0x4c81, r[2]);
  syscall(__NR_ioctl, -1, 0x4c80, r[2]);
  NONFAILING(memcpy((void*)0x200001c0, "/dev/loop#\000", 11));
  res = syz_open_dev(0x200001c0, 0, 0);
  if (res != -1)
    r[3] = res;
  NONFAILING(*(uint8_t*)0x20000040 = 0);
  NONFAILING(*(uint8_t*)0x20000041 = 0);
  NONFAILING(*(uint8_t*)0x20000042 = 0);
  NONFAILING(*(uint8_t*)0x20000043 = 0);
  NONFAILING(*(uint8_t*)0x20000044 = 0);
  NONFAILING(*(uint8_t*)0x20000045 = 0);
  NONFAILING(*(uint8_t*)0x20000046 = 0);
  NONFAILING(*(uint8_t*)0x20000047 = 0);
  NONFAILING(*(uint8_t*)0x20000048 = 0);
  NONFAILING(*(uint8_t*)0x20000049 = 0);
  NONFAILING(*(uint8_t*)0x2000004a = 0);
  NONFAILING(*(uint8_t*)0x2000004b = 0);
  NONFAILING(*(uint8_t*)0x2000004c = 0);
  NONFAILING(*(uint8_t*)0x2000004d = 0);
  NONFAILING(*(uint8_t*)0x2000004e = 0);
  NONFAILING(*(uint8_t*)0x2000004f = 0);
  NONFAILING(*(uint8_t*)0x20000050 = 0);
  NONFAILING(*(uint8_t*)0x20000051 = 0);
  NONFAILING(*(uint8_t*)0x20000052 = 0);
  NONFAILING(*(uint8_t*)0x20000053 = 0);
  NONFAILING(*(uint8_t*)0x20000054 = 0);
  NONFAILING(*(uint8_t*)0x20000055 = 0);
  NONFAILING(*(uint8_t*)0x20000056 = 0);
  NONFAILING(*(uint8_t*)0x20000057 = 0);
  NONFAILING(*(uint8_t*)0x20000058 = 0);
  NONFAILING(*(uint8_t*)0x20000059 = 0);
  NONFAILING(*(uint8_t*)0x2000005a = 0);
  NONFAILING(*(uint8_t*)0x2000005b = 0);
  NONFAILING(*(uint8_t*)0x2000005c = 0);
  NONFAILING(*(uint8_t*)0x2000005d = 0);
  NONFAILING(*(uint8_t*)0x2000005e = 0);
  NONFAILING(*(uint8_t*)0x2000005f = 0);
  NONFAILING(*(uint16_t*)0x20000060 = 0);
  NONFAILING(*(uint32_t*)0x20000064 = 9);
  NONFAILING(*(uint32_t*)0x20000068 = 0x200);
  NONFAILING(*(uint64_t*)0x20000070 = 0);
  NONFAILING(*(uint64_t*)0x20000078 = 0);
  NONFAILING(*(uint32_t*)0x20000080 = 0);
  syscall(__NR_ioctl, r[3], 0xc0481273, 0x20000040);
  syscall(__NR_ioctl, r[3], 0x1274, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  setup_binfmt_misc();
  install_segv_handler();
  use_temporary_dir();
  do_sandbox_none();
  return 0;
}