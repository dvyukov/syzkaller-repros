// kernel BUG at ./include/linux/skbuff.h:LINE!
// https://syzkaller.appspot.com/bug?id=7aee4547f8bfbd6c7b86331670df496cd7c871d8
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

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  memcpy((void*)0x20ae8f92, "\xb1\x00\x48\x5c\xaa\x2a", 6);
  *(uint8_t*)0x20ae8f98 = 0xaa;
  *(uint8_t*)0x20ae8f99 = 0xaa;
  *(uint8_t*)0x20ae8f9a = 0xaa;
  *(uint8_t*)0x20ae8f9b = 0xaa;
  *(uint8_t*)0x20ae8f9c = 0;
  *(uint8_t*)0x20ae8f9d = 0;
  *(uint16_t*)0x20ae8f9e = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x20ae8fa0, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ae8fa0, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ae8fa1, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20ae8fa1, 0, 2, 6);
  *(uint16_t*)0x20ae8fa2 = htobe16(0x1c);
  *(uint16_t*)0x20ae8fa4 = 0;
  *(uint16_t*)0x20ae8fa6 = htobe16(0);
  *(uint8_t*)0x20ae8fa8 = 0;
  *(uint8_t*)0x20ae8fa9 = 0;
  *(uint16_t*)0x20ae8faa = 0;
  *(uint32_t*)0x20ae8fac = htobe32(0);
  *(uint32_t*)0x20ae8fb0 = htobe32(-1);
  *(uint8_t*)0x20ae8fb4 = 0;
  *(uint8_t*)0x20ae8fb5 = 0;
  *(uint16_t*)0x20ae8fb6 = 0;
  *(uint32_t*)0x20ae8fb8 = htobe32(0xe0000001);
  *(uint32_t*)0x203b5000 = 0;
  *(uint32_t*)0x203b5004 = 1;
  *(uint32_t*)0x203b5008 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20ae8fb4, 8);
  *(uint16_t*)0x20ae8fb6 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x20ae8fa0, 20);
  *(uint16_t*)0x20ae8faa = csum_inet_digest(&csum_2);
  r[0] = syscall(__NR_socket, 0xa, 2, 0x88);
  *(uint64_t*)0x2000b000 = 0x2000bfe4;
  *(uint16_t*)0x2000bfe4 = 0xa;
  *(uint16_t*)0x2000bfe6 = htobe16(0x4e23);
  *(uint32_t*)0x2000bfe8 = 0;
  *(uint8_t*)0x2000bfec = 0xfe;
  *(uint8_t*)0x2000bfed = 0x80;
  *(uint8_t*)0x2000bfee = 0;
  *(uint8_t*)0x2000bfef = 0;
  *(uint8_t*)0x2000bff0 = 0;
  *(uint8_t*)0x2000bff1 = 0;
  *(uint8_t*)0x2000bff2 = 0;
  *(uint8_t*)0x2000bff3 = 0;
  *(uint8_t*)0x2000bff4 = 0;
  *(uint8_t*)0x2000bff5 = 0;
  *(uint8_t*)0x2000bff6 = 0;
  *(uint8_t*)0x2000bff7 = 0;
  *(uint8_t*)0x2000bff8 = 0;
  *(uint8_t*)0x2000bff9 = 0;
  *(uint8_t*)0x2000bffa = 0;
  *(uint8_t*)0x2000bffb = 0xbb;
  *(uint32_t*)0x2000bffc = 0;
  *(uint32_t*)0x2000b008 = 0x1c;
  *(uint64_t*)0x2000b010 = 0x20004000;
  *(uint64_t*)0x2000b018 = 0;
  *(uint64_t*)0x2000b020 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x720;
  *(uint32_t*)0x20000008 = 0x29;
  *(uint32_t*)0x2000000c = 3;
  memcpy(
      (void*)0x20000010,
      "\xab\xe1\x40\x56\x36\xdf\x81\x0d\x4c\x0b\x20\x57\xd1\x03\xdc\x16\xb0\x83"
      "\xaa\x6e\xee\x43\xca\x3c\xa6\x9f\x91\x8a\xa5\xf7\x8f\x9b\xbf\x6d\x80\x35"
      "\x1b\xaf\x82\x94\xf6\x2e\xca\x4f\x35\x1a\x70\x9d\xbf\x2e\xe8\x2a\x4b\x38"
      "\x4d\x32\x21\x00\x00\x00\x1f\xc5\x91\xb7\x12\x35\xbd\xae\x4c\x84\xfa\x89"
      "\x51\x3d\xc4\x06\x0f\xad\xdd\x7a\xfb\xf0\x62\xab\xe4\xf8\x70\x5b\xe6\xa7"
      "\x63\x76\xfa\xc3\x35\xbb\x50\xf1\xd9\x50\x9f\xce\x77\x97\x49\xe2\x2d\x59"
      "\x34\x50\x0f\xee\x73\x83\xa6\x5d\xb3\x3f\x6d\xc3\x50\x7b\x9f\x42\x30\x15"
      "\x0d\xe8\xda\xdb\xc3\xe6\xe9\x56\x44\x22\x4e\x69\xd9\xb7\xdd\x46\x4e\x49"
      "\x3f\xe0\xb1\xb0\x88\xc3\xfd\xde\xd9\xa9\x48\x95\x7c\xdd\x4a\xf4\x2c\x7f"
      "\x97\x3b\x7d\xe2\xc7\x98\x3b\xd5\x53\x8d\x6d\x65\x6b\x0a\x9f\x83\x40\xf0"
      "\xbc\xdb\x11\xc6\xe4\x10\xdd\x35\x4a\xf4\x37\x51\xb6\x90\xa0\x62\x46\x45"
      "\x35\xc9\xcc\xc2\x0e\x62\xa1\xd2\xd8\xe5\xcd\xff\x67\xfc\x02\x48\xb4\x9b"
      "\x45\x73\xcd\xb2\xd8\x8b\xe7\x07\xb9\x24\x4a\x5b\x50\xac\x63\x26\x6a\x37"
      "\xf9\x73\xf2\xf6\xbd\x6b\x82\x41\x25\x51\x4e\x2d\x79\x90\xd2\x11\x9c\x89"
      "\xc2\xf7\x46\x25\xef\x5f\x48\x32\x6e\x45\x63\xa9\x6a\x24\x16\x7f\x08\xa7"
      "\xc0\x11\x6f\xa9\xeb\x02\x65\xac\x42\x8e\x2a\x7c\x43\x9e\xcf\x8d\x0a\x54"
      "\x61\xc0\x28\x8b\x0f\xf0\x54\x49\x9b\xfd\xc4\x17\x06\x0c\x70\xbf\x67\x6e"
      "\xb1\xa8\x63\x71\x79\xfe\x43\x9b\x08\x2e\x8c\x62\xe5\x6a\x1c\xc6\xb2\x9c"
      "\xe1\x45\xe2\xa1\xeb\x53\x4a\x99\x05\xd0\x65\xfd\x78\xed\xac\x44\xd7\x32"
      "\x8a\x33\x35\xb9\x0c\x90\xb9\xd9\xde\x27\x64\xbf\xa2\x6f\xf4\x23\x0e\xfc"
      "\xab\xc7\x54\x3d\x44\xa4\x0a\xc2\xdd\xe3\x11\x2a\x06\xb0\x82\x76\x5d\xa2"
      "\x90\x83\x2b\x96\x4b\xd4\x0b\x5b\x92\x1f\xec\x3a\xb0\xea\x64\x7e\x7e\x1b"
      "\xc9\x0c\x4c\x28\x34\xe5\x58\xec\x12\x0c\xb8\xdc\xaf\x38\x3f\x76\x43\x7d"
      "\x5d\xeb\x4c\x31\x33\x54\x9c\x5c\x7f\x3d\xad\xfe\xe8\xdb\x5b\xa4\x85\xe6"
      "\x19\x2f\x5c\xa4\xe1\xc3\x3a\x98\x5d\x2e\x2d\x0b\x3f\x91\x76\xaf\x9b\xca"
      "\x09\xdc\x3c\x09\x7d\x9f\x14\xcb\x1d\xf0\x2d\xe9\x66\x9e\x46\xa3\x70\xf0"
      "\x7d\x24\xf3\x24\x03\x5f\x89\x81\x0b\xae\x52\xca\x76\x2b\x43\x0d\x89\xb1"
      "\xb6\x89\x4c\x15\x22\x64\xdc\x96\x44\xbd\x4e\xe9\x45\x9f\x3b\x05\x4a\xc3"
      "\xa0\x8e\xaf\x0d\x26\xc7\x0d\x78\x7f\xe5\x3a\x3b\xf9\x7f\x4d\x68\x27\x53"
      "\x02\x55\x3c\x83\x7d\x4d\xd5\xc0\xb4\xa2\x4e\xcb\x48\x50\xc6\xa8\x7e\x8d"
      "\x6b\x73\x11\x66\xeb\xbf\x9b\x28\xaf\x80\x99\xb0\xe6\x9f\x09\x26\x7a\xb9"
      "\x2d\xd6\xe6\x05\xd6\x85\x7d\xd9\x4c\x26\xe4\x5c\x44\x8f\x03\x5e\x00\xe5"
      "\x06\x58\xab\x8f\x76\xed\xaf\x16\x87\x73\x78\xdc\x0a\x15\x8f\x99\xed\x85"
      "\xdb\xed\x85\xcd\xe1\xaf\xb3\xd2\x30\xce\x62\x20\x1c\x10\xdf\xb8\x44\xe3"
      "\x23\x58\x3f\xd7\x3e\x55\x10\xde\x94\x95\xef\x8e\xaa\x0e\x2e\x90\xb2\xbe"
      "\x60\xb5\x9b\x3d\x51\xfa\x34\xd3\x40\xca\x2f\x5d\x18\x2d\xda\x89\x45\x79"
      "\xa5\x27\xf2\xc6\x9d\x9b\x34\x0a\x02\xf3\x85\xde\x57\x39\x6c\xd1\xe2\xfe"
      "\x9c\x00\xe0\x5b\xc3\x5e\x5e\xf2\x94\xa4\x71\x0d\x96\x0a\x1a\x74\x3e\x4e"
      "\x24\x20\xf7\x59\xdc\xae\xdd\x17\x4d\x0c\xb8\x66\xe4\x11\x74\xe2\x9e\x91"
      "\x5f\xd5\xd8\xf3\x7f\xfc\x73\x58\x5b\x84\x7b\x8a\x88\x61\x5b\x4f\x7f\xea"
      "\x33\x42\x27\xbc\x1c\xa9\xcd\xcd\x1a\xb4\x0d\x4d\x05\x3b\xd1\x40\x79\x0a"
      "\x8a\x59\x46\xc2\x46\x41\x8d\xdd\x17\x67\xc8\x5a\xf7\x91\xaa\x28\xdc\xcb"
      "\xd0\x87\x8f\x8f\x32\x94\xaa\x9f\x42\x26\x8a\x71\x3c\xe5\x24\xc2\x82\x02"
      "\x9e\x85\x99\xf4\x1b\x1c\x73\x91\xd1\xad\xfb\xbf\xd6\xf6\x17\x0a\x46\x32"
      "\x78\x05\x89\x4c\x49\x73\xd5\x34\xde\x75\x22\xfa\xe7\x84\x95\x14\x12\xd4"
      "\x0a\x20\x60\x44\x59\xdf\x66\x53\xbb\x9e\x70\x63\x1c\xde\x23\xb3\x52\xe1"
      "\xb2\x61\xc1\xe7\xfa\x30\x61\x26\x32\xe0\xff\xb6\x5d\xde\xd0\x37\x86\xc1"
      "\xca\x82\x27\x2b\xaa\xf9\xbc\xf0\x7e\x81\x27\x23\xfc\x3b\xd3\x51\x3b\x7f"
      "\xf9\x35\xb8\x4a\x32\xe8\x77\x4b\xd4\xca\x52\x39\x76\x23\x31\x8c\xba\xcb"
      "\xbb\xaa\x8f\x66\xe6\x6a\x54\x82\xf3\xe0\x6b\x9f\xea\x51\x4f\xd3\x9a\x3d"
      "\x81\x06\x08\xfb\xb3\x60\x54\x58\x06\xa5\x93\xa7\x38\xe1\x06\x7b\x9e\x53"
      "\x87\x89\x14\xae\x56\x7e\x28\x83\x05\x51\xd1\xb5\x90\x66\x23\x39\xda\x58"
      "\x25\x2d\x97\xa2\xd0\x45\x59\x3c\x4c\x0a\x36\x6b\xd6\xfc\x40\xcb\xc3\x26"
      "\x05\xf7\x27\x03\x47\x5c\x18\x7e\x1e\xd1\x3b\x5b\x08\x63\x2d\x39\x4e\xb9"
      "\x87\xa9\x53\x3c\xb9\xea\x1d\x86\xac\xf5\xcf\xb8\x27\x0b\x0d\xb3\xf8\xa6"
      "\x23\x8b\x45\xc5\x61\x13\xf6\xcc\x94\x32\x4b\x98\xcf\xd7\x59\x52\x64\x06"
      "\x26\xd1\xf7\x9c\x66\xff\x9e\xf6\x7a\xcc\x54\x5a\xe4\x26\x45\x44\xc3\xdd"
      "\x7c\x78\x28\x4f\xc5\x3d\x8e\x1b\x6f\x06\x43\xc7\x0d\xa4\xa1\x98\x8a\x06"
      "\x86\xbc\x9b\xfb\xb4\xc7\xff\x5a\xae\xea\x41\x6d\x77\x04\x9b\x13\x00\x94"
      "\xa9\x5a\xfd\x95\xf5\xb6\x36\xaf\x11\x1c\xd7\x4e\x66\xa0\x1f\xdb\xa9\x98"
      "\x74\x9c\x56\x16\xce\xb2\x0e\xcf\x0b\xa9\x60\x45\x4f\x27\xed\x69\xf3\x46"
      "\x17\x2e\x78\xf7\x1e\x0f\xd0\x3f\xd0\x9c\x2c\x7d\xa4\xa0\x44\x0d\xeb\xf6"
      "\xfe\x93\x39\x49\x68\x62\x66\xa3\x78\x09\x58\xa5\xd5\x29\x79\x38\xbe\x51"
      "\x9e\x11\x3f\xad\xd7\x07\x15\x7b\x39\x3d\xf3\x52\x58\xbe\xad\x87\x8a\x1d"
      "\x4b\x55\xf3\x6b\x2b\x29\x16\xcc\x0a\x5d\xa8\x6d\x99\x98\x75\xd1\x57\x26"
      "\x58\x18\x16\xcd\x2d\xed\x58\xf7\xf0\x58\xb6\x06\x8a\x9a\x00\xe3\x70\x7c"
      "\x69\x4e\x69\x0f\xfc\x8c\x8b\x60\x94\xb8\x1c\xf3\xd7\xad\xa2\x15\x34\x04"
      "\xe8\x8e\x2a\x56\xfb\xfa\xc3\xed\x55\x38\xf5\x90\xed\x59\xc3\x16\xd4\x7e"
      "\x20\xd3\x04\x99\x8d\x17\x92\xf2\x6b\x46\x0f\xd1\x90\x9b\x11\xa4\xa0\x7f"
      "\x33\x82\x8d\xc7\x73\x83\x9a\xd1\x17\xf8\x10\xf1\x6e\x75\xe0\x19\xd1\xaa"
      "\x4f\x2d\xb1\x26\x7e\xcb\xa3\xb8\x4d\x63\x9e\x77\x30\xe8\x98\xaf\x32\x1d"
      "\xb1\xc2\xf9\xbb\x76\x69\xa9\xfc\x09\xb8\x6b\xb9\x1b\x79\x7a\xdc\x80\x21"
      "\xf2\xa0\xf5\xca\x84\xe0\xb6\xaa\x4e\x02\xfe\x0e\x97\xa3\x9d\xe6\x97\x21"
      "\x94\xfd\x5f\xe2\x41\x16\x85\x3c\x29\x5b\xaa\x2e\xd5\x74\xaf\x9e\xc1\xb7"
      "\xbd\xcd\x62\xc4\x0e\x0b\xe3\xfa\xf2\x08\xf0\x8a\x27\x62\xa8\xac\x62\x65"
      "\x1d\x37\x76\x8b\x03\x1f\x70\xc6\x43\xb9\xf8\xe7\x75\x30\xcd\xdf\xb7\x1f"
      "\xfd\x9e\xe5\x5a\x55\x02\x69\x5a\xf5\xf5\x02\x52\x9f\xca\x02\x3d\x95\x5f"
      "\x2a\x5f\x88\xd0\x40\x35\x08\x3f\x59\x7e\xae\xbe\x3c\xd1\xaa\x5f\x76\xc2"
      "\xac\x93\x85\x7c\x86\x62\xa8\xbf\x8b\xaa\x8f\x4c\x12\xfe\x73\xb8\x02\x06"
      "\x67\xf0\x70\x07\xf4\x64\xce\x47\x08\x3a\x7f\x5c\x3e\x5b\xcf\x5b\x80\x6f"
      "\x01\xb8\x4d\x8b\xf8\x16\x84\x8a\xcb\x28\x2e\x76\x19\x80\x89\xb2\xc5\xaa"
      "\xdc\x2d\x6c\xe8\xd0\x91\x33\xf8\xc7\x5f\xb2\xd0\x37\x15\x28\x8f\xf7\xd5"
      "\xfd\xab\x4f\xbd\x58\x7d\x8f\x72\x1e\xf5\x7a\x0f\x04\xb1\x3a\x7d\xc0\xa9"
      "\x81\x28\x00\x14\xb4\x6f\x39\x78\x68\xe2\xc5\x06\x84\x91\xd7\x99\x96\xc4"
      "\x05\xf5\xd7\x3a\x27\x21\x6f\xab\x65\xa0\x4d\xb6\x4b\x97\x7a\x83\x23\x36"
      "\x98\xfa\x2e\x55\x7f\x83\xdb\x0f\x31\xc7\xf2\x27\xbb\xc3\xa4\xc8\x8b\x69"
      "\xf4\x8c\x59\xfd\xcd\x2f\x26\x4c\x66\xea\xc7\xcc\xde\x70\xb1\x43\x7c\xc2"
      "\x38\x99\xa0\x11\x72\x30\x26\x8d\x44\xe3\xe3\x9d\xbc\x86\xb7\xeb\xdf\x56"
      "\xd0\xb5\xa8\x40\xf3\x2e\x9a\x68\x22\x92\xf0\x22\x02\xcd\xb9\x1a\x3a\x11"
      "\xb2\xb7\x84\x51\xff\xae\x5d\xdd\xe3\x3c\x4c\x56\x0e\x17\x5c\x1e\xff\x0e"
      "\xac\x00\x5c\x97\x03\xcb\xe5\xe8\x45\xb6\x0a\x07\x96\x74\x60\x9e\x80\x92"
      "\x1d\x45\x3b\x6d\xa5\x28\x22\x63\xa6\x0a\xf3\x7f\xfb\x89\x0f\x3a\xc6\xb7"
      "\x05\x94\xba\xd8\x4a\xc9\x3d\xb8\x3a\xa4\x52\x59\x13\x57\x19\x39\x67\x9f"
      "\x15\xac\x71\xbe\xb0\x5d\xcb\xed\x02\xc7\x9a\x35\x43\x04\xe8\x68\x15\x41"
      "\x6a\x11\xd6\x72\x9a\xd0\x4d\x44\x8b\xbb\xb1\xfc\x80\x97\x8b\xb7\x53\xee"
      "\x6a\x50\xaf\xd2\x55\x79\x08\x9d\xb7\xc9\x97\xb7\x1e\x2d\x0a\x53\xc9\xf1"
      "\x14\x2c\x4e\x55\x37\xc5\x8d\xa6\xbb\xac\x31\x86\xb0\x50\xe1\x77\xc3\x00"
      "\x96\x0c\x50\x37\xe9\x43\x0f\xb1\x3b\x3d\xf9\xa8\x12\xc5\x65\xf8\x00\x00"
      "\x00\x00\x00\x00\x1c\x24\x82\x4e\x08\xae\x49\xbf\x08\x1e\x1e\x9b\x5f\x04"
      "\x87\xef\xbc\xf5\x4d\x8d\x40\xb2\xbb\x62\x8f\xd5\x1d\xb7\x04\xb5\xa6\x76"
      "\x10",
      1801);
  *(uint64_t*)0x2000b028 = 0x720;
  *(uint32_t*)0x2000b030 = 0;
  syscall(__NR_sendmsg, r[0], 0x2000b000, 0x2000c000);
  *(uint64_t*)0x20001f00 = 0;
  *(uint32_t*)0x20001f08 = 0;
  *(uint64_t*)0x20001f10 = 0x20000280;
  *(uint64_t*)0x20000280 = 0x20000000;
  *(uint64_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20000080;
  *(uint64_t*)0x20000298 = 0;
  *(uint64_t*)0x200002a0 = 0x200000c0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint64_t*)0x200002b0 = 0x20000180;
  memcpy((void*)0x20000180,
         "\xfb\x1c\x16\xb9\xf7\x88\x68\x34\x7e\x76\xef\x63\x61\x6c\x74\x05\x63"
         "\x38\x31\xd9\x7c\x29\x51\xdd\x5c\x6f\xc4\xd9\x45\x03\x14\xe1\xb9\xbe"
         "\x21\x4a\x03\x24\x1f\x47\x31\x5e\xb5\x6e\x44\x83\x76\x1a\x49\x9a\xf3"
         "\xee\x30\xab\x8b\x86\xaf\x7c\x19\x48\x39\xe6\xb2\x65\x2d\xa3\x98\x8b"
         "\x45\xce\xe8\x66\x12\x84\xd8\x81\x05\xf7\xbc\xee\xbb\x54\xb3\x4f\x3e"
         "\xce\xe5\x47\x49\x18\x43\x6a\x14\xa6\x0a\x4b\x16\x29\x83\x55\xa1\x9a"
         "\xe5\x5f\x17\xeb\xda\xbc\xe6\xd2\x1f\x57\x56\x11\x5c\xc0\xac\xd1\xee"
         "\x9b\xa9\x39\x37\x2b\x6e\x8d\xcb\xf8\xac\xb6\x7d\x1c\x7a\xe5\x8d\x41"
         "\xdf\x8f\xc0\xa8\x25\x59\xc9\x01\xf7\x5c\x1f\xe7\x9f\x23\x04\x39\xdc"
         "\x53\x2e\xf5\x69\x26\xb4\x2d\x56\x53\x14\x20\xa6\x48\x63\x3e\xd5\x8b"
         "\x34\xec\xff\xdc\x61\xa4\x1c\xfb\x52\x09\xdf\xe3\xed\x96\x82\xf0\x69"
         "\xd7\xe6\x4a\x1c\xa4\x56\xdd\x98\x45\x84\x32\x1a\xeb\xdc\x04\x2f\x6e"
         "\x9f\xc3",
         206);
  *(uint64_t*)0x200002b8 = 0xce;
  *(uint64_t*)0x20001f18 = 4;
  *(uint64_t*)0x20001f20 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x10;
  *(uint32_t*)0x200002c8 = 0x10a;
  *(uint32_t*)0x200002cc = 2;
  *(uint64_t*)0x20001f28 = 0x10;
  *(uint32_t*)0x20001f30 = 0x4044050;
  *(uint32_t*)0x20001f38 = 0xf2;
  *(uint64_t*)0x20001f40 = 0x20000380;
  *(uint16_t*)0x20000380 = 0x18;
  *(uint32_t*)0x20000382 = 1;
  *(uint32_t*)0x20000386 = 0;
  *(uint32_t*)0x2000038a = r[0];
  *(uint32_t*)0x2000038e = 4;
  *(uint32_t*)0x20000392 = 3;
  *(uint32_t*)0x20000396 = 1;
  *(uint32_t*)0x2000039a = 3;
  *(uint16_t*)0x2000039e = 0xa;
  *(uint16_t*)0x200003a0 = htobe16(0x4e21);
  *(uint32_t*)0x200003a2 = 0x1e;
  *(uint8_t*)0x200003a6 = 0;
  *(uint8_t*)0x200003a7 = 0;
  *(uint8_t*)0x200003a8 = 0;
  *(uint8_t*)0x200003a9 = 0;
  *(uint8_t*)0x200003aa = 0;
  *(uint8_t*)0x200003ab = 0;
  *(uint8_t*)0x200003ac = 0;
  *(uint8_t*)0x200003ad = 0;
  *(uint8_t*)0x200003ae = 0;
  *(uint8_t*)0x200003af = 0;
  *(uint8_t*)0x200003b0 = 0;
  *(uint8_t*)0x200003b1 = 0;
  *(uint8_t*)0x200003b2 = 0;
  *(uint8_t*)0x200003b3 = 0;
  *(uint8_t*)0x200003b4 = 0;
  *(uint8_t*)0x200003b5 = 0;
  *(uint32_t*)0x200003b6 = 7;
  *(uint32_t*)0x20001f48 = 0x3a;
  *(uint64_t*)0x20001f50 = 0x20000580;
  *(uint64_t*)0x20000580 = 0x200003c0;
  *(uint64_t*)0x20000588 = 0;
  *(uint64_t*)0x20000590 = 0x200004c0;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x200005a0 = 0x20000500;
  *(uint64_t*)0x200005a8 = 0;
  *(uint64_t*)0x20001f58 = 3;
  *(uint64_t*)0x20001f60 = 0x200005c0;
  *(uint64_t*)0x200005c0 = 0x10;
  *(uint32_t*)0x200005c8 = 0x10c;
  *(uint32_t*)0x200005cc = 1;
  *(uint64_t*)0x200005d0 = 0x10;
  *(uint32_t*)0x200005d8 = 0x1ff;
  *(uint32_t*)0x200005dc = 5;
  *(uint64_t*)0x200005e0 = 0x10;
  *(uint32_t*)0x200005e8 = 0x3a;
  *(uint32_t*)0x200005ec = 0xffffffb4;
  *(uint64_t*)0x200005f0 = 0x10;
  *(uint32_t*)0x200005f8 = 0x11f;
  *(uint32_t*)0x200005fc = 0;
  *(uint64_t*)0x20000600 = 0x10;
  *(uint32_t*)0x20000608 = 0x29;
  *(uint32_t*)0x2000060c = 0x85;
  *(uint64_t*)0x20001f68 = 0x50;
  *(uint32_t*)0x20001f70 = 0x20000040;
  *(uint32_t*)0x20001f78 = 7;
  *(uint64_t*)0x20001f80 = 0x20001780;
  *(uint16_t*)0x20001780 = 4;
  *(uint16_t*)0x20001782 = htobe16(3);
  *(uint32_t*)0x20001784 = htobe32(8);
  memcpy((void*)0x20001788, "\xab\x05\x60\x2e\xe2\x01", 6);
  *(uint8_t*)0x2000178e = 1;
  *(uint8_t*)0x2000178f = 0;
  *(uint32_t*)0x20001f88 = 0x10;
  *(uint64_t*)0x20001f90 = 0x20001b40;
  *(uint64_t*)0x20001b40 = 0x200017c0;
  *(uint64_t*)0x20001b48 = 0;
  *(uint64_t*)0x20001b50 = 0x20001880;
  *(uint64_t*)0x20001b58 = 0;
  *(uint64_t*)0x20001b60 = 0x20001900;
  *(uint64_t*)0x20001b68 = 0;
  *(uint64_t*)0x20001b70 = 0x200019c0;
  *(uint64_t*)0x20001b78 = 0;
  *(uint64_t*)0x20001b80 = 0x20001a00;
  *(uint64_t*)0x20001b88 = 0;
  *(uint64_t*)0x20001b90 = 0x20001ac0;
  *(uint64_t*)0x20001b98 = 0;
  *(uint64_t*)0x20001f98 = 6;
  *(uint64_t*)0x20001fa0 = 0x20001bc0;
  *(uint64_t*)0x20001bc0 = 0x10;
  *(uint32_t*)0x20001bc8 = 0x119;
  *(uint32_t*)0x20001bcc = 5;
  *(uint64_t*)0x20001bd0 = 0x10;
  *(uint32_t*)0x20001bd8 = 0x11e;
  *(uint32_t*)0x20001bdc = 1;
  *(uint64_t*)0x20001be0 = 0x10;
  *(uint32_t*)0x20001be8 = 0x1ff;
  *(uint32_t*)0x20001bec = 0xffffff65;
  *(uint64_t*)0x20001bf0 = 0x10;
  *(uint32_t*)0x20001bf8 = 0x10e;
  *(uint32_t*)0x20001bfc = 0;
  *(uint64_t*)0x20001c00 = 0x10;
  *(uint32_t*)0x20001c08 = 0x10d;
  *(uint32_t*)0x20001c0c = 9;
  *(uint64_t*)0x20001c10 = 0x10;
  *(uint32_t*)0x20001c18 = 0x18a;
  *(uint32_t*)0x20001c1c = 0x370;
  *(uint64_t*)0x20001fa8 = 0x60;
  *(uint32_t*)0x20001fb0 = 0x40;
  *(uint32_t*)0x20001fb8 = 1;
  syscall(__NR_sendmmsg, r[0], 0x20001f00, 3, 0x20000080);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}