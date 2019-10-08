// possible deadlock in rtnl_lock
// https://syzkaller.appspot.com/bug?id=038c47c353490e96ed3592dfd723466c8d2f0648
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res;
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = 4;
  *(uint16_t*)0x20000008 = 0xa;
  *(uint16_t*)0x2000000a = htobe16(0x4e24);
  *(uint32_t*)0x2000000c = 1;
  *(uint8_t*)0x20000010 = -1;
  *(uint8_t*)0x20000011 = 2;
  *(uint8_t*)0x20000012 = 0;
  *(uint8_t*)0x20000013 = 0;
  *(uint8_t*)0x20000014 = 0;
  *(uint8_t*)0x20000015 = 0;
  *(uint8_t*)0x20000016 = 0;
  *(uint8_t*)0x20000017 = 0;
  *(uint8_t*)0x20000018 = 0;
  *(uint8_t*)0x20000019 = 0;
  *(uint8_t*)0x2000001a = 0;
  *(uint8_t*)0x2000001b = 0;
  *(uint8_t*)0x2000001c = 0;
  *(uint8_t*)0x2000001d = 0;
  *(uint8_t*)0x2000001e = 0;
  *(uint8_t*)0x2000001f = 1;
  *(uint32_t*)0x20000020 = 8;
  *(uint64_t*)0x20000028 = 0;
  *(uint64_t*)0x20000030 = 0;
  *(uint64_t*)0x20000038 = 0;
  *(uint64_t*)0x20000040 = 0;
  *(uint64_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint64_t*)0x20000070 = 0;
  *(uint64_t*)0x20000078 = 0;
  *(uint64_t*)0x20000080 = 0;
  *(uint16_t*)0x20000088 = 0xa;
  *(uint16_t*)0x2000008a = htobe16(0x4e23);
  *(uint32_t*)0x2000008c = 0x401;
  *(uint8_t*)0x20000090 = 0;
  *(uint8_t*)0x20000091 = 0;
  *(uint8_t*)0x20000092 = 0;
  *(uint8_t*)0x20000093 = 0;
  *(uint8_t*)0x20000094 = 0;
  *(uint8_t*)0x20000095 = 0;
  *(uint8_t*)0x20000096 = 0;
  *(uint8_t*)0x20000097 = 0;
  *(uint8_t*)0x20000098 = 0;
  *(uint8_t*)0x20000099 = 0;
  *(uint8_t*)0x2000009a = 0;
  *(uint8_t*)0x2000009b = 0;
  *(uint8_t*)0x2000009c = 0;
  *(uint8_t*)0x2000009d = 0;
  *(uint8_t*)0x2000009e = 0;
  *(uint8_t*)0x2000009f = 0;
  *(uint32_t*)0x200000a0 = 0x101;
  *(uint64_t*)0x200000a8 = 0;
  *(uint64_t*)0x200000b0 = 0;
  *(uint64_t*)0x200000b8 = 0;
  *(uint64_t*)0x200000c0 = 0;
  *(uint64_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0;
  *(uint64_t*)0x200000d8 = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint64_t*)0x200000f0 = 0;
  *(uint64_t*)0x200000f8 = 0;
  *(uint64_t*)0x20000100 = 0;
  syscall(__NR_setsockopt, r[0], 0x29, 0x2e, 0x20000000, 0x108);
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x201fefe4 = 0xa;
  *(uint16_t*)0x201fefe6 = htobe16(0x4e22);
  *(uint32_t*)0x201fefe8 = 0;
  *(uint8_t*)0x201fefec = 0;
  *(uint8_t*)0x201fefed = 0;
  *(uint8_t*)0x201fefee = 0;
  *(uint8_t*)0x201fefef = 0;
  *(uint8_t*)0x201feff0 = 0;
  *(uint8_t*)0x201feff1 = 0;
  *(uint8_t*)0x201feff2 = 0;
  *(uint8_t*)0x201feff3 = 0;
  *(uint8_t*)0x201feff4 = 0;
  *(uint8_t*)0x201feff5 = 0;
  *(uint8_t*)0x201feff6 = 0;
  *(uint8_t*)0x201feff7 = 0;
  *(uint8_t*)0x201feff8 = 0;
  *(uint8_t*)0x201feff9 = 0;
  *(uint8_t*)0x201feffa = 0;
  *(uint8_t*)0x201feffb = 0;
  *(uint32_t*)0x201feffc = 0;
  syscall(__NR_bind, r[1], 0x201fefe4, 0x1c);
  syscall(__NR_listen, r[1], 0);
  *(uint16_t*)0x201cd000 = 0xa;
  *(uint16_t*)0x201cd002 = htobe16(0x4e22);
  *(uint32_t*)0x201cd004 = 0;
  *(uint8_t*)0x201cd008 = 0;
  *(uint8_t*)0x201cd009 = 0;
  *(uint8_t*)0x201cd00a = 0;
  *(uint8_t*)0x201cd00b = 0;
  *(uint8_t*)0x201cd00c = 0;
  *(uint8_t*)0x201cd00d = 0;
  *(uint8_t*)0x201cd00e = 0;
  *(uint8_t*)0x201cd00f = 0;
  *(uint8_t*)0x201cd010 = 0;
  *(uint8_t*)0x201cd011 = 0;
  *(uint8_t*)0x201cd012 = -1;
  *(uint8_t*)0x201cd013 = -1;
  *(uint32_t*)0x201cd014 = htobe32(0);
  *(uint32_t*)0x201cd018 = 0;
  syscall(__NR_connect, r[0], 0x201cd000, 0x1c);
  *(uint32_t*)0x200fbffc = 2;
  syscall(__NR_setsockopt, r[0], 0x29, 1, 0x200fbffc, 4);
  *(uint8_t*)0x20002f54 = -1;
  *(uint8_t*)0x20002f55 = -1;
  *(uint8_t*)0x20002f56 = -1;
  *(uint8_t*)0x20002f57 = -1;
  *(uint8_t*)0x20002f58 = -1;
  *(uint8_t*)0x20002f59 = -1;
  *(uint8_t*)0x20002f5a = 0xaa;
  *(uint8_t*)0x20002f5b = 0xaa;
  *(uint8_t*)0x20002f5c = 0xaa;
  *(uint8_t*)0x20002f5d = 0xaa;
  *(uint8_t*)0x20002f5e = 0;
  *(uint8_t*)0x20002f5f = 0;
  *(uint16_t*)0x20002f60 = htobe16(0x8847);
  *(uint8_t*)0x20002f62 = 0;
  *(uint8_t*)0x20002f63 = 0;
  memcpy((void*)0x20002f64, "u", 1);
  memcpy((void*)0x20002f65, "\xe1\x11\x84", 3);
  *(uint16_t*)0x20002f68 = htobe16(0);
  *(uint32_t*)0x20003000 = 0;
  *(uint32_t*)0x20003004 = 1;
  *(uint32_t*)0x20003008 = 0;
  *(uint32_t*)0x2000300c = 0;
  *(uint32_t*)0x20003010 = 0;
  *(uint32_t*)0x20003014 = 0;
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}