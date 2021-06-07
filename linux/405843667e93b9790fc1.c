// WARNING in cfg80211_inform_single_bss_frame_data
// https://syzkaller.appspot.com/bug?id=405843667e93b9790fc1
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sched.h>
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
#include <unistd.h>

#include <linux/capability.h>
#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

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

struct nlmsg {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[4096];
};

static void netlink_init(struct nlmsg* nlmsg, int typ, int flags,
                         const void* data, int size)
{
  memset(nlmsg, 0, sizeof(*nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg->pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(struct nlmsg* nlmsg, int typ, const void* data,
                         int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  if (size > 0)
    memcpy(attr + 1, data, size);
  nlmsg->pos += NLMSG_ALIGN(attr->nla_len);
}

static int netlink_send_ext(struct nlmsg* nlmsg, int sock, uint16_t reply_type,
                            int* reply_len)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (reply_len)
    *reply_len = 0;
  if (hdr->nlmsg_type == NLMSG_DONE)
    return 0;
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
  return ((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_send(struct nlmsg* nlmsg, int sock)
{
  return netlink_send_ext(nlmsg, sock, 0, NULL);
}

static int netlink_query_family_id(struct nlmsg* nlmsg, int sock,
                                   const char* family_name)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, family_name,
               strnlen(family_name, GENL_NAMSIZ - 1) + 1);
  int n = 0;
  int err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n);
  if (err < 0) {
    return -1;
  }
  uint16_t id = 0;
  struct nlattr* attr = (struct nlattr*)(nlmsg->buf + NLMSG_HDRLEN +
                                         NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg->buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  return id;
}

static struct nlmsg nlmsg;

#define WIFI_INITIAL_DEVICE_COUNT 2
#define WIFI_MAC_BASE                                                          \
  {                                                                            \
    0x08, 0x02, 0x11, 0x00, 0x00, 0x00                                         \
  }
#define WIFI_IBSS_BSSID                                                        \
  {                                                                            \
    0x50, 0x50, 0x50, 0x50, 0x50, 0x50                                         \
  }
#define WIFI_IBSS_SSID                                                         \
  {                                                                            \
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10                                         \
  }
#define WIFI_DEFAULT_FREQUENCY 2412
#define WIFI_DEFAULT_SIGNAL 0
#define WIFI_DEFAULT_RX_RATE 1
#define HWSIM_CMD_REGISTER 1
#define HWSIM_CMD_FRAME 2
#define HWSIM_CMD_NEW_RADIO 4
#define HWSIM_ATTR_SUPPORT_P2P_DEVICE 14
#define HWSIM_ATTR_PERM_ADDR 22

#define IF_OPER_UP 6
struct join_ibss_props {
  int wiphy_freq;
  bool wiphy_freq_fixed;
  uint8_t* mac;
  uint8_t* ssid;
  int ssid_len;
};

static int set_interface_state(const char* interface_name, int on)
{
  struct ifreq ifr;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return -1;
  }
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, interface_name);
  int ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
  if (ret < 0) {
    close(sock);
    return -1;
  }
  if (on)
    ifr.ifr_flags |= IFF_UP;
  else
    ifr.ifr_flags &= ~IFF_UP;
  ret = ioctl(sock, SIOCSIFFLAGS, &ifr);
  close(sock);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

static int nl80211_set_interface(struct nlmsg* nlmsg, int sock,
                                 int nl80211_family, uint32_t ifindex,
                                 uint32_t iftype)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = NL80211_CMD_SET_INTERFACE;
  netlink_init(nlmsg, nl80211_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, NL80211_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
  netlink_attr(nlmsg, NL80211_ATTR_IFTYPE, &iftype, sizeof(iftype));
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
    return -1;
  }
  return 0;
}

static int nl80211_join_ibss(struct nlmsg* nlmsg, int sock, int nl80211_family,
                             uint32_t ifindex, struct join_ibss_props* props)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = NL80211_CMD_JOIN_IBSS;
  netlink_init(nlmsg, nl80211_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, NL80211_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
  netlink_attr(nlmsg, NL80211_ATTR_SSID, props->ssid, props->ssid_len);
  netlink_attr(nlmsg, NL80211_ATTR_WIPHY_FREQ, &(props->wiphy_freq),
               sizeof(props->wiphy_freq));
  if (props->mac)
    netlink_attr(nlmsg, NL80211_ATTR_MAC, props->mac, ETH_ALEN);
  if (props->wiphy_freq_fixed)
    netlink_attr(nlmsg, NL80211_ATTR_FREQ_FIXED, NULL, 0);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
    return -1;
  }
  return 0;
}

