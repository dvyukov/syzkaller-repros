// general protection fault in xfrm_state_walk_done
// https://syzkaller.appspot.com/bug?id=7a7e5fbdfd2c69fc673e8e4c5847f1e6004ec3bb
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
  res = syscall(__NR_socket, 0x10, 3, 6);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200007c0 = 0x20000000;
  *(uint16_t*)0x20000000 = 0x10;
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x200007c8 = 0xc;
  *(uint64_t*)0x200007d0 = 0x20000780;
  *(uint64_t*)0x20000780 = 0x20006980;
  *(uint32_t*)0x20006980 = 0x3c;
  *(uint16_t*)0x20006984 = 0x12;
  *(uint16_t*)0x20006986 = 0x301;
  *(uint32_t*)0x20006988 = 0x100;
  *(uint32_t*)0x2000698c = 0;
  *(uint16_t*)0x20006990 = 8;
  *(uint16_t*)0x20006992 = 0xa;
  *(uint32_t*)0x20006994 = r[0];
  *(uint16_t*)0x20006998 = 0xc;
  *(uint16_t*)0x2000699a = 0x70;
  *(uint16_t*)0x2000699c = 8;
  *(uint16_t*)0x2000699e = 0x77;
  *(uint32_t*)0x200069a0 = 0;
  *(uint16_t*)0x200069a4 = 0x10;
  *(uint16_t*)0x200069a6 = 0x20;
  *(uint16_t*)0x200069a8 = 4;
  *(uint16_t*)0x200069aa = 0x3b;
  *(uint16_t*)0x200069ac = 8;
  *(uint16_t*)0x200069ae = 0x1b;
  *(uint32_t*)0x200069b0 = htobe32(2);
  *(uint16_t*)0x200069b4 = 8;
  *(uint16_t*)0x200069b6 = 8;
  *(uint32_t*)0x200069b8 = htobe32(0x7f000001);
  *(uint64_t*)0x20000788 = 0x3c;
  *(uint64_t*)0x200007d8 = 1;
  *(uint64_t*)0x200007e0 = 0;
  *(uint64_t*)0x200007e8 = 0;
  *(uint32_t*)0x200007f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200007c0, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}