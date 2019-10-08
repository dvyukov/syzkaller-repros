// KMSAN: uninit-value in __skb_flow_dissect
// https://syzkaller.appspot.com/bug?id=47f2abc6c0b5739506ffebf29155eb5181f27a4f
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

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

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x11, 0x800000000002, 0x300);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x2095bffc = 0;
  *(uint8_t*)0x2095bffe = 0;
  *(uint8_t*)0x2095bfff = 0;
  syscall(__NR_setsockopt, r[0], 0x107, 0x12, 0x2095bffc, 4);
  res = syscall(__NR_socket, 2, 3, 0x2f);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x205b5ff0 = 2;
  *(uint16_t*)0x205b5ff2 = htobe16(0);
  *(uint32_t*)0x205b5ff4 = htobe32(0);
  *(uint8_t*)0x205b5ff8 = 0;
  *(uint8_t*)0x205b5ff9 = 0;
  *(uint8_t*)0x205b5ffa = 0;
  *(uint8_t*)0x205b5ffb = 0;
  *(uint8_t*)0x205b5ffc = 0;
  *(uint8_t*)0x205b5ffd = 0;
  *(uint8_t*)0x205b5ffe = 0;
  *(uint8_t*)0x205b5fff = 0;
  syscall(__NR_sendto, r[1], 0x2014cf2c, 0, 0x8000, 0x205b5ff0, 0x10);
  memcpy((void*)0x20000040, "\x33\xf0\x18\xeb", 4);
  *(uint16_t*)0x20000140 = 2;
  *(uint16_t*)0x20000142 = htobe16(0);
  *(uint32_t*)0x20000144 = htobe32(0x7f000001);
  *(uint8_t*)0x20000148 = 0;
  *(uint8_t*)0x20000149 = 0;
  *(uint8_t*)0x2000014a = 0;
  *(uint8_t*)0x2000014b = 0;
  *(uint8_t*)0x2000014c = 0;
  *(uint8_t*)0x2000014d = 0;
  *(uint8_t*)0x2000014e = 0;
  *(uint8_t*)0x2000014f = 0;
  syscall(__NR_sendto, r[1], 0x20000040, 4, 0, 0x20000140, 0x10);
  *(uint8_t*)0x20000000 = 1;
  *(uint8_t*)0x20000001 = 0x80;
  *(uint8_t*)0x20000002 = 0xc2;
  *(uint8_t*)0x20000003 = 0;
  *(uint8_t*)0x20000004 = 0;
  *(uint8_t*)0x20000005 = 0;
  *(uint8_t*)0x20000006 = 0xaa;
  *(uint8_t*)0x20000007 = 0xaa;
  *(uint8_t*)0x20000008 = 0xaa;
  *(uint8_t*)0x20000009 = 0xaa;
  *(uint8_t*)0x2000000a = 0;
  *(uint8_t*)0x2000000b = 0;
  *(uint16_t*)0x2000000c = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x2000000e, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000000e, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000000f, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2000000f, 0, 2, 6);
  *(uint16_t*)0x20000010 = htobe16(0x1c);
  *(uint16_t*)0x20000012 = htobe16(0);
  *(uint16_t*)0x20000014 = htobe16(0);
  *(uint8_t*)0x20000016 = 0;
  *(uint8_t*)0x20000017 = 0;
  *(uint16_t*)0x20000018 = 0;
  *(uint32_t*)0x2000001a = htobe32(0);
  *(uint32_t*)0x2000001e = htobe32(0xe0000001);
  *(uint8_t*)0x20000022 = 0;
  *(uint8_t*)0x20000023 = 0;
  *(uint16_t*)0x20000024 = 0;
  *(uint32_t*)0x20000026 = htobe32(0);
  *(uint32_t*)0x203b5000 = 0;
  *(uint32_t*)0x203b5004 = 0;
  *(uint32_t*)0x203b5008 = 0;
  *(uint32_t*)0x203b500c = 0;
  *(uint32_t*)0x203b5010 = 0;
  *(uint32_t*)0x203b5014 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20000022, 8);
  *(uint16_t*)0x20000024 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x2000000e, 20);
  *(uint16_t*)0x20000018 = csum_inet_digest(&csum_2);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}