static int get_ifla_operstate(struct nlmsg* nlmsg, int ifindex)
{
  struct ifinfomsg info;
  memset(&info, 0, sizeof(info));
  info.ifi_family = AF_UNSPEC;
  info.ifi_index = ifindex;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1) {
    return -1;
  }
  netlink_init(nlmsg, RTM_GETLINK, 0, &info, sizeof(info));
  int n;
  int err = netlink_send_ext(nlmsg, sock, RTM_NEWLINK, &n);
  close(sock);
  if (err) {
    return -1;
  }
  struct rtattr* attr = IFLA_RTA(NLMSG_DATA(nlmsg->buf));
  for (; RTA_OK(attr, n); attr = RTA_NEXT(attr, n)) {
    if (attr->rta_type == IFLA_OPERSTATE)
      return *((int32_t*)RTA_DATA(attr));
  }
  return -1;
}

static int await_ifla_operstate(struct nlmsg* nlmsg, char* interface,
                                int operstate)
{
  int ifindex = if_nametoindex(interface);
  while (true) {
    usleep(1000);
    int ret = get_ifla_operstate(nlmsg, ifindex);
    if (ret < 0)
      return ret;
    if (ret == operstate)
      return 0;
  }
  return 0;
}

static int nl80211_setup_ibss_interface(struct nlmsg* nlmsg, int sock,
                                        int nl80211_family_id, char* interface,
                                        struct join_ibss_props* ibss_props)
{
  int ifindex = if_nametoindex(interface);
  if (ifindex == 0) {
    return -1;
  }
  int ret = nl80211_set_interface(nlmsg, sock, nl80211_family_id, ifindex,
                                  NL80211_IFTYPE_ADHOC);
  if (ret < 0) {
    return -1;
  }
  ret = set_interface_state(interface, 1);
  if (ret < 0) {
    return -1;
  }
  ret = nl80211_join_ibss(nlmsg, sock, nl80211_family_id, ifindex, ibss_props);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

static int hwsim80211_create_device(struct nlmsg* nlmsg, int sock,
                                    int hwsim_family,
                                    uint8_t mac_addr[ETH_ALEN])
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = HWSIM_CMD_NEW_RADIO;
  netlink_init(nlmsg, hwsim_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, HWSIM_ATTR_SUPPORT_P2P_DEVICE, NULL, 0);
  netlink_attr(nlmsg, HWSIM_ATTR_PERM_ADDR, mac_addr, ETH_ALEN);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
    return -1;
  }
  return 0;
}

static void initialize_wifi_devices(void)
{
  uint8_t mac_addr[6] = WIFI_MAC_BASE;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock < 0) {
    return;
  }
  int hwsim_family_id = netlink_query_family_id(&nlmsg, sock, "MAC80211_HWSIM");
  int nl80211_family_id = netlink_query_family_id(&nlmsg, sock, "nl80211");
  uint8_t ssid[] = WIFI_IBSS_SSID;
  uint8_t bssid[] = WIFI_IBSS_BSSID;
  struct join_ibss_props ibss_props = {.wiphy_freq = WIFI_DEFAULT_FREQUENCY,
                                       .wiphy_freq_fixed = true,
                                       .mac = bssid,
                                       .ssid = ssid,
                                       .ssid_len = sizeof(ssid)};
  for (int device_id = 0; device_id < WIFI_INITIAL_DEVICE_COUNT; device_id++) {
    mac_addr[5] = device_id;
    int ret = hwsim80211_create_device(&nlmsg, sock, hwsim_family_id, mac_addr);
    if (ret < 0)
      exit(1);
    char interface[6] = "wlan0";
    interface[4] += device_id;
    if (nl80211_setup_ibss_interface(&nlmsg, sock, nl80211_family_id, interface,
                                     &ibss_props) < 0)
      exit(1);
  }
  for (int device_id = 0; device_id < WIFI_INITIAL_DEVICE_COUNT; device_id++) {
    char interface[6] = "wlan0";
    interface[4] += device_id;
    int ret = await_ifla_operstate(&nlmsg, interface, IF_OPER_UP);
    if (ret < 0)
      exit(1);
  }
  close(sock);
}

