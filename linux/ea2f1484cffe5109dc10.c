// WARNING in io_wqe_enqueue
// https://syzkaller.appspot.com/bug?id=ea2f1484cffe5109dc10
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
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

#define SIZEOF_IO_URING_SQE 64
#define SIZEOF_IO_URING_CQE 16
#define SQ_HEAD_OFFSET 0
#define SQ_TAIL_OFFSET 64
#define SQ_RING_MASK_OFFSET 256
#define SQ_RING_ENTRIES_OFFSET 264
#define SQ_FLAGS_OFFSET 276
#define SQ_DROPPED_OFFSET 272
#define CQ_HEAD_OFFSET 128
#define CQ_TAIL_OFFSET 192
#define CQ_RING_MASK_OFFSET 260
#define CQ_RING_ENTRIES_OFFSET 268
#define CQ_RING_OVERFLOW_OFFSET 284
#define CQ_FLAGS_OFFSET 280
#define CQ_CQES_OFFSET 320

struct io_sqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t flags;
  uint32_t dropped;
  uint32_t array;
  uint32_t resv1;
  uint64_t resv2;
};

struct io_cqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t overflow;
  uint32_t cqes;
  uint64_t resv[2];
};

struct io_uring_params {
  uint32_t sq_entries;
  uint32_t cq_entries;
  uint32_t flags;
  uint32_t sq_thread_cpu;
  uint32_t sq_thread_idle;
  uint32_t features;
  uint32_t resv[4];
  struct io_sqring_offsets sq_off;
  struct io_cqring_offsets cq_off;
};

#define IORING_OFF_SQ_RING 0
#define IORING_OFF_SQES 0x10000000ULL

#define sys_io_uring_setup 425
static long syz_io_uring_setup(volatile long a0, volatile long a1,
                               volatile long a2, volatile long a3,
                               volatile long a4, volatile long a5)
{
  uint32_t entries = (uint32_t)a0;
  struct io_uring_params* setup_params = (struct io_uring_params*)a1;
  void* vma1 = (void*)a2;
  void* vma2 = (void*)a3;
  void** ring_ptr_out = (void**)a4;
  void** sqes_ptr_out = (void**)a5;
  uint32_t fd_io_uring = syscall(sys_io_uring_setup, entries, setup_params);
  uint32_t sq_ring_sz =
      setup_params->sq_off.array + setup_params->sq_entries * sizeof(uint32_t);
  uint32_t cq_ring_sz = setup_params->cq_off.cqes +
                        setup_params->cq_entries * SIZEOF_IO_URING_CQE;
  uint32_t ring_sz = sq_ring_sz > cq_ring_sz ? sq_ring_sz : cq_ring_sz;
  *ring_ptr_out = mmap(vma1, ring_sz, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_POPULATE | MAP_FIXED, fd_io_uring,
                       IORING_OFF_SQ_RING);
  uint32_t sqes_sz = setup_params->sq_entries * SIZEOF_IO_URING_SQE;
  *sqes_ptr_out =
      mmap(vma2, sqes_sz, PROT_READ | PROT_WRITE,
           MAP_SHARED | MAP_POPULATE | MAP_FIXED, fd_io_uring, IORING_OFF_SQES);
  return fd_io_uring;
}

