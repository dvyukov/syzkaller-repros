// general protection fault in bpf_skb_cgroup_id
// https://syzkaller.appspot.com/bug?id=33f36d0754d4c5c0e102
// status:6
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
  *(uint32_t*)0x202a0fb8 = 3;
  *(uint32_t*)0x202a0fbc = 4;
  *(uint64_t*)0x202a0fc0 = 0x200004c0;
  memcpy(
      (void*)0x200004c0,
      "\x85\x00\x00\x00\x4f\x00\x00\x00\x35\x00\x00\x00\x00\x00\x00\x00\x94\x00"
      "\x00\x00\x05\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00\xf4\x67\x08\x80"
      "\x27\x1e\x35\x03\x20\x0f\xfa\x95\xa2\xc8\xc0\x37\xc5\xa1\x42\xdf\xa8\xd7"
      "\x62\x87\x01\x00\x51\x97\xfa\xbd\x5f\x70\x10\xe8\x1a\xe0\xb7\x37\x12\x6e"
      "\xa6\xf7\xdc\x39\xcd\x34\xd5\xae\xed\x8d\x38\xe6\x5c\xb6\xe2\x2f\xf5\xdb"
      "\xe5\x47\x04\xd2\x5c\x79\x94\x9c\x23\xe2\xeb\x15\xd7\x55\xa2\x35\x0e\xa7"
      "\xc0\x9c\xc5\x8d\xe1\x94\xf4\x48\x42\xa5\xf0\xa8\x32\x0e\x13\x82\x2c\x87"
      "\x07\xf8\x61\x2c\x10\xb1\x00\x00\x00\x00\xb0\xd3\x71\x2c\x7e\x93\x36\x3a"
      "\xf3\xc0\x75\xff\x1e\x23\x5b\x6a\x32\xd9\x54\x33\xbb\x75\x5a\xd5\xd5\x76"
      "\x09\x36\x6c\x63\x86\xd5\xec\x72\x09\xd0\x31\xf4\x0f\x2f\x78\x46\xc7\x44"
      "\xae\x6a\xf3\xc0\x97\xf5\xfe\x47\xfe\x5f\x17\xfd\xab\x80\x0f\x41\x04\xdb"
      "\xab\xa4\x6a\xac\x3a\x81\x5b\x1e\x5c\x6d\x1d\x22\x4b\x64\xbe\x6c\x4d\x7f"
      "\x47\xef\x21\xeb\x7e\x46\xf9\x5a\xba\x0b\x7e\xb6\xba\xaa\x4a\x97\x79\xf8"
      "\x55\x5e\xc6\xa7\x68\xc1\xf2\xc2\x21\xc1\x10\xef\x05\x00\x00\x00\x0e\xe2"
      "\x67\xab\x76\xf5\xa8\xd9\x28\xcf\x95\x84\x6b\xe6\x27\x7c\x04\xb8\xc5\x32"
      "\x48\x12\x69\x6a\x62\x3c\xd8\xa4\xf8\xdc\x8d\xcb\xa0\x0b\x1b\x2d\x05\x47"
      "\xc4\x5b\x0c\x52\x08\x7b\x5e\xfa\xbf\xdf\x8a\x9b\x97\xb9\xa9\x51\x66\x7d"
      "\xd5\x8e\xa0\x32\x7b\x56\xd4\xeb\xfb\x19\xa3\x42\x68\x33\x56\x48\x2a\x5e"
      "\xdc\xa5\x2f\xb9\x8c\x14\x52\xb6\x51\xeb\xf9\x42\x78\x8c\xeb\xd6\x87\x90"
      "\xb9\x8a\x6e\xf7\x29\x7f\x7b\x27\x44\x41\x6d\xaf\xc6\xe9\x55\x00\xf5\x3e"
      "\x53\x09\xec\x91\xd8\x3c\xf4\xfb\xc7\x75\xd9\xc0\x7d\x8d\x59\x1a\x4d\xac"
      "\x60\xff\x00\xe6\x29\xb3\xb2\x00\x00\x00\x00\x00\x00\x00\x00\xd2\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7b\x87\x19\x4f\x00\x1a"
      "\xc3\x9d\x85\xb2\xe0\x40\x9c\x30\xb0\xe7\x21\xe3\x2a\x73\x82\xae\x38\x5e"
      "\x36\xa1\xb5\x1e\x42\xf1\x2a\x88\x15\x5d\x9e\x06\xdd\xb4\x4b\x06\xb7\x82"
      "\x0e\x11\x73\x8e\x2b\xba\x8c\xcc\xc9\xc0\xa1\x7e\x3b\xe1\xa6\x50\xf2\x57"
      "\x54\x66\x3a\xe2\x69\xc7\xa5\xfe\x8f\x57\x44\x4c\x10\xa5\x00\x00\x00\x00"
      "\xfa\x7a\xb6\xcd\xd8\xe9\x2c\x0b\xaa\xdf\xd1\x6d\x1a\xb8\x3d\xc3\x3b\x23"
      "\x53\x60\xb4\xbe\x84\x92\xce\x99\xd3\xa0\x98\xfd\x9b\x57\x36\x92\x4f\xf2"
      "\xc6\xda\xc6\x2a\xb8\x32\xea\x58\x26\xc6\x21\x81\x42\x1a\xf1\x46\xb7\x2e"
      "\x06\xaf\xa6\xa2\x1b\xbc\x7d\x46\x13\x44\x9d\x64\xef\x73\x37\x08\x58\xd7"
      "\x13\x23\x6e\x19\x85\xcd\x7f\xb9\x10\x9b\x48\x74\xab\x0f\xfa\x53\x36\xf1"
      "\x78\x40\x48\x68\xca\x03\x9e\x53\xdd\xb7\x37\xc8\x66\xad\x87\x2c\x37\xa5"
      "\x5d\xd7\x54\x42\xce\xbb\x86\xdc\xfe\xeb\xbb\x31\x49\xc5\x04\xaa\x18\x44"
      "\x54\xaf\x30\x02\x12\x7e\x9a\x01\xae\xc6\x93\x9c\x02\x82\x62\x0f\x83\x87"
      "\xe1\xd0\x1b\x23\x13\x66\xd7\xd7\x4a\xcb\xad\xd6\x25\x63\x92\xe2\x06\x3c"
      "\x36\x29\xc9\x0a\x73\xe2\x05\x36\xc3\xb7\xdb\xa9\x43\x3c\x2e\xaf\x8e\x47"
      "\xff\xff\x3d\xfe\xa5\xda\x8a\xa4\xb9\xae\x2d\xe5\x4c\xbe\xf1\x6f\x16\xfd"
      "\x5d\xc5\x21\x00\x00\x00\x00\x00\x00\x00\x59\xaa\x49\x03\x57\xcc\xd5\x75"
      "\x94\x82\x34\xe3\x3c\xdd\xca\x9d\x3a\x1e\xe2\x82\x23\x4a\x07\x08\x8f\xb6"
      "\x55\x5a\x92\x8c\x81\x83\x4f\xcc\xcc\x6d\xcc\xe9\x3c\xd7\xd3\x99\x57\x29"
      "\x26\xbb\xe2\xce\x58\x97\x7b\xfd\x08\xf2\xb1\x2c\xd8\xd8\xa6\x89\x57\x1f"
      "\x4a\x90\xc3\x22\x2f\xf1\xf2\x2a\x88\xca\x4f\xd5\xcb\xec\x14\x91\x34\x1e"
      "\xac\xfe\xab\xb4\x4f\xb3\x0a\x0f\x22\x49\x20\xcb\x5e\x17\x58\x22\x15\x36"
      "\xd0\xa8\x77\xc6\x08\x56\x77\xf4\x5f\x44\x77\xfe\x26\x29\xdc\xfd\x63\x3d"
      "\x2b\x72\x36\x89\xd6\x0e\xc5\x97\x34\xd6\x43\xe4\x45\x71\xa6\xa5\xbc\x63"
      "\xb0\x7e\x52\x11\xe8\x76\x34\xec\x3d\x51\x7a\xc3\x5f\x20\x9d\xdb\x7e\x32"
      "\x3e\x62\xfd\xca\xe8\xc4\x56\x6f\xcd\x77\x54\x5c\xa8\x83\xed\x45\xf4\xde"
      "\x04\x51\x0f\xd4\x91\x47\x8e\xf1\x58\x94\x8b\xf7\xa3\x94\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x8d\x61\x95\xae\xcb\x54\x83\x50\x65\x88\xc2\xd3\x90\x73"
      "\xea\x66\x8f\xef\x1f\xad\xb2\x2c\xb6\xb6\x61\xe4\xfc\xf7\x97\xe3\xd1\x0b"
      "\xf0\x92\x93\xe8\x01\x3a\xee\x2c\x52\xd5\xeb\x6c\x52\xc8\x2a\x53\x70\x88"
      "\x8c\xdf\xa5\x97\x6f\xc5\x33\x75\xff\x07\x9b\x36\xe0\xf9\x57\x42",
      952);
  *(uint64_t*)0x202a0fc8 = 0x20000140;
  memcpy((void*)0x20000140, "GPL\000", 4);
  *(uint32_t*)0x202a0fd0 = 0;
  *(uint32_t*)0x202a0fd4 = 0xe0;
  *(uint64_t*)0x202a0fd8 = 0x20000180;
  *(uint32_t*)0x202a0fe0 = 0;
  *(uint32_t*)0x202a0fe4 = 0;
  memset((void*)0x202a0fe8, 0, 16);
  *(uint32_t*)0x202a0ff8 = 0;
  *(uint32_t*)0x202a0ffc = 0;
  *(uint32_t*)0x202a1000 = -1;
  *(uint32_t*)0x202a1004 = 8;
  *(uint64_t*)0x202a1008 = 0;
  *(uint32_t*)0x202a1010 = 0;
  *(uint32_t*)0x202a1014 = 0x10;
  *(uint64_t*)0x202a1018 = 0;
  *(uint32_t*)0x202a1020 = 0xfffffc7c;
  *(uint32_t*)0x202a1024 = 0;
  *(uint32_t*)0x202a1028 = -1;
  res = syscall(__NR_bpf, 5ul, 0x202a0fb8ul, 0x15ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000440 = r[0];
  *(uint32_t*)0x20000444 = 0xfffff000;
  *(uint32_t*)0x20000448 = 0xe;
  *(uint32_t*)0x2000044c = 0xf00001a0;
  *(uint64_t*)0x20000450 = 0x200000c0;
  memcpy((void*)0x200000c0,
         "\x61\xdf\x71\x2b\xc8\x84\xfe\xd5\x72\x27\x80\xb6\xc2\xa7", 14);
  *(uint64_t*)0x20000458 = 0;
  *(uint32_t*)0x20000460 = 0x8000;
  *(uint32_t*)0x20000464 = 0;
  *(uint32_t*)0x20000468 = 0;
  *(uint32_t*)0x2000046c = 0x9eb3be72;
  *(uint64_t*)0x20000470 = 0x20000000;
  *(uint64_t*)0x20000478 = 0x20000000;
  *(uint32_t*)0x20000480 = 0;
  *(uint32_t*)0x20000484 = 0;
  syscall(__NR_bpf, 0xaul, 0x20000440ul, 0x28ul);
  return 0;
}