static long syz_genetlink_get_family_id(volatile long name)
{
  struct nlmsg nlmsg_tmp;
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd == -1) {
    return -1;
  }
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name);
  close(fd);
  if (ret < 0) {
    return -1;
  }
  return ret;
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
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
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

static int wait_for_loop(int pid)
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
  initialize_wifi_devices();
  loop();
  exit(1);
}

#define HWSIM_ATTR_RX_RATE 5
#define HWSIM_ATTR_SIGNAL 6
#define HWSIM_ATTR_ADDR_RECEIVER 1
#define HWSIM_ATTR_FRAME 3

#define WIFI_MAX_INJECT_LEN 2048

static int hwsim_register_socket(struct nlmsg* nlmsg, int sock,
                                 int hwsim_family)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = HWSIM_CMD_REGISTER;
  netlink_init(nlmsg, hwsim_family, 0, &genlhdr, sizeof(genlhdr));
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
    return -1;
  }
  return 0;
}

static int hwsim_inject_frame(struct nlmsg* nlmsg, int sock, int hwsim_family,
                              uint8_t* mac_addr, uint8_t* data, int len)
{
  struct genlmsghdr genlhdr;
  uint32_t rx_rate = WIFI_DEFAULT_RX_RATE;
  uint32_t signal = WIFI_DEFAULT_SIGNAL;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = HWSIM_CMD_FRAME;
  netlink_init(nlmsg, hwsim_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, HWSIM_ATTR_RX_RATE, &rx_rate, sizeof(rx_rate));
  netlink_attr(nlmsg, HWSIM_ATTR_SIGNAL, &signal, sizeof(signal));
  netlink_attr(nlmsg, HWSIM_ATTR_ADDR_RECEIVER, mac_addr, ETH_ALEN);
  netlink_attr(nlmsg, HWSIM_ATTR_FRAME, data, len);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
    return -1;
  }
  return 0;
}

