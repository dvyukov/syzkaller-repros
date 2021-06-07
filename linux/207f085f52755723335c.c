// KASAN: use-after-free Read in reiserfs_fill_super
// https://syzkaller.appspot.com/bug?id=207f085f52755723335c
// status:6
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

  memcpy((void*)0x20000000, "reiserfs\000", 9);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20000040;
  memcpy((void*)0x20000040,
         "\x00\x40\x00\x00\xec\x1f\x00\x00\x13\x20\x00\x00\x12\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x20\x00\x00\x00\x04\x00\x00\x61\x1c\xad\x49\x84\x03"
         "\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00\x00\x10\xec\x03\x02\x00\x01"
         "\x00\x52\x65\x49\x73\x45\x72\x46\x73\x00\x00\x00\x00\x03\x00\x00\x00"
         "\x02\x00\x01\x00\x00\x00\x00\x00\x01\x00\x00\x00\x03\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x17\xff\x91"
         "\x70\x37\x26\x26\x3a\xdc\x80\x33\x28\x46\x7e\x33\x20\x77\xff\xff\xff"
         "\xff\xc1\x65\x5f\x00\x4e\xed\x00",
         144);
  *(uint64_t*)0x20000208 = 0x90;
  *(uint64_t*)0x20000210 = 0x10000;
  *(uint64_t*)0x20000218 = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0x11000;
  *(uint64_t*)0x20000230 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0x11800;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 0x2012000;
  *(uint64_t*)0x20000260 = 0x200002c0;
  memcpy((void*)0x200002c0,
         "\x01\x00\x02\x00\x75\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\xe0\x0f\x00\x40\x01\x00\x00"
         "\x00\x02\x00\x00\x00\x01\x00\x00\x00\xf4\x01\x00\x00\x02\x00\x23\x00"
         "\xbd\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5c\xa1\xd9\xc8\xab\x3d"
         "\x86\x08\x81\xe1\xc8\xd7\x33\xa8\x05\x39\x8b\xbc\x93\x71\xd1\xc0\x61"
         "\x34\x97\xfc\xd1\xf7\x86\x95\x58\x25\x4b\x02\xf2\x66\x39\x0f\xb8\xa8"
         "\x8e\xaa\x67\xc9\x26\xa3\x46\x2c\xba\xca\x41\x83\xe7\xe1\xb6\x18\xa1"
         "\x67\xf7\x75\xc4\xb1\xb7\xad\x5b\xe5\xef\x63\x1a\xdd\xc3\xb3\xe8\xc8"
         "\x16\x2f\xc7\xc6\xce\x83\x7d\x97\x08\xe4\xe9\x14\x3f\xc2\x42\xd5\x68"
         "\x6c\x23\xe1\x41\xd3\x54\xfd\xdd\x35\xa3\x32\x1f\x30\xc8\xd8\xc1\x8f"
         "\x59\x96\x9f\x76\x49\xcd\xa0\x45\x75\x32\xe1\xdb\x0d\xcf",
         218);
  *(uint64_t*)0x20000268 = 0xda;
  *(uint64_t*)0x20000270 = 0x2013000;
  *(uint64_t*)0x20000278 = 0;
  *(uint64_t*)0x20000280 = 0;
  *(uint64_t*)0x20000288 = 0x2013fa0;
  *(uint8_t*)0x20011100 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0x4000000, 6, 0x20000200, 0,
                  0x20011100);
  return 0;
}