static long syz_io_uring_submit(volatile long a0, volatile long a1,
                                volatile long a2, volatile long a3)
{
  char* ring_ptr = (char*)a0;
  char* sqes_ptr = (char*)a1;
  char* sqe = (char*)a2;
  uint32_t sqes_index = (uint32_t)a3;
  uint32_t sq_ring_entries = *(uint32_t*)(ring_ptr + SQ_RING_ENTRIES_OFFSET);
  uint32_t cq_ring_entries = *(uint32_t*)(ring_ptr + CQ_RING_ENTRIES_OFFSET);
  uint32_t sq_array_off =
      (CQ_CQES_OFFSET + cq_ring_entries * SIZEOF_IO_URING_CQE + 63) & ~63;
  if (sq_ring_entries)
    sqes_index %= sq_ring_entries;
  char* sqe_dest = sqes_ptr + sqes_index * SIZEOF_IO_URING_SQE;
  memcpy(sqe_dest, sqe, SIZEOF_IO_URING_SQE);
  uint32_t sq_ring_mask = *(uint32_t*)(ring_ptr + SQ_RING_MASK_OFFSET);
  uint32_t* sq_tail_ptr = (uint32_t*)(ring_ptr + SQ_TAIL_OFFSET);
  uint32_t sq_tail = *sq_tail_ptr & sq_ring_mask;
  uint32_t sq_tail_next = *sq_tail_ptr + 1;
  uint32_t* sq_array = (uint32_t*)(ring_ptr + sq_array_off);
  *(sq_array + sq_tail) = sqes_index;
  __atomic_store_n(sq_tail_ptr, sq_tail_next, __ATOMIC_RELEASE);
  return 0;
}

static long syz_memcpy_off(volatile long a0, volatile long a1, volatile long a2,
                           volatile long a3, volatile long a4)
{
  char* dest = (char*)a0;
  uint32_t dest_off = (uint32_t)a1;
  char* src = (char*)a2;
  uint32_t src_off = (uint32_t)a3;
  size_t n = (size_t)a4;
  return (long)memcpy(dest + dest_off, src + src_off, n);
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

#ifndef __NR_io_uring_enter
#define __NR_io_uring_enter 426
#endif

uint64_t r[5] = {0xffffffffffffffff, 0x0, 0x0, 0xffffffffffffffff,
                 0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  syscall(__NR_prlimit64, 0, 6ul, 0x20000180ul, 0ul);
  *(uint32_t*)0x200002c4 = 0;
  *(uint32_t*)0x200002c8 = 0;
  *(uint32_t*)0x200002cc = 0;
  *(uint32_t*)0x200002d0 = 0;
  *(uint32_t*)0x200002d8 = -1;
  memset((void*)0x200002dc, 0, 12);
  res = -1;
  res = syz_io_uring_setup(0x187, 0x200002c0, 0x206d5000, 0x206d6000,
                           0x20000000, 0x20000100);
  if (res != -1) {
    r[0] = res;
    r[1] = *(uint64_t*)0x20000000;
    r[2] = *(uint64_t*)0x20000100;
  }
  *(uint32_t*)0x20000140 = 0x307e;
  syz_memcpy_off(r[1], 0x40, 0x20000140, 0, 4);
  res = syscall(__NR_socketpair, 1ul, 5ul, 0, 0x20000040ul);
  if (res != -1) {
    r[3] = *(uint32_t*)0x20000040;
    r[4] = *(uint32_t*)0x20000044;
  }
  *(uint8_t*)0x200000c0 = 9;
  *(uint8_t*)0x200000c1 = 0;
  *(uint16_t*)0x200000c2 = 0;
  *(uint32_t*)0x200000c4 = r[4];
  *(uint64_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0x20000640;
  *(uint64_t*)0x20000640 = 0;
  *(uint32_t*)0x20000648 = 0;
  *(uint64_t*)0x20000650 = 0;
  *(uint64_t*)0x20000658 = 0;
  *(uint64_t*)0x20000660 = 0;
  *(uint64_t*)0x20000668 = 0;
  *(uint32_t*)0x20000670 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint16_t*)0x200000e8 = 0;
  *(uint16_t*)0x200000ea = 0;
  memset((void*)0x200000ec, 0, 20);
  syz_io_uring_submit(r[1], r[2], 0x200000c0, 0);
  syscall(__NR_io_uring_enter, r[0], 0x2a6e, 0, 0ul, 0ul, 0ul);
  syscall(__NR_shutdown, r[3], 1ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}