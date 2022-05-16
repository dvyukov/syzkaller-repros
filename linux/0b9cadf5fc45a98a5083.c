// possible deadlock in f2fs_write_checkpoint
// https://syzkaller.appspot.com/bug?id=0b9cadf5fc45a98a5083
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/loop.h>

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

static unsigned long long procid;

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

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

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void setup_binderfs()
{
  if (mkdir("/dev/binderfs", 0777)) {
  }
  if (mount("binder", "/dev/binderfs", "binder", 0, NULL)) {
  }
  if (symlink("/dev/binderfs", "./binderfs")) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

static int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  setup_binderfs();
  loop();
  exit(1);
}

void loop(void)
{
  memcpy((void*)0x20000000, "f2fs\000", 5);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000,
         "\x10\x20\xf5\xf2\x01\x00\x0b\x00\x09\x00\x00\x00\x03\x00\x00\x00\x0c"
         "\x00\x00\x00\x09\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x18\x00\x00\x00\x1f\x00\x00"
         "\x00\x02\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00"
         "\x18\x00\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x06\x00\x00\x00"
         "\x0a\x00\x00\x00\x0e\x00\x00\x00\x10\x00\x00\x03\x00\x00\x00\x01\x00"
         "\x00\x00\x02\x00\x00\x00\x1e\x33\x03\x65\x71\xef\x4c\x6d\xa1\x85\x41"
         "\xc4\xcb\x7e\x0f\x42\x00\x00\x00\x00",
         128);
  *(uint64_t*)0x20000208 = 0x80;
  *(uint64_t*)0x20000210 = 0x400;
  *(uint64_t*)0x20000218 = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0x860;
  *(uint64_t*)0x20000230 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0xa80;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 0xb80;
  *(uint64_t*)0x20000260 = 0x20010500;
  memcpy((void*)0x20010500,
         "\000\000\000\000\b\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
         32);
  *(uint64_t*)0x20000268 = 0x20;
  *(uint64_t*)0x20000270 = 0xc80;
  *(uint64_t*)0x20000278 = 0;
  *(uint64_t*)0x20000280 = 0;
  *(uint64_t*)0x20000288 = 0xec0;
  *(uint64_t*)0x20000290 = 0;
  *(uint64_t*)0x20000298 = 0;
  *(uint64_t*)0x200002a0 = 0x1400;
  *(uint64_t*)0x200002a8 = 0;
  *(uint64_t*)0x200002b0 = 0;
  *(uint64_t*)0x200002b8 = 0x1860;
  *(uint64_t*)0x200002c0 = 0;
  *(uint64_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0x1a80;
  *(uint64_t*)0x200002d8 = 0;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0x1b80;
  *(uint64_t*)0x200002f0 = 0;
  *(uint64_t*)0x200002f8 = 0;
  *(uint64_t*)0x20000300 = 0x1c80;
  *(uint64_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0;
  *(uint64_t*)0x20000318 = 0x1ec0;
  *(uint64_t*)0x20000320 = 0x20010e00;
  memcpy(
      (void*)0x20010e00,
      "\x51\x35\x21\x15\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x0b\x00"
      "\x00\x00\x00\x00\x00\x00\x0d\x00\x00\x00\x10\x00\x00\x00\x12\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\xff\xff\xff\xff\xff\xff"
      "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x03\x00\x06\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x0b\x00"
      "\x00\x00\x05\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
      "\xff\xff\xff\xff\xff\xff\xff\xff\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xc5\x01\x00\x00\x06\x00\x00\x00\x01\x00\x00\x00"
      "\x07\x00\x00\x00\x07\x00\x00\x00\x0a\x00\x00\x00\x40\x00\x00\x00\x40\x00"
      "\x00\x00\xfc\x0f\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      192);
  *(uint64_t*)0x20000328 = 0xc0;
  *(uint64_t*)0x20000330 = 0x200000;
  *(uint64_t*)0x20000338 = 0x20010f00;
  memcpy((void*)0x20010f00,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x2c\x1b\x39\xb8\x00\x00"
         "\x03\x00\x00\x00\x00\x03\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20000340 = 0x40;
  *(uint64_t*)0x20000348 = 0x200fe0;
  *(uint64_t*)0x20000350 = 0x20011000;
  memcpy(
      (void*)0x20011000,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x02\x0c\x60"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01"
      "\x00\x00\x00\x05\x10\xec\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x14\x00",
      192);
  *(uint64_t*)0x20000358 = 0xc0;
  *(uint64_t*)0x20000360 = 0x2011e0;
  *(uint64_t*)0x20000368 = 0x20011100;
  memcpy((void*)0x20011100,
         "\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x04\x00\x78\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
         "\x05\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00",
         96);
  *(uint64_t*)0x20000370 = 0x60;
  *(uint64_t*)0x20000378 = 0x2012e0;
  *(uint64_t*)0x20000380 = 0x20011200;
  memcpy((void*)0x20011200,
         "\x00\x00\x00\x0b\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint64_t*)0x20000388 = 0x20;
  *(uint64_t*)0x20000390 = 0x201380;
  *(uint64_t*)0x20000398 = 0;
  *(uint64_t*)0x200003a0 = 0;
  *(uint64_t*)0x200003a8 = 0x2013e0;
  *(uint64_t*)0x200003b0 = 0;
  *(uint64_t*)0x200003b8 = 0;
  *(uint64_t*)0x200003c0 = 0x202000;
  *(uint64_t*)0x200003c8 = 0;
  *(uint64_t*)0x200003d0 = 0;
  *(uint64_t*)0x200003d8 = 0x202e00;
  *(uint64_t*)0x200003e0 = 0;
  *(uint64_t*)0x200003e8 = 0;
  *(uint64_t*)0x200003f0 = 0x202fe0;
  *(uint64_t*)0x200003f8 = 0;
  *(uint64_t*)0x20000400 = 0;
  *(uint64_t*)0x20000408 = 0x203fe0;
  *(uint64_t*)0x20000410 = 0x20011800;
  memcpy(
      (void*)0x20011800,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x51\x35\x21\x15"
      "\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x0b\x00\x00\x00\x00\x00"
      "\x00\x00\x0d\x00\x00\x00\x10\x00\x00\x00\x12\x00\x00\x00\x00\x00\x00\x00"
      "\x01\x00\x00\x00\x02\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
      "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x03\x00\x06\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x0b\x00\x00\x00\x05\x00"
      "\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
      "\xff\xff\xff\xff\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xc5\x01\x00\x00\x06\x00\x00\x00\x01\x00\x00\x00\x07\x00\x00\x00"
      "\x07\x00\x00\x00\x0a\x00\x00\x00\x40\x00\x00\x00\x40\x00\x00\x00\xfc\x0f"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00",
      224);
  *(uint64_t*)0x20000418 = 0xe0;
  *(uint64_t*)0x20000420 = 0x204fe0;
  *(uint64_t*)0x20000428 = 0x20011900;
  memcpy((void*)0x20011900,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x2c\x1b\x39\xb8",
         32);
  *(uint64_t*)0x20000430 = 0x20;
  *(uint64_t*)0x20000438 = 0x205fe0;
  *(uint64_t*)0x20000440 = 0;
  *(uint64_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x3ff000;
  *(uint64_t*)0x20000458 = 0;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0x3ff040;
  *(uint64_t*)0x20000470 = 0;
  *(uint64_t*)0x20000478 = 0;
  *(uint64_t*)0x20000480 = 0x400000;
  *(uint64_t*)0x20000488 = 0;
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0x400fe0;
  *(uint64_t*)0x200004a0 = 0;
  *(uint64_t*)0x200004a8 = 0;
  *(uint64_t*)0x200004b0 = 0x4011e0;
  *(uint64_t*)0x200004b8 = 0;
  *(uint64_t*)0x200004c0 = 0;
  *(uint64_t*)0x200004c8 = 0x4012e0;
  *(uint64_t*)0x200004d0 = 0;
  *(uint64_t*)0x200004d8 = 0;
  *(uint64_t*)0x200004e0 = 0x401380;
  *(uint64_t*)0x200004e8 = 0;
  *(uint64_t*)0x200004f0 = 0;
  *(uint64_t*)0x200004f8 = 0x4013e0;
  *(uint64_t*)0x20000500 = 0;
  *(uint64_t*)0x20000508 = 0;
  *(uint64_t*)0x20000510 = 0x402000;
  *(uint64_t*)0x20000518 = 0;
  *(uint64_t*)0x20000520 = 0;
  *(uint64_t*)0x20000528 = 0x402e00;
  *(uint64_t*)0x20000530 = 0;
  *(uint64_t*)0x20000538 = 0;
  *(uint64_t*)0x20000540 = 0x402fe0;
  *(uint64_t*)0x20000548 = 0;
  *(uint64_t*)0x20000550 = 0;
  *(uint64_t*)0x20000558 = 0x403fe0;
  *(uint64_t*)0x20000560 = 0;
  *(uint64_t*)0x20000568 = 0;
  *(uint64_t*)0x20000570 = 0x404fe0;
  *(uint64_t*)0x20000578 = 0;
  *(uint64_t*)0x20000580 = 0;
  *(uint64_t*)0x20000588 = 0x405fe0;
  *(uint64_t*)0x20000590 = 0;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x200005a0 = 0x5ff000;
  *(uint64_t*)0x200005a8 = 0;
  *(uint64_t*)0x200005b0 = 0;
  *(uint64_t*)0x200005b8 = 0x5ff040;
  *(uint64_t*)0x200005c0 = 0x20012a00;
  memcpy((void*)0x20012a00,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x00\x03\x00\x00\x00\x01\x10"
         "\x00\x00\x00\x04\x00\x00\x00\x02\x10\x00\x00\x00\x05\x00\x00\x00\x00"
         "\x12\x00\x00\x00\x06\x00\x00\x00\x01\x12\x00\x00\x00\x07\x00\x00\x00"
         "\x02\x12\x00\x00\x00\x08\x00\x00\x00\x05\x12\x00\x00\x00\x09\x00\x00"
         "\x00\x04\x12\x00\x00\x00\x00\x00\x00\x00\x00",
         96);
  *(uint64_t*)0x200005c8 = 0x60;
  *(uint64_t*)0x200005d0 = 0xa00000;
  *(uint64_t*)0x200005d8 = 0;
  *(uint64_t*)0x200005e0 = 0;
  *(uint64_t*)0x200005e8 = 0xc00000;
  *(uint64_t*)0x200005f0 = 0x20012c00;
  memcpy((void*)0x20012c00,
         "\xed\x41\x00\x20\x5c\xf9\x01\x00\x53\x5f\x01\x00\x03\x00\x00\x00\x00"
         "\x10\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x0f\xbc"
         "\x64\x5f\x00\x00\x00\x00\x12\xbc\x64\x5f\x00\x00\x00\x00\x12\xbc\x64"
         "\x5f\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x6b\xc3\x19\x0f\x6b\xc3\x19"
         "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         96);
  *(uint64_t*)0x200005f8 = 0x60;
  *(uint64_t*)0x20000600 = 0x1001000;
  *(uint64_t*)0x20000608 = 0;
  *(uint64_t*)0x20000610 = 0;
  *(uint64_t*)0x20000618 = 0x1001160;
  *(uint64_t*)0x20000620 = 0x20012e00;
  memcpy(
      (void*)0x20012e00,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x03\x00\x00\x00\x00\x00"
      "\x00\x00\x4d\x35\x21\x15\x00\x00\x00\x00\x02\x10\x00\x00\xed\x41\x00\x25"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x7c\x0d\x00\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x12\xbc\x64\x5f\x00\x00\x00\x00"
      "\x12\xbc\x64\x5f\x00\x00\x00\x00\x12\xbc\x64\x5f\x00\x00\x00\x00\x0f\x6b"
      "\xc3\x19\x0f\x6b\xc3\x19\x0f\x6b\xc3\x19\x3c\x5f\x6a\xef\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x10\x00\x00\x03\x00\x00\x00\x05\x00\x00\x00\x66\x69"
      "\x6c\x65\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      160);
  *(uint64_t*)0x20000628 = 0xa0;
  *(uint64_t*)0x20000630 = 0x1001fe0;
  *(uint64_t*)0x20000638 = 0;
  *(uint64_t*)0x20000640 = 0;
  *(uint64_t*)0x20000648 = 0x1002160;
  *(uint64_t*)0x20000650 = 0;
  *(uint64_t*)0x20000658 = 0;
  *(uint64_t*)0x20000660 = 0x1002960;
  *(uint64_t*)0x20000668 = 0;
  *(uint64_t*)0x20000670 = 0;
  *(uint64_t*)0x20000678 = 0x1002fe0;
  *(uint64_t*)0x20000680 = 0;
  *(uint64_t*)0x20000688 = 0;
  *(uint64_t*)0x20000690 = 0x1200000;
  *(uint64_t*)0x20000698 = 0;
  *(uint64_t*)0x200006a0 = 0;
  *(uint64_t*)0x200006a8 = 0x1200160;
  *(uint64_t*)0x200006b0 = 0;
  *(uint64_t*)0x200006b8 = 0;
  *(uint64_t*)0x200006c0 = 0x1200fe0;
  *(uint64_t*)0x200006c8 = 0;
  *(uint64_t*)0x200006d0 = 0;
  *(uint64_t*)0x200006d8 = 0x1201160;
  *(uint64_t*)0x200006e0 = 0;
  *(uint64_t*)0x200006e8 = 0;
  *(uint64_t*)0x200006f0 = 0x1201fe0;
  *(uint64_t*)0x200006f8 = 0;
  *(uint64_t*)0x20000700 = 0;
  *(uint64_t*)0x20000708 = 0x1202160;
  *(uint64_t*)0x20000710 = 0;
  *(uint64_t*)0x20000718 = 0;
  *(uint64_t*)0x20000720 = 0x1202f00;
  *(uint64_t*)0x20000728 = 0;
  *(uint64_t*)0x20000730 = 0;
  *(uint64_t*)0x20000738 = 0x1202fe0;
  *(uint64_t*)0x20000740 = 0;
  *(uint64_t*)0x20000748 = 0;
  *(uint64_t*)0x20000750 = 0x1204000;
  *(uint64_t*)0x20000758 = 0;
  *(uint64_t*)0x20000760 = 0;
  *(uint64_t*)0x20000768 = 0x1204160;
  *(uint64_t*)0x20000770 = 0;
  *(uint64_t*)0x20000778 = 0;
  *(uint64_t*)0x20000780 = 0x1204fe0;
  *(uint64_t*)0x20000788 = 0;
  *(uint64_t*)0x20000790 = 0;
  *(uint64_t*)0x20000798 = 0x1205160;
  *(uint64_t*)0x200007a0 = 0;
  *(uint64_t*)0x200007a8 = 0;
  *(uint64_t*)0x200007b0 = 0x1205fe0;
  *(uint64_t*)0x200007b8 = 0;
  *(uint64_t*)0x200007c0 = 0;
  *(uint64_t*)0x200007c8 = 0x1601000;
  *(uint64_t*)0x200007d0 = 0;
  *(uint64_t*)0x200007d8 = 0;
  *(uint64_t*)0x200007e0 = 0x1601940;
  *(uint8_t*)0x20014500 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0x4000000, 0x3f, 0x20000200, 0,
                  0x20014500);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
