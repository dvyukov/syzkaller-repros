// KASAN: slab-out-of-bounds Read in exfat_clear_bitmap
// https://syzkaller.appspot.com/bug?id=a4087e40b9c13aad7892
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/loop.h>

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

static unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

static unsigned long fs_image_segment_check(unsigned long size,
                                            unsigned long nsegs,
                                            struct fs_image_segment* segs)
{
  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (size_t i = 0; i < nsegs; i++) {
    if (segs[i].size > IMAGE_MAX_SIZE)
      segs[i].size = IMAGE_MAX_SIZE;
    segs[i].offset %= IMAGE_MAX_SIZE;
    if (segs[i].offset > IMAGE_MAX_SIZE - segs[i].size)
      segs[i].offset = IMAGE_MAX_SIZE - segs[i].size;
    if (size < segs[i].offset + segs[i].offset)
      size = segs[i].offset + segs[i].offset;
  }
  if (size > IMAGE_MAX_SIZE)
    size = IMAGE_MAX_SIZE;
  return size;
}
static int setup_loop_device(long unsigned size, long unsigned nsegs,
                             struct fs_image_segment* segs,
                             const char* loopname, int* memfd_p, int* loopfd_p)
{
  int err = 0, loopfd = -1;
  size = fs_image_segment_check(size, nsegs, segs);
  int memfd = syscall(__NR_memfd_create, "syzkaller", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (size_t i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
  }
  loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    err = errno;
    goto error_close_memfd;
  }
  if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
    if (errno != EBUSY) {
      err = errno;
      goto error_close_loop;
    }
    ioctl(loopfd, LOOP_CLR_FD, 0);
    usleep(1000);
    if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
      err = errno;
      goto error_close_loop;
    }
  }
  *memfd_p = memfd;
  *loopfd_p = loopfd;
  return 0;

error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return -1;
}

static long syz_mount_image(volatile long fsarg, volatile long dir,
                            volatile unsigned long size,
                            volatile unsigned long nsegs,
                            volatile long segments, volatile long flags,
                            volatile long optsarg)
{
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;
  int res = -1, err = 0, loopfd = -1, memfd = -1, need_loop_device = !!segs;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(size, nsegs, segs, loopname, &memfd, &loopfd) == -1)
      return -1;
    source = loopname;
  }
  mkdir(target, 0777);
  char opts[256];
  memset(opts, 0, sizeof(opts));
  if (strlen(mount_opts) > (sizeof(opts) - 32)) {
  }
  strncpy(opts, mount_opts, sizeof(opts) - 32);
  if (strcmp(fs, "iso9660") == 0) {
    flags |= MS_RDONLY;
  } else if (strncmp(fs, "ext", 3) == 0) {
    if (strstr(opts, "errors=panic") || strstr(opts, "errors=remount-ro") == 0)
      strcat(opts, ",errors=continue");
  } else if (strcmp(fs, "xfs") == 0) {
    strcat(opts, ",nouuid");
  }
  res = mount(source, target, fs, flags, opts);
  if (res == -1) {
    err = errno;
    goto error_clear_loop;
  }
  res = open(target, O_RDONLY | O_DIRECTORY);
  if (res == -1) {
    err = errno;
  }

