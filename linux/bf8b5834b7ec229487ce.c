// WARNING in ieee802154_del_device
// https://syzkaller.appspot.com/bug?id=bf8b5834b7ec229487ce
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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
                            int* reply_len, bool dofail)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  ssize_t n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                     (struct sockaddr*)&addr, sizeof(addr));
  if (n != (ssize_t)hdr->nlmsg_len) {
    if (dofail)
      exit(1);
    return -1;
  }
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (reply_len)
    *reply_len = 0;
  if (n < 0) {
    if (dofail)
      exit(1);
    return -1;
  }
  if (n < (ssize_t)sizeof(struct nlmsghdr)) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  if (hdr->nlmsg_type == NLMSG_DONE)
    return 0;
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < (ssize_t)(sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  if (hdr->nlmsg_type != NLMSG_ERROR) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  errno = -((struct nlmsgerr*)(hdr + 1))->error;
  return -errno;
}

static int netlink_query_family_id(struct nlmsg* nlmsg, int sock,
                                   const char* family_name, bool dofail)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, family_name,
               strnlen(family_name, GENL_NAMSIZ - 1) + 1);
  int n = 0;
  int err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n, dofail);
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
    errno = EINVAL;
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  return id;
}

const int kInitNetNsFd = 239;

static long syz_init_net_socket(volatile long domain, volatile long type,
                                volatile long proto)
{
  return syscall(__NR_socket, domain, type, proto);
}

static long syz_genetlink_get_family_id(volatile long name,
                                        volatile long sock_arg)
{
  bool dofail = false;
  int fd = sock_arg;
  if (fd < 0) {
    dofail = true;
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd == -1) {
      return -1;
    }
  }
  struct nlmsg nlmsg_tmp;
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name, dofail);
  if ((int)sock_arg < 0)
    close(fd);
  if (ret < 0) {
    return -1;
  }
  return ret;
}

uint64_t r[4] = {0x0, 0xffffffffffffffff, 0xffffffffffffffff, 0x0};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000080, "nl802154\000", 9);
  res = -1;
  res = syz_genetlink_get_family_id(0x20000080, -1);
  if (res != -1)
    r[0] = res;
  res = -1;
  res = syz_init_net_socket(0x10, 3, 0x10);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000440 = 0;
  *(uint32_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x20000400;
  *(uint64_t*)0x20000400 = 0x20000380;
  *(uint32_t*)0x20000380 = 0x44;
  *(uint16_t*)0x20000384 = r[0];
  *(uint16_t*)0x20000386 = 0x72a7;
  *(uint32_t*)0x20000388 = 0;
  *(uint32_t*)0x2000038c = 0;
  *(uint8_t*)0x20000390 = 7;
  *(uint8_t*)0x20000391 = 0;
  *(uint16_t*)0x20000392 = 0;
  *(uint16_t*)0x20000394 = 0xa;
  *(uint16_t*)0x20000396 = 4;
  memcpy((void*)0x20000398, "wpan4\000", 6);
  *(uint16_t*)0x200003a0 = 0xc;
  *(uint16_t*)0x200003a2 = 6;
  *(uint64_t*)0x200003a4 = 0x300000003;
  *(uint16_t*)0x200003ac = 8;
  *(uint16_t*)0x200003ae = 5;
  *(uint32_t*)0x200003b0 = 0x316b08a2;
  *(uint16_t*)0x200003b4 = 8;
  *(uint16_t*)0x200003b6 = 5;
  *(uint32_t*)0x200003b8 = 1;
  *(uint16_t*)0x200003bc = 8;
  *(uint16_t*)0x200003be = 1;
  *(uint32_t*)0x200003c0 = 0;
  *(uint64_t*)0x20000408 = 0x44;
  *(uint64_t*)0x20000458 = 1;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0;
  *(uint32_t*)0x20000470 = 0xc044;
  syscall(__NR_sendmsg, r[1], 0x20000440ul, 0x4104ul);
  res = -1;
  res = syz_init_net_socket(0x10, 3, 0x10);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x20000080, "nl802154\000", 9);
  res = -1;
  res = syz_genetlink_get_family_id(0x20000080, -1);
  if (res != -1)
    r[3] = res;
  *(uint64_t*)0x200002c0 = 0;
  *(uint32_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0x20000280;
  *(uint64_t*)0x20000280 = 0x20000200;
  *(uint32_t*)0x20000200 = 0x68;
  *(uint16_t*)0x20000204 = r[3];
  *(uint16_t*)0x20000206 = 1;
  *(uint32_t*)0x20000208 = 0x70bd2c;
  *(uint32_t*)0x2000020c = 0x25dfdbfb;
  *(uint8_t*)0x20000210 = 0x1b;
  *(uint8_t*)0x20000211 = 0;
  *(uint16_t*)0x20000212 = 0;
  *(uint16_t*)0x20000214 = 8;
  *(uint16_t*)0x20000216 = 3;
  *(uint32_t*)0x20000218 = 0;
  *(uint16_t*)0x2000021c = 0xc;
  *(uint16_t*)0x2000021e = 6;
  *(uint64_t*)0x20000220 = 0;
  *(uint16_t*)0x20000228 = 8;
  *(uint16_t*)0x2000022a = 3;
  *(uint32_t*)0x2000022c = 0;
  *(uint16_t*)0x20000230 = 8;
  *(uint16_t*)0x20000232 = 3;
  *(uint32_t*)0x20000234 = 0;
  *(uint16_t*)0x20000238 = 8;
  *(uint16_t*)0x2000023a = 3;
  *(uint32_t*)0x2000023c = 0;
  *(uint16_t*)0x20000240 = 0x10;
  STORE_BY_BITMASK(uint16_t, , 0x20000242, 0x23, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000243, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000243, 1, 7, 1);
  *(uint16_t*)0x20000244 = 0xc;
  *(uint16_t*)0x20000246 = 4;
  *(uint64_t*)0x20000248 = 0xaaaaaaaaaaaa0202;
  *(uint16_t*)0x20000250 = 0xc;
  *(uint16_t*)0x20000252 = 6;
  *(uint64_t*)0x20000254 = 0x200000002;
  *(uint16_t*)0x2000025c = 0xc;
  *(uint16_t*)0x2000025e = 6;
  *(uint64_t*)0x20000260 = 2;
  *(uint64_t*)0x20000288 = 0x68;
  *(uint64_t*)0x200002d8 = 1;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0;
  *(uint32_t*)0x200002f0 = 0xc000;
  syscall(__NR_sendmsg, r[2], 0x200002c0ul, 0x40000ul);
  return 0;
}
