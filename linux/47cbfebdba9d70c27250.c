// kernel BUG at fs/ntfs/aops.c:LINE!
// https://syzkaller.appspot.com/bug?id=47cbfebdba9d70c27250
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

static unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define sys_memfd_create 319

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
  int memfd = syscall(sys_memfd_create, "syzkaller", 0);
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

  memcpy((void*)0x20000000, "ntfs\000", 5);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000,
         "\xeb\x52\x90\x4e\x54\x46\x53\x20\x20\x20\x20\x00\x02\x08\x00\x00\x00"
         "\x00\x00\x00\x00\xf8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x80\x00\x80\x00\xff\x0f\x00\x00\x00\x00\x00\x00\x04\x00\x00"
         "\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xf6\x00\x00\x00"
         "\x01\x00\x00\x00\x3d\xaa\xf5\x5a\xf9\x83\x09\x09\x00\x00\x00\x00\x0e"
         "\x1f\xbe\x71\x7c\xac\x22\xc0\x74\x0b\x56\xb4\x0e\xbb\x07\x00\xcd\x10"
         "\x5e\xeb\xf0\x32\xe4\xcd\x16\xcd\x19\xeb\xfe\x54\x68\x69\x73\x20\x69"
         "\x73\x20\x6e\x6f\x74\x20\x61\x20\x62\x6f\x6f\x74\x61\x62\x6c\x65\x20"
         "\x64\x69\x73\x6b\x2e\x20\x50\x6c\x65\x61\x73\x65\x20\x69\x6e\x73\x65"
         "\x72\x74\x20\x61\x20\x62\x6f\x6f\x74\x61\x62\x6c\x65\x20\x66\x6c\x6f"
         "\x70\x70\x79\x20\x61\x6e\x64\x0d\x0a\x70\x72\x65\x73\x73\x20\x61\x6e"
         "\x79\x20\x6b\x65\x79\x20\x74\x6f\x20\x74\x72\x79\x20\x61\x67\x61\x69"
         "\x6e\x20\x2e\x2e\x2e\x20\x0d\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00",
         224);
  *(uint64_t*)0x20000208 = 0xe0;
  *(uint64_t*)0x20000210 = 0;
  *(uint64_t*)0x20000218 = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0x1e0;
  *(uint64_t*)0x20000230 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0x2000;
  *(uint64_t*)0x20000248 = 0x20010300;
  memcpy((void*)0x20010300,
         "\x46\x49\x4c\x45\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
         "\x00\x01\x00\x38\x00\x01\x00\x98\x01\x00\x00\x00\x04\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00"
         "\x00\x00\x00\x00\x00\x10\x00\x00\x00\x60\x00\x00\x00\x00\x00\x18\x00"
         "\x00\x00\x00\x00\x48\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x30"
         "\x00\x00\x00\x68\x00\x00\x00\x00\x00\x18\x00\x00\x00\x02\x00\x4a\x00"
         "\x00\x00\x18\x00\x01\x00\x05\x00\x00\x00\x00\x00\x05\x00\x00\x6f\x95"
         "\xff\xc7\x8d\xd6\x01\x00\x6f\x95\xff\xc7\x8d\xd6\x01\x00\x6f\x95\xff"
         "\xc7\x8d\xd6\x01\x00\x6f\x95\xff\xc7\x8d\xd6\x01\x00\x70\x00\x00\x00"
         "\x00\x00\x00\x00\x6c\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00"
         "\x00\x00\x04\x03\x24\x00\x4d\x00\x46\x00\x54\x00\x00\x00\x00\x00\x00"
         "\x00\x80\x00\x00\x00\x48\x00\x00\x00\x01\x00\x40\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x12\x00\x00\x00\x00\x00\x00\x00\x40"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x30\x01\x00\x00\x00\x00\x00\x00\x18"
         "\x01\x00\x00\x00\x00\x00",
         312);
  *(uint64_t*)0x20000250 = 0x138;
  *(uint64_t*)0x20000258 = 0x4000;
  *(uint64_t*)0x20000260 = 0x20010500;
  memcpy((void*)0x20010500, "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\b",
         31);
  *(uint64_t*)0x20000268 = 0x1f;
  *(uint64_t*)0x20000270 = 0x41e0;
  *(uint64_t*)0x20000278 = 0x20010600;
  memcpy((void*)0x20010600,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x46\x49"
         "\x4c\x45\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x01"
         "\x00\x38\x00\x01\x00\x58\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x03\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x00"
         "\x00\x00\x00\x10\x00\x00\x00\x60\x00\x00\x00\x00\x00\x18\x00\x00\x00"
         "\x00\x00\x48\x00\x00\x00\x18\x00\x00\x00\x00\x6f\x95\xff\xc7\x8d\xd6"
         "\x01\x00\x6f\x95\xff\xc7\x8d\xd6\x01\x00\x6f\x95\xff\xc7\x8d\xd6\x01"
         "\x00\x6f\x95\xff\xc7\x8d\xd6\x01\x06\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x30\x00\x00"
         "\x00\x70\x00\x00\x00\x00\x00\x18\x00\x00\x00\x02\x00\x52\x00\x00\x00"
         "\x18\x00\x01\x00\x05\x00\x00\x00\x00\x00\x05\x00\x00\x6f\x95\xff\xc7"
         "\x8d\xd6\x01\x00\x6f\x95\xff\xc7\x8d\xd6\x01\x00\x6f\x95\xff\xc7\x8d"
         "\xd6\x01\x00\x6f\x95\xff\xc7\x8d\xd6\x01\x00\x10\x00\x00\x00\x00\x00"
         "\x00\x00\x10\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00"
         "\x08\x03\x24\x00\x4d\x00\x46\x00\x54\x00\x4d\x00\x69\x00\x72\x00\x72"
         "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x48\x00\x00\x00\x01\x00"
         "\x40\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00"
         "\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00"
         "\x00\x00\x00\x21\x01\xff",
         363);
  *(uint64_t*)0x20000280 = 0x16b;
  *(uint64_t*)0x20000288 = 0x43e0;
  *(uint64_t*)0x20000290 = 0;
  *(uint64_t*)0x20000298 = 0;
  *(uint64_t*)0x200002a0 = 0x45e0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint64_t*)0x200002b0 = 0;
  *(uint64_t*)0x200002b8 = 0x47e0;
  *(uint8_t*)0x2007dc00 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0, 8, 0x20000200, 0, 0x2007dc00);
  return 0;
}
