// kernel BUG in iput
// https://syzkaller.appspot.com/bug?id=2cfd2a4fc18e40303fb2
// status:6
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
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
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/loop.h>

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

static unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
}

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

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  int iter = 0;
  DIR* dp = 0;
retry:
  while (umount2(dir, MNT_DETACH | UMOUNT_NOFOLLOW) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  struct dirent* ep = 0;
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH | UMOUNT_NOFOLLOW) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH | UMOUNT_NOFOLLOW))
        exit(1);
    }
  }
  closedir(dp);
  for (int i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH | UMOUNT_NOFOLLOW))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
}

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void reset_loop()
{
  char buf[64];
  snprintf(buf, sizeof(buf), "/dev/loop%llu", procid);
  int loopfd = open(buf, O_RDWR);
  if (loopfd != -1) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
  if (symlink("/dev/binderfs", "./binderfs")) {
  }
}

static void setup_fault()
{
  static struct {
    const char* file;
    const char* val;
    bool fatal;
  } files[] = {
      {"/sys/kernel/debug/failslab/ignore-gfp-wait", "N", true},
      {"/sys/kernel/debug/fail_futex/ignore-private", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/min-order", "0", false},
  };
  unsigned i;
  for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].file, files[i].val)) {
      if (files[i].fatal)
        exit(1);
    }
  }
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    reset_loop();
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
    remove_dir(cwdbuf);
  }
}

void execute_one(void)
{
  memcpy((void*)0x20000000, "exfat\000", 6);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000,
         "\xeb\x76\x90\x45\x58\x46\x41\x54\x20\x20\x20\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x08"
         "\x00\x00\x00\x88\x00\x00\x00\x0f\x00\x00\x00\x05\x00\x00\x00\x34\x12"
         "\x34\x12\x00\x01\x00\x00\x09\x03\x01\x80",
         112);
  *(uint64_t*)0x20000208 = 0x70;
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
  *(uint64_t*)0x200002c0 = 0x20010900;
  memcpy((void*)0x20010900,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x200002c8 = 0x20;
  *(uint64_t*)0x200002d0 = 0x11e0;
  *(uint64_t*)0x200002d8 = 0x20010a00;
  memcpy(
      (void*)0x20010a00,
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab"
      "\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11\xd0\xab\x19\x11"
      "\xd0\xab\x19\x11\xd0\xab\x19\x11",
      512);
  *(uint64_t*)0x200002e0 = 0x200;
  *(uint64_t*)0x200002e8 = 0x1600;
  *(uint64_t*)0x200002f0 = 0x20011300;
  memcpy((void*)0x20011300,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa",
         32);
  *(uint64_t*)0x200002f8 = 0x20;
  *(uint64_t*)0x20000300 = 0x80;
  *(uint64_t*)0x20000308 = 0x20011800;
  memcpy((void*)0x20011800,
         "\xf8\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x04\x00\x00\x00\xff"
         "\xff\xff\xff\xff\xff\xff\xff",
         24);
  *(uint64_t*)0x20000310 = 0x18;
  *(uint64_t*)0x20000318 = 0x10000;
  *(uint64_t*)0x20000320 = 0x20013100;
  memcpy((void*)0x20013100,
         "\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x81\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x02\x00\x00\x00\x02",
         57);
  *(uint64_t*)0x20000328 = 0x39;
  *(uint64_t*)0x20000330 = 0x14000;
  *(uint8_t*)0x20013b00 = 0;
  inject_fault(98);
  syz_mount_image(0x20000000, 0x20000100, 0xa400, 0xd, 0x20000200, 0,
                  0x20013b00);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_fault();
  use_temporary_dir();
  loop();
  return 0;
}
