// BUG: unable to handle kernel paging request in dummy_set_vf_rss_query_en
// https://syzkaller.appspot.com/bug?id=bf734d725242b5546368965cad16d952c9d6bd89
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000100, "dummy0\000\000\000\000\000\000\000\000\000\000",
         16);
  *(uint32_t*)0x20000110 = 0;
  res = syscall(__NR_ioctl, r[0], 0x8933, 0x20000100);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000110;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x200000c0;
  *(uint64_t*)0x200000c0 = 0x20000000;
  memcpy((void*)0x20000000, "\x34\x00\x00\x00\x13\x00\x01\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000014 = r[1];
  memcpy((void*)0x20000018,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x14\x00\x16\x00\x10\x00\x01\x00\x0c"
         "\x00\x07\x00\x08\x00\x0f\xd3\xc9\x54\xd7\x60\xff\xb3\x9a\xc9\x69\x62"
         "\x36\x75\x4e\xaa\x5c\x94\xda\x07\x9b\x74\x9a\xde\x61\x24\xde\xb5\xd6"
         "\x14\xad\x2c\xdf\xc5\x87\x38\xf5\x92\x04\xd6\x53\x73\xdf\x8c\xca\x05"
         "\x10\x16\x7f\xd2\xbf\xa9\x1e\xf8\x35\x13\xb1\x62\x64\x27\xf0\xba\x80"
         "\xe5\xae",
         87);
  *(uint64_t*)0x200000c8 = 0x34;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000240, 0);
  return 0;
}