error_clear_loop:
  if (need_loop_device) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
    close(memfd);
  }
  errno = err;
  return res;
}

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  memcpy((void*)0x20000000, "exfat\000", 6);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000,
         "\xeb\x76\x90\x45\x58\x46\x41\x54\x20\x20\x20\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x08"
         "\x00\x00\x00\x88\x00\x00\x00\x0f\x00\x00\x00\x05\x00\x00\x00\x34\x12"
         "\x34\x12\x00\x01\x00\x00\x09\x03\x01\x80\x49\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         128);
  *(uint64_t*)0x20000208 = 0x80;
  *(uint64_t*)0x20000210 = 0;
  *(uint64_t*)0x20000218 = 0x20010100;
  memcpy((void*)0x20010100,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x20000220 = 0x20;
  *(uint64_t*)0x20000228 = 0x1e0;
  *(uint64_t*)0x20000230 = 0x20010200;
  memcpy((void*)0x20010200,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x20000238 = 0x20;
  *(uint64_t*)0x20000240 = 0x3e0;
  *(uint64_t*)0x20000248 = 0x20010300;
  memcpy((void*)0x20010300,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x20000250 = 0x20;
  *(uint64_t*)0x20000258 = 0x5e0;
  *(uint64_t*)0x20000260 = 0x20010400;
  memcpy((void*)0x20010400,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x20000268 = 0x20;
  *(uint64_t*)0x20000270 = 0x7e0;
  *(uint64_t*)0x20000278 = 0x20010500;
  memcpy((void*)0x20010500,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x20000280 = 0x20;
  *(uint64_t*)0x20000288 = 0x9e0;
  *(uint64_t*)0x20000290 = 0x20010600;
  memcpy((void*)0x20010600,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x20000298 = 0x20;
  *(uint64_t*)0x200002a0 = 0xbe0;
  *(uint64_t*)0x200002a8 = 0x20010700;
  memcpy((void*)0x20010700,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x200002b0 = 0x20;
  *(uint64_t*)0x200002b8 = 0xde0;
  *(uint64_t*)0x200002c0 = 0x20010800;
  memcpy((void*)0x20010800,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x200002c8 = 0x20;
  *(uint64_t*)0x200002d0 = 0xfe0;
  *(uint64_t*)0x200002d8 = 0x20010900;
  memcpy((void*)0x20010900,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x200002e0 = 0x20;
  *(uint64_t*)0x200002e8 = 0x11e0;
  *(uint64_t*)0x200002f0 = 0x20010a00;
  memcpy(
      (void*)0x20010a00,
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac"
      "\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11\x00\xac\x19\x11"
      "\x00\xac\x19\x11\x00\xac\x19\x11\xeb\x76\x90\x45\x58\x46\x41\x54\x20\x20"
      "\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x03\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x80\x00"
      "\x00\x00\x08\x00\x00\x00\x88\x00\x00\x00\x0f\x00\x00\x00\x05\x00\x00\x00"
      "\x34\x12\x34\x12\x00\x01\x00\x00\x09\x03\x01\x80\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      640);
  *(uint64_t*)0x200002f8 = 0x280;
  *(uint64_t*)0x20000300 = 0x1600;
  *(uint64_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0;
  *(uint64_t*)0x20000318 = 0x19e0;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint64_t*)0x20000330 = 0x1be0;
  *(uint64_t*)0x20000338 = 0;
  *(uint64_t*)0x20000340 = 0;
  *(uint64_t*)0x20000348 = 0x1de0;
  *(uint64_t*)0x20000350 = 0;
  *(uint64_t*)0x20000358 = 0;
  *(uint64_t*)0x20000360 = 0x1fe0;
  *(uint64_t*)0x20000368 = 0;
  *(uint64_t*)0x20000370 = 0;
  *(uint64_t*)0x20000378 = 0x21e0;
  *(uint64_t*)0x20000380 = 0;
  *(uint64_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x23e0;
  *(uint64_t*)0x20000398 = 0;
  *(uint64_t*)0x200003a0 = 0;
  *(uint64_t*)0x200003a8 = 0x25e0;
  *(uint64_t*)0x200003b0 = 0;
  *(uint64_t*)0x200003b8 = 0;
  *(uint64_t*)0x200003c0 = 0x27e0;
  *(uint64_t*)0x200003c8 = 0;
  *(uint64_t*)0x200003d0 = 0;
  *(uint64_t*)0x200003d8 = 0x29e0;
  *(uint64_t*)0x200003e0 = 0;
  *(uint64_t*)0x200003e8 = 0;
  *(uint64_t*)0x200003f0 = 0x2e00;
  *(uint64_t*)0x200003f8 = 0x20011800;
  memcpy((void*)0x20011800,
         "\xf8\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x04\x00\x00\x00\xff"
         "\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x0a\x00\x00\x00\x0b\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20000400 = 0x40;
  *(uint64_t*)0x20000408 = 0x10000;
  *(uint64_t*)0x20000410 = 0;
  *(uint64_t*)0x20000418 = 0;
  *(uint64_t*)0x20000420 = 0x11000;
  *(uint64_t*)0x20000428 = 0;
  *(uint64_t*)0x20000430 = 0;
  *(uint64_t*)0x20000438 = 0x12000;
  *(uint64_t*)0x20000440 = 0x20013100;
  memcpy(
      (void*)0x20013100,
      "\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x81\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00"
      "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x82\x00\x00\x00\x0d\xd3\x19\xe6"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\xcc\x16"
      "\x00\x00\x00\x00\x00\x00\x85\x02\x67\xa4\x10\x00\x00\x00\x65\x81\x32\x51"
      "\x65\x81\x32\x51\x65\x81\x32\x51\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xc0\x03\x00\x05\x23\xb5\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x06\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\xc1\x00"
      "\x66\x00\x69\x00\x6c\x00\x65\x00\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x85\x02\xc7\x36\x20\x00"
      "\x00\x00\x65\x81\x32\x51\x65\x81\x32\x51\x65\x81\x32\x51\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\xc0\x03\x00\x05\x24\x35\x00\x00\x0a\x00"
      "\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x0a\x00\x00\x00"
      "\x00\x00\x00\x00\xc1\x00\x66\x00\x69\x00\x6c\x00\x65\x00\x31\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x85\x02\xe0\xee\x20\x00\x00\x00\x65\x81\x32\x51\x65\x81\x32\x51\x65\x81"
      "\x32\x51\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\x01\x00\x05"
      "\x24\xb5\x00\x00\x28\x23\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x09\x00"
      "\x00\x00\x28\x23\x00\x00\x00\x00\x00\x00\xc1\x00\x66\x00\x69\x00\x6c\x00"
      "\x65\x00\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x85\x02\x64\x9e\x20\x00\x00\x00\x65\x81\x32\x51"
      "\x65\x81\x32\x51\x65\x81\x32\x51\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xc0\x03\x00\x09\xe3\xa0\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x0c\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\xc1\x00"
      "\x66\x00\x69\x00\x6c\x00\x65\x00\x2e\x00\x63\x00\x6f\x00\x6c\x00\x64\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      480);
  *(uint64_t*)0x20000448 = 0x1e0;
  *(uint64_t*)0x20000450 = 0x14000;
  *(uint64_t*)0x20000458 = 0;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0x15000;
  *(uint64_t*)0x20000470 = 0;
  *(uint64_t*)0x20000478 = 0;
  *(uint64_t*)0x20000480 = 0x16000;
  *(uint64_t*)0x20000488 = 0;
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0x17004;
  *(uint64_t*)0x200004a0 = 0;
  *(uint64_t*)0x200004a8 = 0;
  *(uint64_t*)0x200004b0 = 0x1b000;
  syz_mount_image(0x20000000, 0x20000100, 0x20000, 0x1d, 0x20000200, 0,
                  0x20013b00);
  memcpy((void*)0x20000040, "./file0\000", 8);
  syscall(__NR_chdir, 0x20000040ul);
  memcpy((void*)0x20000080, "./file1\000", 8);
  syscall(__NR_creat, 0x20000080ul, 0ul);
  return 0;
}