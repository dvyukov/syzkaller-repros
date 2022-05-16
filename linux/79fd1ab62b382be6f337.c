// WARNING in bpf_prog_test_run_xdp
// https://syzkaller.appspot.com/bug?id=79fd1ab62b382be6f337
// status:0
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint32_t*)0x20000280 = 6;
  *(uint32_t*)0x20000284 = 4;
  *(uint64_t*)0x20000288 = 0x20001880;
  memcpy(
      (void*)0x20001880,
      "\x18\x02\x00\x00\xe2\x22\x00\xff\x00\x00\x00\x00\x00\x00\x00\x00\x85\x00"
      "\x00\x00\x36\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00\x92\x2a\xe8\x37"
      "\x13\xab\x96\x62\xce\x3a\xe3\x56\x53\x8d\xda\x12\x00\x00\x00\xda\xa9\xdd"
      "\x1b\x10\xfb\x54\xa8\xcb\x72\xd2\x8d\x82\xde\x5a\xc5\x4e\x32\xad\x55\x8c"
      "\x46\xff\xf4\x20\x8d\x49\x63\x19\x79\xa4\x2d\x68\x84\xec\x11\xcd\x14\x13"
      "\x8b\x8f\xe9\x03\xdd\xc7\x02\x03\x00\x00\x00\x51\x0c\x88\x12\xbe\xf8\x83"
      "\xd7\x69\x67\x65\x19\x0d\x8a\x26\x33\x45\xe4\x4d\x51\x87\xb3\xc4\xd8\xfa"
      "\xd9\xf2\xcf\x8a\xff\x66\xd6\xb3\x18\x1f\xfc\x1d\x62\xb3\x95\x4c\x11\xc2"
      "\x78\x39\xdc\xf4\x7b\x4d\x29\x6e\x73\x59\xea\x7b\xa7\x5d\xd0\x98\xbb\xc4"
      "\xfa\x13\xae\xe4\x8c\xa9\xe8\x96\x9f\xae\xbf\x31\x83\xfe\x80\x38\xbb\xf5"
      "\x02\x4b\x52\xdc\x26\x5b\x36\xfc\x9d\xae\x00\xa0\xd0\x95\x6d\x25\x2b\xd8"
      "\xb6\x46\x4e\xac\x4e\xa0\x4b\xa0\x6b\x49\x26\x5f\xd6\xde\xf8\xb8\xd3\xca"
      "\x6e\x3a\xbd\xb2\xdf\xc6\x16\xa1\xe3\x40\xbb\x8e\x2a\x2e\x3a\xdc\x57\x19"
      "\x6b\x40\xde\xf3\x85\x8e\xf5\x69\x14\x7f\xa4\x10\x83\x28\x39\x2d\x32\x2a"
      "\xb4\xdf\x10\xa2\xf6\xe4\x7d\x42\x83\xe8\x59\xa4\xf9\x0e\x42\x41\x0f\x57"
      "\x46\x6f\x59\xcb\xef\xae\xa2\x54\x40\x47\xd6\xd8\xac\x1d\xfe\xa5\x06\xf2"
      "\xce\xf8\xaa\x17\xc7\x29\x30\x0d\x23\x1a\x9f\x04\xc2\x91\x9e\x60\xdf\xa8"
      "\xa0\xb3\x67\x54\xed\x52\x90\xa8\xcd\x84\x70\xe7\x76\xd6\xb8\x06\xb5\x58"
      "\xc5\xe7\xf4\x09\x45\x78\x71\xb4\xa4\xa4\xc9\x64\xa1\x3a\xac\xea\xa2\xe6"
      "\x8d\x25\x1e\x71\x48\x30\xd6\xcb\xde\x49\xb2\x9a\xfc\x65\x21\x06\x19\x46"
      "\x8e\x7b\x6c\xb5\xf4\xfc\x00\x00\x00\x00\x00\x00\x4b\x58\x8c\x74\x5c\x38"
      "\x0e\x5f\xe5\x72\x38\x49\x00\xc8\x00\x09\xe4\x12\x6c\xc1\x7c\xe7\x38\xe8"
      "\xf0\xc7\x5b\x62\x7a\x00\x00\xde\x45\xcb\xc1\x13\x9f\x91\x0e\x68\xae\xe4"
      "\x43\x86\x17\x1e\xaf\xe8\xa4\x45\xad\xda\x85\xe1\xfb\xf8\x7d\x3b\xc9\x32"
      "\x29\x95\x4b\x48\x3e\x86\x24\x3d\x6b\xc1\x27\x09\xe2\x49\xcb\xef\x19\x52"
      "\x7a\x9c\x5a\xa1\x4c\x3d\x05\xb9\x17\xb4\xf8\x24\xb0\x2b\x99\xfb\xd4\x83"
      "\x0e\x2b\xa8\xc1\x15\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x06\x00\x00\x00\x00\x00\x00\x49\x9e\xbf\x95\x1b\xf6\xea\x71\xcb"
      "\xd3\xf3\x81\x77\xd5\x2d\x13\x2a\xe1\x2f\x04\x42\xb9\x7a\x81\xcb\x19\x0c"
      "\xe2\x55\xb0\xd4\xfb\x21\xe8\x34\x45\x7f\x71\x2d\x77\xc6\xd6\x81\x12\xc1"
      "\xd2\xb2\x74\xbc\x0b\x8e\x64\xed\x1b\xf7\x0d\x77\x15\xef\x8c\xfc\x6e\x8f"
      "\xcb\x02\x7a\xbe\x02\x5b\xf0\xae\xa9\x06\xdf\xfd\x65\x88\xc6\x9b\x31\x9c"
      "\x79\xb6\xc8\xe8\x30\x67\xa3\xdc\x96\xe6\xfe\xef\x8b\xa2\xed\x85\x1a\xd2"
      "\x49\x3d\x38\xb1\x54\x8d\x8a\x7f\xad\xf4\x8d\xd2\x00\xf4\xd0\x36\x5a\xe9"
      "\x09\xdb\xcd\x72\x9b\x5c\x90\xcc\xa3\x9e\xde\xa1\xb0\x85\xb6\x97\x0c\x0b"
      "\x30\xc6\xc0\x4f\x97\xdf\x1d\xf5\x9f\x7f\x83\x0e\xb6\x6e\x6f\xc5\x97\x13"
      "\xb4\xce\xfa\xb5\x33\xa2\x2a\x45\xe4\xc2\x7a\x9f\x15\xcb\x16\xcb\x2c\x73"
      "\x64\x78\xd0\xa1\x17\x42\x7d\x2d\xd0\x50\x15\x89\x4d\x14\x9a\x2d\x53\x1b"
      "\xa5\x8f\x32\x13\x35\x6c\x2c\x6d\xea\xe8\x67\xa7\xdc\x41\x3a\x61\x83\x27"
      "\x5f\x2c\x63\x4b\x7c\x3f\x59\xa4\x94\xf0\x6c\xf3\x30\x3a\x79\x45\xe9\x46"
      "\xd1\xac\xa0\x46\xed\xb3\x21\xdb\xc7\x5d\x35\x4f\x16\x2c\x6c\x10\x4a\x05"
      "\x47\xe2\xfa\xe7\x65\x11\x91\xf3\x81\x26\x75\x70\xc8\x31\x2d\x32\xce\x99"
      "\x44\xa0\xe6\xcb\xb4\x6a\xa4\x7f\x26\x91\x55\x55\xaf\x73\xeb\x65\x0c\xe9"
      "\xd4\xd3\x5e\xde\xf3\x56\xf9\xa4\x48\x71\x53\xd4\xd9\x83\xef\x24\x01\x74"
      "\x51\x64\x6d\xcc\xa6\xce\xc5\x5b\x54\x01\x7c\xab\x2e\x91",
      806);
  *(uint64_t*)0x20000290 = 0x20000040;
  memcpy((void*)0x20000040, "GPL\000", 4);
  *(uint32_t*)0x20000298 = 4;
  *(uint32_t*)0x2000029c = 0x1076;
  *(uint64_t*)0x200002a0 = 0x20000300;
  *(uint32_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002ac = 0;
  memset((void*)0x200002b0, 0, 16);
  *(uint32_t*)0x200002c0 = 0;
  *(uint32_t*)0x200002c4 = 0;
  *(uint32_t*)0x200002c8 = -1;
  *(uint32_t*)0x200002cc = 8;
  *(uint64_t*)0x200002d0 = 0;
  *(uint32_t*)0x200002d8 = 0;
  *(uint32_t*)0x200002dc = 0x10;
  *(uint64_t*)0x200002e0 = 0;
  *(uint32_t*)0x200002e8 = 0;
  *(uint32_t*)0x200002ec = 0;
  *(uint32_t*)0x200002f0 = -1;
  *(uint32_t*)0x200002f4 = 0;
  *(uint64_t*)0x200002f8 = 0;
  res = syscall(__NR_bpf, 5ul, 0x20000280ul, 0x70ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200013c0 = r[0];
  *(uint32_t*)0x200013c4 = 0x1000000;
  *(uint32_t*)0x200013c8 = 0xfffffe9e;
  *(uint32_t*)0x200013cc = 0;
  *(uint64_t*)0x200013d0 = 0x20000000;
  memcpy((void*)0x20000000,
         "\x85\x5c\xe0\xda\xe4\x27\x95\xce\x89\x4d\xaf\xb1\x59\x69\x27\xe8\x3b"
         "\xb5\x82\xab\x3a\x3d\xbf\x97\x6f\x51\x7b\x38\xa3\xdd\xd7\x2b\x0a\xb3"
         "\x02\x87\x36\x50\x25\x9e\x22\x5d\x2a\x38\x55\x3a\xa3\x40\x93\x1f\x4d"
         "\xd5\x81\xe7\x63\x50\x53\x42",
         58);
  *(uint64_t*)0x200013d8 = 0;
  *(uint32_t*)0x200013e0 = 0;
  *(uint32_t*)0x200013e4 = 0;
  *(uint32_t*)0x200013e8 = 0;
  *(uint32_t*)0x200013ec = 0;
  *(uint64_t*)0x200013f0 = 0;
  *(uint64_t*)0x200013f8 = 0;
  *(uint32_t*)0x20001400 = 0;
  *(uint32_t*)0x20001404 = 0;
  syscall(__NR_bpf, 0xaul, 0x200013c0ul, 0x48ul);
  return 0;
}
