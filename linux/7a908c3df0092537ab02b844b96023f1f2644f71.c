// KASAN: stack-out-of-bounds Read in xfrm_selector_match
// https://syzkaller.appspot.com/bug?id=7a908c3df0092537ab02b844b96023f1f2644f71
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200011c0 = htobe32(0);
  *(uint64_t*)0x200011d0 = htobe64(0);
  *(uint64_t*)0x200011d8 = htobe64(1);
  *(uint16_t*)0x200011e0 = htobe16(0);
  *(uint16_t*)0x200011e2 = htobe16(0);
  *(uint16_t*)0x200011e4 = htobe16(0);
  *(uint16_t*)0x200011e6 = htobe16(0);
  *(uint16_t*)0x200011e8 = 0xa;
  *(uint8_t*)0x200011ea = 0;
  *(uint8_t*)0x200011eb = -1;
  *(uint8_t*)0x200011ec = 0;
  *(uint32_t*)0x200011f0 = 0;
  *(uint32_t*)0x200011f4 = 0;
  *(uint64_t*)0x200011f8 = 0;
  *(uint64_t*)0x20001200 = 0;
  *(uint64_t*)0x20001208 = 0;
  *(uint64_t*)0x20001210 = 0;
  *(uint64_t*)0x20001218 = 0;
  *(uint64_t*)0x20001220 = 0;
  *(uint64_t*)0x20001228 = 0;
  *(uint64_t*)0x20001230 = 0;
  *(uint64_t*)0x20001238 = 0;
  *(uint64_t*)0x20001240 = 0;
  *(uint64_t*)0x20001248 = 0;
  *(uint64_t*)0x20001250 = 0x40000000000;
  *(uint32_t*)0x20001258 = 0;
  *(uint32_t*)0x2000125c = 0;
  *(uint8_t*)0x20001260 = 1;
  *(uint8_t*)0x20001261 = 1;
  *(uint8_t*)0x20001262 = 0xfe;
  *(uint8_t*)0x20001263 = 0;
  *(uint32_t*)0x20001268 = htobe32(0x7f000001);
  *(uint32_t*)0x20001278 = htobe32(0);
  *(uint8_t*)0x2000127c = -1;
  *(uint16_t*)0x20001280 = 0;
  *(uint8_t*)0x20001284 = 0xac;
  *(uint8_t*)0x20001285 = 0x14;
  *(uint8_t*)0x20001286 = 0x14;
  *(uint8_t*)0x20001287 = 0;
  *(uint32_t*)0x20001294 = 0;
  *(uint8_t*)0x20001298 = 0;
  *(uint8_t*)0x20001299 = 0;
  *(uint8_t*)0x2000129a = 0;
  *(uint32_t*)0x2000129c = 0;
  *(uint32_t*)0x200012a0 = 0;
  *(uint32_t*)0x200012a4 = 0;
  syscall(__NR_setsockopt, r[0], 0x29, 0x23, 0x200011c0, 0xe8);
  *(uint16_t*)0x20000200 = 0xa;
  *(uint16_t*)0x20000202 = htobe16(0);
  *(uint32_t*)0x20000204 = 0;
  *(uint64_t*)0x20000208 = htobe64(0);
  *(uint64_t*)0x20000210 = htobe64(1);
  *(uint32_t*)0x20000218 = 0;
  syscall(__NR_sendto, r[0], 0x20000140, 0, 0x20000005, 0x20000200, 0x1c);
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint32_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d4 = 0;
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}