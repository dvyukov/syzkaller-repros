// general protection fault in fq_codel_enqueue (3)
// https://syzkaller.appspot.com/bug?id=7a12909485b94426aceb
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

uint64_t r[4] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x11;
  memcpy((void*)0x20000082,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, r[0], 0x20000080ul, 0x80ul);
  *(uint32_t*)0x20000000 = 0x14;
  res = syscall(__NR_getsockname, r[0], 0x200003c0ul, 0x20000000ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x200003c4;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x20000780;
  *(uint64_t*)0x20000780 = 0x20000140;
  *(uint32_t*)0x20000140 = 0x40;
  *(uint16_t*)0x20000144 = 0x24;
  *(uint16_t*)0x20000146 = 0xf0b;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = -1;
  *(uint8_t*)0x20000150 = 0;
  *(uint8_t*)0x20000151 = 0;
  *(uint16_t*)0x20000152 = 0;
  *(uint32_t*)0x20000154 = r[1];
  *(uint16_t*)0x20000158 = 0;
  *(uint16_t*)0x2000015a = 0;
  *(uint16_t*)0x2000015c = -1;
  *(uint16_t*)0x2000015e = -1;
  *(uint16_t*)0x20000160 = 0;
  *(uint16_t*)0x20000162 = 0;
  *(uint16_t*)0x20000164 = 0xd;
  *(uint16_t*)0x20000166 = 1;
  memcpy((void*)0x20000168, "fq_codel\000", 9);
  *(uint16_t*)0x20000174 = 0xc;
  *(uint16_t*)0x20000176 = 2;
  *(uint16_t*)0x20000178 = 8;
  *(uint16_t*)0x2000017a = 2;
  *(uint32_t*)0x2000017c = 0;
  *(uint64_t*)0x20000788 = 0x40;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[2], 0x20000040ul, 0ul);
  *(uint32_t*)0x20000200 = 0xc;
  *(uint32_t*)0x20000204 = 0xe;
  *(uint64_t*)0x20000208 = 0x20000380;
  memcpy(
      (void*)0x20000380,
      "\xb7\x02\x00\x00\x01\x00\x00\x00\xbf\xa3\x00\x00\x00\x00\x00\x00\x07\x03"
      "\x00\x00\x00\xfe\xff\xff\x7a\x0a\xf0\xff\xf8\xff\xff\xff\x79\xa4\xf0\xff"
      "\x00\x00\x00\x00\xb7\x06\x00\x00\xff\xff\xff\xff\x2d\x64\x05\x00\x00\x00"
      "\x00\x00\x65\x04\x04\x00\x01\x00\x00\x00\x04\x04\x00\x00\x01\x00\x7d\x60"
      "\xb7\x03\x00\x00\x00\x00\x00\x00\x6a\x0a\x00\xfe\x00\x00\x00\x00\x85\x00"
      "\x00\x00\x0d\x00\x00\x00\xb7\x00\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00"
      "\x00\x00\x00\x00\xe3\xa3\x33\xa0\xda\xf2\xf7\x34\x51\xc0\xe1\x7a\x60\x6f"
      "\xec\x68\xcb\x7d\x67\x51\x4f\xe6\x00\x77\xd4\xdd\x90\x12\x3d\x27\xe7\xcf"
      "\x43\x54\x8e\xe8\x58\x57\xad\x4a\x77\xcb\x56\xe0\xfc\xb3\xdf\xd4\xe7\x0e"
      "\xbe\xc6\x77\xd6\xac\x14\xd2\xc7\x94\xf7\x2c\xbf\x5f\xe3\x17\x89\xe7\x02"
      "\x27\xbf\xd8\x11\x5e\xfd\x90\xc8\xc4\x82\x58\xf8\xdb\xe8\x2e\x16\xcf\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x03\x79\x0a\xe2\xfd\x45\xd5\x4b\x10\x7c\x8c\x8a"
      "\x14\x19\x5e\x32\xf1\x37\x99\xd6\x70\x74\x32\xff\x48\xbc\x08\x57\x60\x31"
      "\x41\x66\x44\x3c\xe7\x2c\x74\xf3\xdb\x89\x0e\x1f\xf1\x5a\x10\xd9\x1f\x27"
      "\xe9\xa2\x32\xfe\x22\x38\xff\xf8\x67\xba\x8f\xd4\x1b\x29\xca\xad\x2a\x98"
      "\x6e\x0e\x24\x4b\xd1\x17\x47\xff\xda\x1a\x86\x9d\xb7\xe6\x32\xdf\x4d\xe8"
      "\x57\x23\x44\xb4\x19\xc4\x5c\x21\x70\xfe\x87\x36\x92\xd8\x25\x51\x70\xc1"
      "\x68\x22\xbd\xff\xd3\x13\x54\x80\xda\xe9\x3c\x7e\x33\xbd\xef\x00\x00\x00"
      "\x00\xd8\xfd\x8c\x79\xa5\xd0\x96\x7a\xb7\xe4\x36\x86\xb4\xd1\xe0\x3e\x32"
      "\x6b\xee\xa7\x90\x5e\xf7\xde\x37\x5e\xf8\xbc\x81\x43\xdf\x20\xd1\x3c\x37"
      "\xdb\x26\x99\x71\x21\x0f\xab\x70\x71\xcc\x30\x94\x07\x8a\x04\x47\x77\xaa"
      "\xb9\xd8\x00\x00\x0a\xfe\xfd\x7b\x72\xa0\x95\x0d\x38\x9b\xc9\xcb\x43\xaa"
      "\x60\x7b\x72\x69\x56\x1d\xd5\x0b\x22\xbd\x24\x91\x33\x18\x18\xa1\x0f\x2a"
      "\xc8\xc3\x24\x95\x82\xa2\x0d\x4e\x04\xfd\x1a\xb7\x88\x3f\x65\x6b\x84\x13"
      "\x7d\x5f\x7a\x6e\xdb\xa8\x6a\x7b\x9a\x4c\x2f\x3b\x3a\x8a\xbf\x93\xb2\x80"
      "\xea\x53\xce\x01\xdc\xc2\xd3\x0f\x43\x10\xe8\x28\x1b\x1c\x17\xb5\x0c\xd3"
      "\xef\xba\x43\x2c\xd5\xa6\xc8\x96\xd9\x0c\xdc\x01\x7f\x97\x59\x06\x0e\xa8"
      "\x8a\x2f\x65\x97\xe9\x66\xa8\x5c\x9a\x74\xca\x19\x67\x00\x21\x8f\x91\x97"
      "\x46\xbb\x4b\x84\xc1\x6f\xd5\x6e\x3d\x54\x81\xb3\xd7\x5a\xb7\x61\x3b\x64"
      "\x4b\xa7\x58\x0b\x2a\x09\x42\x39\x4e\xd1\x73\x75\x17\xee\xd1\xc8\x92\x42"
      "\x2f\x54\xd5\xa2\xe1\xcf\x1a\x60\xfe\x2d\xcc\x1a\x46\x5a\xa8\xd5\x4c\xb4"
      "\x00\x00\x53\xc3\xf0\x1e\xa7\x14\xa7\xbd\x4f\xad\x61\x5f\xf6\xe2\x99\x15"
      "\x89\xbb\xdd\x1a\xe0\xd1\xbc\xe6\x5c\x62\x0c\x4a\x25\x27\xa8\x2a\xfe\x16"
      "\xe0\x19\xa5\xe0\xb5\xec\x1b\x1b\x8a\xaf\xba\x09\x00\x00\xc3\x63\x04\x88"
      "\xed\xcc\x4a\x8c\xbd\x32\x46\xe9\x62\xb7\x73\xa7\x5b\x28\xa5\x1c\xd0\x9c"
      "\xbc\xb3\x57\x7f\xea\x6f\x1e\x9f\xe8\xcd\x2b\x53\x2c\x08\x4c\xbd\x05\x1b"
      "\x4a\xea\xf0\xe7\x81\x33\x54\xf1\x4a\x45\x3b\x09\x39\x48\xd4\x9b\xca\x31"
      "\xa7\x17\x04\x19\xbb\x1d\x32\xf2\x56\xff\x30\x10\xe6\x9b\x2f\x04\x82\xc6"
      "\x3a\xb7\x8a\x74\xde\xc8\xb1\x9c\xa7\x6e\x7f\xdf\x29\x76\x6d\x96\xe4\xc2"
      "\xf5\x7b\xa9\x17\xf0\xab\xd5\x25\x3e\x87\xfb\xe5\xb8\x6a\x4b\x91\x51\xc3"
      "\x16\xdf\x39\x25\xd8\x68\x99\xab\x3e\x20\x36\xf4\x86\x6e\x27\x7d\x82\x82"
      "\x91\x65\x92\x2f\x8a\xeb\x4e\xa2\xb9\xdb\x2d\xbb\x6a\x6b\x15\x7d\x40\x19"
      "\x54\x5e\x00\x02\x8d\x09\x60\x8e\xf2\xe1\x86\xfd\x21\x6c\xe8\x4d\x91\x38"
      "\x13\xe1\xbe\x50\xf5\xb6\xb9\x8d\x5d\xab\x01\x80\x00\x00\x00\x00\x00\x00"
      "\xf4\x1c\x29\xb5\x50\xf4\x5d\x68\xa6\x27\xdb\x83\x35\xfd\x97\xbd\x63\x36"
      "\x94\xeb\xb6\x17\x3d\x94\x74\xc5\x0c\x03\x60\xda\xa3\x87\xba\x5d\xa3\xe0"
      "\x92\x46\x3a\x4c\x08\xf2\xc6\x86\xca\x2a\xb5\xf8\xc6\x23\x53\xb8\xf2\xc8"
      "\x17\x00\x00\x00\x00\x29\x7c\x81\x61\xe5\xc9\x39\x85\x52\x71\xfe\x95\xdf"
      "\x69\x0f\x65\x8d\xa3\x81\x53\x64\x6e\x66\x0e\x9b\xa2\x12\x98\x6c\xd3\x62"
      "\x8a\x50\x0d\x14\xd3\x78\xd4\xaf\x50\x8f\x42\x8e\xcb\x0e\x85\x32\x2f\x9f"
      "\xd3\x17\xb2\x9e\xc2\x3b\x19\x3b\x1d\xed\x28\x24\xdc\x90\xee\xea\x4c\x56"
      "\xa6\x76\xe9\xd6\x46\x25\xcd\xc2\x66\x60\x9d\x2f\x9e\x6b\x92\x5c\xc4\x13"
      "\xcd\xe5\x41\x48\x69\x38\x7f\x23\xa3\x09\xfe\x29\xc7\xe6\x31\x01\xfe\xc3"
      "\x4b\x22\xaa\xe7\x7b\xd5\xdf\x5f\x61\x81\xf5\xb4\x63\x91\x19\xfc\x54\x46"
      "\x89\x34\x79\xe7\x90\x7e\x2e\xed\xfd\x72\x89\x63\x66\x2f\x80\x29\x92\x6a"
      "\x7f\xee\xef\x66\xc6\xf9\x69\xb1\x86\xf5\xd5\xe6\x7d\xcb\x4e\x24\xd4\x84"
      "\x8d\xe6\xa0\x7a\xb8\x85\x45\x09\xe1\x9c\xe4\x26\x68\x05\xd3\x4f\x1a\xad"
      "\xee\x9f\x42\x74\x87\x31\xc7\x42\x15\xa2\x00\x9e\xe2\x76\x93",
      1059);
  *(uint64_t*)0x20000210 = 0x20000340;
  memcpy((void*)0x20000340, "syzkaller\000", 10);
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  memset((void*)0x20000230, 0, 16);
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = -1;
  *(uint32_t*)0x2000024c = 8;
  *(uint64_t*)0x20000250 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000258 = 0x24f;
  *(uint32_t*)0x2000025c = 0x10;
  *(uint64_t*)0x20000260 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint32_t*)0x2000026c = 0;
  *(uint32_t*)0x20000270 = -1;
  res = syscall(__NR_bpf, 5ul, 0x20000200ul, 0x48ul);
  if (res != -1)
    r[3] = res;
  *(uint32_t*)0x20000140 = r[3];
  *(uint32_t*)0x20000144 = 0x60;
  *(uint32_t*)0x20000148 = 0xe;
  *(uint32_t*)0x2000014c = 0;
  *(uint64_t*)0x20000150 = 0x20000100;
  memcpy((void*)0x20000100,
         "\x28\x0e\x00\x3f\x43\x05\x60\x7e\x5b\xc5\x79\x5e\x65\x58", 14);
  *(uint64_t*)0x20000158 = 0;
  *(uint32_t*)0x20000160 = 0x100;
  *(uint32_t*)0x20000164 = 0xf2ffffff;
  *(uint32_t*)0x20000168 = 0;
  *(uint32_t*)0x2000016c = 0;
  *(uint64_t*)0x20000170 = 0;
  *(uint64_t*)0x20000178 = 0;
  *(uint32_t*)0x20000180 = 0;
  *(uint32_t*)0x20000184 = 0;
  syscall(__NR_bpf, 0xaul, 0x20000140ul, 0x48ul);
  return 0;
}
