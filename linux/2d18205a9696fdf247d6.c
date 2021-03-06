// possible deadlock in strp_work
// https://syzkaller.appspot.com/bug?id=2d18205a9696fdf247d6
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 2ul, 1ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20e5b000 = 2;
  *(uint16_t*)0x20e5b002 = htobe16(0x4e20);
  *(uint32_t*)0x20e5b004 = htobe32(0xe0000001);
  syscall(__NR_bind, r[0], 0x20e5b000ul, 0x10ul);
  *(uint16_t*)0x20ccb000 = 2;
  *(uint16_t*)0x20ccb002 = htobe16(0x4e20);
  *(uint32_t*)0x20ccb004 = htobe32(0);
  syscall(__NR_connect, r[0], 0x20ccb000ul, 0x10ul);
  *(uint32_t*)0x20000100 = 1;
  *(uint32_t*)0x20000104 = 3;
  *(uint64_t*)0x20000108 = 0x200008c0;
  memcpy(
      (void*)0x200008c0,
      "\x18\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x26\xd0\x00\x00\x95\x00"
      "\x2b\x00\x00\x00\x00\x00\x93\xad\xff\x00\x00\xdc\xc1\xf2\x55\x53\xae\x83"
      "\x1e\xf3\x23\xa8\x22\x55\xf6\x74\x41\x2d\x02\x00\x00\x00\x00\x00\x00\x5a"
      "\xb5\x27\xee\x36\x97\xf5\x7f\xe1\x2e\xa7\x50\x9e\x1f\xcf\xb0\xb3\xf4\x27"
      "\x9e\x7b\x34\x60\xdd\x37\x53\x63\xed\xf6\xba\x6b\xda\x6d\x2a\xfe\xe3\x30"
      "\x25\xa3\x0b\x45\xbd\xcf\x1d\x27\x36\x83\x62\x6e\x00\xfc\x25\x4d\x57\x0d"
      "\xca\x6b\x78\xad\x83\x34\x88\xcf\xe4\x10\x9e\xd2\x04\x9e\xdd\x0d\x69\x61"
      "\x3d\x3c\xd6\x1f\x00\x15\x8e\x23\x00\x00\x00\x00\x00\x00\x00\x85\x31\xbf"
      "\xf8\xc0\xc8\x2e\x9b\x1a\x00\xe2\x58\xf8\x2b\x6c\xe7\x23\x13\xe0\x75\xbb"
      "\x49\x27\x46\xec\xf5\x9f\x45\xa2\x44\x36\x30\xfc\xb5\xb4\xf9\xeb\x5e\x84"
      "\xb1\xaf\x1a\x8a\x2b\x36\xf8\x79\x96\x28\xc8\x72\x63\xd9\xd8\x32\xb6\xca"
      "\x6f\x29\x77\xe3\x34\xa4\xee\xfc\xd5\x6e\xb6\xee\x1e\x3f\x9b\x89\x16\x44"
      "\x6f\x0a\x8c\x2a\x00\x00\xd1\x41\x7b\xde\x5c\x92\x9a\x76\x5a\x78\xcc\xdc"
      "\x89\x09\x43\xec\xc2\x91\x6f\x14\x07\xc9\xab\x51\x61\xe4\x3c\x47\xda\x04"
      "\x08\x8b\x81\x19\x5b\xb1\x12\xa3\x47\x1c\xbf\xe8\x2e\xb5\xca\x3f\x9e\x42"
      "\x04\x90\x28\x90\x0f\xb3\x8f\xfa\x35\xe7\x71\x2e\xef\xd6\x44\x94\x21\x0e"
      "\xbd\x07\x24\xbb\xe3\x3c\xa3\xaf\xf5\xac\x81\x4e\x7e\xa0\xfa\x4a\xd9\x37"
      "\xd9\x1f\x14\x84\xf0\xd6\x4e\xed\x8f\x4d\x66\x1a\xa8\xea\xa7\xa3\x84\x73"
      "\x4d\xd1\xf7\xea\x38\xb8\x86\x86\xa4\x6f\x6b\x93\xc9\xcb\x42\xfc\x0a\xe0"
      "\xb1\x1a\xe0\x0a\x15\xae\xd2\xdb\x19\xb5\x9f\x94\x47\x7d\x15\xd4\xa9\xc2"
      "\x7c\x6b\x6d\x9d\x60\x53\xef\x16\xf9\xf7\xd3\xe9\x72\xdd\x85\x65\xc7\xfe"
      "\xe5\x80\xc9\x30\x1f\xef\x96\xbe\x02\x95\x8a\x62\x80\x8b\x84\xb7\x9b\x17"
      "\x76\x27\xc9\xd1\x0c\x5a\xc3\x91\x8f\x25\xba\xa0\xe9\x4a\xf1\xe1\x67\x58"
      "\x42\x98\xf1\x24\x2e\x75\x76\x0e\x85\x80\x18\x19\x4f\xb2\x68\xbc\xee\xfe"
      "\x4e\x91\x69\x17\x40\x16\x3a\x48\x89\xad\x47\x7a\x8a\x39\xb8\x61\xfe\xa3"
      "\x06\xa8\x7c\x1c\x19\x94\xc0\xaf\x68\x38\x16\xd6\x91\xe0\xa2\x2c\xc8\x3c"
      "\xca\x0d\x66\x13\x59\xef\x0c\xed\xd8\x7e\xcc\x09\x89\x54\x97\x9d\xfd\xc4"
      "\x65\xc3\x41\xdc\x9f\xbd\x3e\xba\x11\x31\xfe\x2a\x5e\x75\xd0\x05\x8b\xef"
      "\x91\x1f\x6d\x0a\xc0\x9b\x0a\x45\xe4\x45\xf9\xa3\x9f\xa8\x52\xf8\x5c\x49"
      "\x82\xd9\x72\xda\x12\x72\x4c\xcd\x3e\x6b\xa4\x67\xb6\xcd\x00\x00\x00\x00"
      "\x00\x00\x00\x8a\x19\xd8\x74\xb2\x45\xf4\x64\x2f\x4c\x9a\x28\x48\xd2\x96"
      "\xcf\xdc\x25\xba\x13\xaa\xaf\x7e\x93\x24\xe2\x7f\x7e\xc4\x55\x30\x98\x26"
      "\xe8\x50\x7f\x8d\xd2\xb9\xcd\xb2\x0c\x8c\xc4\x2d\x51\x4b\xe0\xfe\x58\x9e"
      "\xa0\x17\xc2\xf2\xa9\x0c\xc3\x19\x92\xa0\x5b\x24\xf0\x6b\xc2\x7c\x14\x55"
      "\x30\x7b\x3c\x6f\xa8\x00\xf2\x19\x6d\xa0\x6d\xee\x8b\xad\x19\x58\x7d\xc0"
      "\x92\x8c\x8e\xc9\xbd\x58\x37\xd9\x59\xd5\xd4\x36\x74\xb1\xc0\x0f\x0b\xd5"
      "\xc0\x60\xf8\xbc\x9b\x00\x00\x00\x00\x7d\xe7\xb0\x2c\x36\x04\x98\xdb\x4b"
      "\xd7\x55\x5d\xec\x6d\x45\xe8\xc0\xee\x8e\x45\x83\x4c\x51\xdc\x39\x95\xfc"
      "\x42\x42\x33\x1d\x24\xec\xff\xb6\xb5\x5b\x07\xa7\x0b\x1a\x78\x53\x3c\x55"
      "\x87\xd7\x1f\x63\x63\x7d\x29\x0c\xb2\x03\x66\x8f\x44\xb5\x91\x6d\xbd\x64"
      "\x79\xba\x72\x51\xe6\x01\xd6\x6d\x79\x79\xbd\xc7\x71\x34\x6f\xd7\x22\x68"
      "\xd1\x07\xc2\xa9\x61\x2b\xa0\x91\xf6\x6f\xb1\x39\x09\x2a\xe8\x69\x46\xb5"
      "\x38\xde\xde\xc3\xb9\x28\x3c\x8b\x29\x46\xf8\x8a\x46\xec\x01\x33\x8c\x5f"
      "\xbc\xb7\x33\x79\xc5\xba\x75\xda\xe5\xef\x2d\xcd\xc3\x30\x66\x92\xb5\x92"
      "\x03\x4c\x0e\xf5\x0a\xa2\x04\xf0\x7c\x9e\x1f\xf7\x96\xbd\x61\x9e\x47\x8b"
      "\xc1\xb3\x39\xd1\x31\x25\x03\x9e\xf6\x89\x27\xeb\xba\x22\x49\x7b\x17\x57"
      "\xd6\x5b\x9f\x1a\x6b\x40\x2c\xb9\x84\xf9\xf9\x59\xe7\x2b\xe0\xae\x24\x46"
      "\xf2\xd8\xc2\xc6\x9f\xfc\x0f\xb1\x3f\xc9\x9b\x30\x26\xd3\x85\x5e\x9a\x51"
      "\x6b\x9d\x9d\x65\x9b\x4e\xc1\xef\x98\xd8\xd2\x93\x51\x8e\xbc\x53\x9f\xb0"
      "\xbf\xb9\x83\xd2\x02\x48\x75\xec\xab\x0a\x0a\x29\xf0\x9e\xb6\xb7\xc4\x53"
      "\xf7\x74\xb9\x29\xeb\x1b\x8d\xab\xed\xfa\xf4\xac\xb5\xd2\xa8\x42\xac\x6f"
      "\xc7\x6a\x2e\x01\x83\xa0\x34\x01\xde\xc9\x66\xce\x46\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x6c\x9e\x82\xc0\xc4\x99\x73\x61\xc6\xac\xec\x9d\x1e\x9c\x42"
      "\x1e\x92\x39\x76\x2a\xa6\xf9\xf0\x38\x2c\x21\xeb\x48\xca\x04\x78\xc1\x63"
      "\xa2\xb6\x2f\xd4\x6e\x41\x09\xb3\x89\x96\x61\x62\x8a\xc2\x42\x88\xf6\x56"
      "\xf6\x37\x43\x73\xe5\x01\x7f\xce\x00\x2c\x4e\x04\xf8\xca\x63\xae\xe8\xdc"
      "\x51\x44\x54\x89\xe6\x4e\x03\x5a\x2c\x1a\x26\x18\x3f\x96\x64\x72\x17\x0f"
      "\x02\xf9\x28\x6d\xeb\x03\xdc\x65\xc1\xd2\x4e\x87\x18\x67\x63\x1b\xf7\x18"
      "\x96\xa4\x7d\xde\xb5\x4e\xaf\xc8\xed\xc1\xd6\x6b\x2a\x40\xfc\x3f\x93\xc8"
      "\x93\xf6\x00\x00\x00\x00\x00\x00\x00\x1a\xdc\xab\xa0\xa8\x91\xfc\xb8\x37"
      "\x3a\x57\xa7\x26\xa6\x74\xf0\xd5\x43\x3d\x0d\x3f\xb7\xb3\x9f\x17\xbf\x5f"
      "\x12\x00\x00\x00\x00\x93\x05\xd8\xf7\x2b\x4c\x6d\x40\xfd\xa2\xe8\xbf\x35"
      "\x89\x8a\xb8\xde\x9f\xd2\xff\x63\xbf\x0f\x83\x3c\x97\xb2\x76\x7d\xc2\x14"
      "\x22\x76\x06\xb7\xdc\x67\x4e\x5e\x3c\xf0\xe9\x45\x4f\xf1\x0b\x5a\xa9\xe0"
      "\xff\xd9\x54\x5d\x78\x6b\xb7\x5f\x4b\x4c\x30\xd7\xd3\x52\x6e\x0b\xad\x96"
      "\x63\x24\x70\x47\xd1\xc9\x7a\x9a\xa5\x38\x04\x4b\x48\x47\x3d\xcc\x46\x27"
      "\xe5\x8e\x79\x4e\x9c\xbe\xef\x57\x11\xec\x6a\x1f\xb9\x91\x99\xb8\xff\x8f"
      "\x9f\x09\xd5\x67\xa5\x95\xb8\x96\xf0\x3c\x6a\x1e\x85\x7b\x78\x26\x20\x8c"
      "\x3c\xea\xc0\xe0\x84\x1c\x26\x28\xc3\xe6\x91\x56\xf6\x41\xf7\x6f\x50\x1b"
      "\x6c\xb6\xc7",
      1245);
  *(uint64_t*)0x20000110 = 0x202bf000;
  memcpy((void*)0x202bf000, "syzkaller\000", 10);
  *(uint32_t*)0x20000118 = 4;
  *(uint32_t*)0x2000011c = 0x1c9;
  *(uint64_t*)0x20000120 = 0x20000040;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  *(uint8_t*)0x20000130 = 0;
  *(uint8_t*)0x20000131 = 0;
  *(uint8_t*)0x20000132 = 0;
  *(uint8_t*)0x20000133 = 0;
  *(uint8_t*)0x20000134 = 0;
  *(uint8_t*)0x20000135 = 0;
  *(uint8_t*)0x20000136 = 0;
  *(uint8_t*)0x20000137 = 0;
  *(uint8_t*)0x20000138 = 0;
  *(uint8_t*)0x20000139 = 0;
  *(uint8_t*)0x2000013a = 0;
  *(uint8_t*)0x2000013b = 0;
  *(uint8_t*)0x2000013c = 0;
  *(uint8_t*)0x2000013d = 0;
  *(uint8_t*)0x2000013e = 0;
  *(uint8_t*)0x2000013f = 0;
  *(uint32_t*)0x20000140 = 0;
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = -1;
  *(uint32_t*)0x2000014c = 8;
  *(uint64_t*)0x20000150 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000158 = 0;
  *(uint32_t*)0x2000015c = 0x10;
  *(uint64_t*)0x20000160 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000168 = 0;
  *(uint32_t*)0x2000016c = 0;
  *(uint32_t*)0x20000170 = -1;
  res = syscall(__NR_bpf, 5ul, 0x20000100ul, 0x48ul);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x29ul, 5ul, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000380 = r[0];
  *(uint32_t*)0x20000384 = r[1];
  syscall(__NR_ioctl, r[2], 0x89e0, 0x20000380ul);
  *(uint32_t*)0x20000180 = r[0];
  *(uint32_t*)0x20000184 = r[1];
  syscall(__NR_ioctl, r[2], 0x89e0, 0x20000180ul);
  return 0;
}