static long syz_80211_inject_frame(volatile long a0, volatile long a1,
                                   volatile long a2)
{
  uint8_t* mac_addr = (uint8_t*)a0;
  uint8_t* buf = (uint8_t*)a1;
  int buf_len = (int)a2;
  struct nlmsg tmp_msg;
  if (buf_len < 0 || buf_len > WIFI_MAX_INJECT_LEN) {
    return -1;
  }
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock < 0) {
    return -1;
  }
  int hwsim_family_id =
      netlink_query_family_id(&tmp_msg, sock, "MAC80211_HWSIM");
  int ret = hwsim_register_socket(&tmp_msg, sock, hwsim_family_id);
  if (ret < 0) {
    close(sock);
    return -1;
  }
  ret = hwsim_inject_frame(&tmp_msg, sock, hwsim_family_id, mac_addr, buf,
                           buf_len);
  close(sock);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0x0};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000200, "nl80211\000", 8);
  res = -1;
  res = syz_genetlink_get_family_id(0x20000200);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000700, "wlan1\000\000\000\000\000\000\000\000\000\000\000",
         16);
  res = syscall(__NR_ioctl, r[0], 0x8933, 0x20000700ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000710;
  *(uint64_t*)0x20000340 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint64_t*)0x20000350 = 0x20000300;
  *(uint64_t*)0x20000300 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x34;
  *(uint16_t*)0x20000244 = r[1];
  *(uint16_t*)0x20000246 = 5;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 0x21;
  *(uint8_t*)0x20000251 = 0;
  *(uint16_t*)0x20000252 = 0;
  *(uint16_t*)0x20000254 = 8;
  *(uint16_t*)0x20000256 = 3;
  *(uint32_t*)0x20000258 = r[2];
  *(uint16_t*)0x2000025c = 6;
  *(uint16_t*)0x2000025e = 0x2a;
  *(uint8_t*)0x20000260 = 0xdd;
  *(uint8_t*)0x20000261 = 0;
  *(uint16_t*)0x20000264 = 0x10;
  STORE_BY_BITMASK(uint16_t, , 0x20000266, 0x2d, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000267, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000267, 1, 7, 1);
  *(uint16_t*)0x20000268 = 0xa;
  *(uint16_t*)0x2000026a = 0;
  *(uint8_t*)0x2000026c = 2;
  *(uint8_t*)0x2000026d = 2;
  *(uint8_t*)0x2000026e = 2;
  *(uint8_t*)0x2000026f = 2;
  *(uint8_t*)0x20000270 = 2;
  *(uint8_t*)0x20000271 = 2;
  *(uint64_t*)0x20000308 = 0x34;
  *(uint64_t*)0x20000358 = 1;
  *(uint64_t*)0x20000360 = 0;
  *(uint64_t*)0x20000368 = 0;
  *(uint32_t*)0x20000370 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000340ul, 0ul);
  *(uint8_t*)0x20000300 = 8;
  *(uint8_t*)0x20000301 = 2;
  *(uint8_t*)0x20000302 = 0x11;
  *(uint8_t*)0x20000303 = 0;
  *(uint8_t*)0x20000304 = 0;
  *(uint8_t*)0x20000305 = 1;
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 0, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 8, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 0, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 1, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 2, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 4, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 5, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 6, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 7, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000042, 0, 0, 15);
  STORE_BY_BITMASK(uint16_t, , 0x20000043, 0, 7, 1);
  *(uint8_t*)0x20000044 = 8;
  *(uint8_t*)0x20000045 = 2;
  *(uint8_t*)0x20000046 = 0x11;
  *(uint8_t*)0x20000047 = 0;
  *(uint8_t*)0x20000048 = 0;
  *(uint8_t*)0x20000049 = 1;
  *(uint8_t*)0x2000004a = 8;
  *(uint8_t*)0x2000004b = 2;
  *(uint8_t*)0x2000004c = 0x11;
  *(uint8_t*)0x2000004d = 0;
  *(uint8_t*)0x2000004e = 0;
  *(uint8_t*)0x2000004f = 0;
  *(uint8_t*)0x20000050 = 8;
  *(uint8_t*)0x20000051 = 2;
  *(uint8_t*)0x20000052 = 0x11;
  *(uint8_t*)0x20000053 = 0;
  *(uint8_t*)0x20000054 = 0;
  *(uint8_t*)0x20000055 = 0;
  STORE_BY_BITMASK(uint16_t, , 0x20000056, 0, 0, 4);
  STORE_BY_BITMASK(uint16_t, , 0x20000056, 0, 4, 12);
  *(uint64_t*)0x20000058 = 0;
  *(uint16_t*)0x20000060 = 0x64;
  *(uint16_t*)0x20000062 = 1;
  *(uint8_t*)0x20000064 = 0;
  *(uint8_t*)0x20000065 = 6;
  *(uint8_t*)0x20000066 = 2;
  *(uint8_t*)0x20000067 = 2;
  *(uint8_t*)0x20000068 = 2;
  *(uint8_t*)0x20000069 = 2;
  *(uint8_t*)0x2000006a = 2;
  *(uint8_t*)0x2000006b = 2;
  *(uint8_t*)0x2000006c = 1;
  *(uint8_t*)0x2000006d = 8;
  STORE_BY_BITMASK(uint8_t, , 0x2000006e, 2, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x2000006e, 1, 7, 1);
  STORE_BY_BITMASK(uint8_t, , 0x2000006f, 4, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x2000006f, 1, 7, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000070, 0xb, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x20000070, 1, 7, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000071, 0x16, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x20000071, 1, 7, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000072, 0xc, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x20000072, 0, 7, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000073, 0x12, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x20000073, 0, 7, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000074, 0x18, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x20000074, 0, 7, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000075, 0x24, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x20000075, 0, 7, 1);
  syz_80211_inject_frame(0x20000300, 0x20000040, 0x23);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
