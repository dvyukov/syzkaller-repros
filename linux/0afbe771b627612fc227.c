// BUG: Dentry still in use [unmount of erofs loop0]
// https://syzkaller.appspot.com/bug?id=0afbe771b627612fc227
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

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
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
  loop();
  exit(1);
}

void loop(void)
{
  memcpy((void*)0x20000000, "erofs\000", 6);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy(
      (void*)0x20010000,
      "\xe2\xe1\xf5\xe0\xb9\x82\xa3\x8e\x01\x00\x00\x00\x0c\x00\x24\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xe8\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb0\x5f\x46\x92\xa6"
      "\x4c\xad\x96\x45\x2d\xd7\x0d\x29\xf8\x2d\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x05\x00\x00\x00\xc0\x41\x00\x00\x74\x00\x00\x00\x00\x00\x00\x00"
      "\xff\xff\xff\xff\x00\x00\x00\x00\x5c\xf9\x00\x00\x53\x5f\x00\x00\xe8\x03"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x24\x00\x00\x00\x00\x00"
      "\x00\x00\x54\x00\x02\x00\x24\x00\x00\x00\x00\x00\x00\x00\x55\x00\x02\x00"
      "\x2a\x00\x00\x00\x00\x00\x00\x00\x57\x00\x01\x00\x30\x00\x00\x00\x00\x00"
      "\x00\x00\x60\x00\x02\x00\x5b\x00\x00\x00\x00\x00\x00\x00\x65\x00\x01\x00"
      "\x5f\x00\x00\x00\x00\x00\x00\x00\x6a\x00\x01\x00\x5f\x00\x00\x00\x00\x00"
      "\x00\x00\x6f\x00\x01\x00\x2e\x2e\x2e\x66\x69\x6c\x65\x2e\x63\x6f\x6c\x64"
      "\x66\x69\x6c\x65\x30\x66\x69\x6c\x65\x31\x66\x69\x6c\x65\x32\x66\x69\x6c"
      "\x65\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00"
      "\xed\x81\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x01\x00"
      "\x00\x00\x5c\xf9\x00\x00\x53\x5f\x00\x00\xe8\x03\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a"
      "\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a"
      "\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a"
      "\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a"
      "\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a"
      "\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\xed\x41\x00\x00\x3d\x00"
      "\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x02\x00\x00\x00\x5c\xf9\x00\x00"
      "\x53\x5f\x00\x00\xe8\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x30\x00\x00\x00\x00\x00\x00\x00\x30\x00\x02\x00\x24\x00\x00\x00\x00\x00"
      "\x00\x00\x31\x00\x02\x00\x34\x00\x00\x00\x00\x00\x00\x00\x33\x00\x01\x00"
      "\x57\x00\x00\x00\x00\x00\x00\x00\x38\x00\x07\x00\x2e\x2e\x2e\x66\x69\x6c"
      "\x65\x30\x66\x69\x6c\x65\x31\x00\x00\x00\x05\x00\x00\x00\xed\x81\x00\x00"
      "\x1a\x04\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x03\x00\x00\x00\x5c\xf9"
      "\x00\x00\x53\x5f\x00\x00\xe8\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x79\x7a\x6b\x61\x6c\x6c"
      "\x65\x72\x73\x79\x7a\x6b\x61\x6c\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00"
      "\xff\xa1\x00\x00\x26\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x04\x00"
      "\x00\x00\x5c\xf9\x00\x00\x53\x5f\x00\x00\xe8\x03\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x2f\x74\x6d\x70\x2f\x73\x79\x7a\x2d\x69\x6d\x61"
      "\x67\x65\x67\x65\x6e\x32\x33\x37\x32\x35\x31\x31\x39\x32\x2f\x66\x69\x6c"
      "\x65\x30\x2f\x66\x69\x6c\x65\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00"
      "\x09\x00\xed\x81\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff"
      "\x05\x00\x00\x00\x5c\xf9\x00\x00\x53\x5f\x00\x00\xe8\x03\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x06\x01\x06\x00\x78\x61\x74\x74\x72\x31\x78\x61\x74\x74\x72\x31"
      "\x06\x01\x06\x00\x78\x61\x74\x74\x72\x32\x78\x61\x74\x74\x72\x32\x73\x79"
      "\x7a\x6b\x61\x6c\x6c\x65\x72\x73\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x07\x00\x00\x00\xed\x81\x00\x00\x28\x23\x00\x00\x00\x00\x00\x00\x01\x00"
      "\x00\x00\x06\x00\x00\x00\x5c\xf9\x00\x00\x53\x5f\x00\x00\xe8\x03\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x10\x01\x20\x00\x00\x00\x00\x28\x03\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00",
      2112);
  *(uint64_t*)0x20000208 = 0x840;
  *(uint64_t*)0x20000210 = 0x400;
  *(uint64_t*)0x20000218 = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0x1fc0;
  *(uint8_t*)0x20010a00 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0x2000, 2, 0x20000200, 0, 0x20010a00);
  memcpy((void*)0x20000040, "./file0\000", 8);
  memcpy((void*)0x20000080, "./file0\000", 8);
  memcpy((void*)0x200000c0, "incremental-fs\000", 15);
  syscall(__NR_mount, 0x20000040ul, 0x20000080ul, 0x200000c0ul, 0x18010ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
