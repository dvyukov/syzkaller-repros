// INFO: task hung in io_uring_flush
// https://syzkaller.appspot.com/bug?id=6338dcebf269a590b668
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

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

#define __NR_io_uring_setup 425
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
  uint32_t fd_io_uring = syscall(__NR_io_uring_setup, entries, setup_params);
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

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
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

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

#ifndef __NR_io_uring_enter
#define __NR_io_uring_enter 426
#endif

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0, 0x0};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x20000080 = 0;
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = 0;
  *(uint32_t*)0x20000098 = -1;
  *(uint32_t*)0x2000009c = 0;
  *(uint32_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a4 = 0;
  *(uint32_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000ac = 0;
  *(uint32_t*)0x200000b0 = 0;
  *(uint32_t*)0x200000b4 = 0;
  *(uint32_t*)0x200000b8 = 0;
  *(uint32_t*)0x200000bc = 0;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint32_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d4 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint32_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e4 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = 0;
  *(uint32_t*)0x200000f4 = 0;
  res = -1;
  res = syz_io_uring_setup(0x6ad4, 0x20000080, 0x20001000, 0x206d4000,
                           0x20000180, 0x20000040);
  if (res != -1) {
    r[0] = res;
    r[1] = *(uint32_t*)0x20000098;
    r[2] = *(uint64_t*)0x20000180;
    r[3] = *(uint64_t*)0x20000040;
  }
  *(uint8_t*)0x200001c0 = 0x14;
  *(uint8_t*)0x200001c1 = 4;
  *(uint16_t*)0x200001c2 = 0;
  *(uint32_t*)0x200001c4 = 0;
  *(uint64_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000240;
  *(uint32_t*)0x20000240 = -1;
  *(uint32_t*)0x200001d8 = 1;
  *(uint32_t*)0x200001dc = 0;
  *(uint64_t*)0x200001e0 = 0;
  *(uint16_t*)0x200001e8 = 0;
  *(uint16_t*)0x200001ea = 0;
  *(uint8_t*)0x200001ec = 0;
  *(uint8_t*)0x200001ed = 0;
  *(uint8_t*)0x200001ee = 0;
  *(uint8_t*)0x200001ef = 0;
  *(uint8_t*)0x200001f0 = 0;
  *(uint8_t*)0x200001f1 = 0;
  *(uint8_t*)0x200001f2 = 0;
  *(uint8_t*)0x200001f3 = 0;
  *(uint8_t*)0x200001f4 = 0;
  *(uint8_t*)0x200001f5 = 0;
  *(uint8_t*)0x200001f6 = 0;
  *(uint8_t*)0x200001f7 = 0;
  *(uint8_t*)0x200001f8 = 0;
  *(uint8_t*)0x200001f9 = 0;
  *(uint8_t*)0x200001fa = 0;
  *(uint8_t*)0x200001fb = 0;
  *(uint8_t*)0x200001fc = 0;
  *(uint8_t*)0x200001fd = 0;
  *(uint8_t*)0x200001fe = 0;
  *(uint8_t*)0x200001ff = 0;
  syz_io_uring_submit(r[2], r[3], 0x200001c0, 0);
  *(uint32_t*)0x20001080 = 0;
  *(uint32_t*)0x20001084 = 0;
  *(uint32_t*)0x20001088 = 3;
  *(uint32_t*)0x2000108c = 0;
  *(uint64_t*)0x20001090 = 0;
  *(uint64_t*)0x20001098 = 0;
  *(uint64_t*)0x200010a0 = 0;
  *(uint64_t*)0x200010a8 = 0;
  *(uint64_t*)0x200010b0 = 0;
  *(uint64_t*)0x200010b8 = 0;
  *(uint32_t*)0x200010c0 = 0;
  *(uint32_t*)0x200010c4 = 8;
  *(uint64_t*)0x200010c8 = 0x81;
  *(uint64_t*)0x200010d0 = 0xdc4c;
  *(uint64_t*)0x200010d8 = 0;
  *(uint64_t*)0x200010e0 = 0;
  *(uint64_t*)0x200010e8 = 0;
  *(uint64_t*)0x200010f0 = 0;
  *(uint64_t*)0x200010f8 = 0;
  *(uint32_t*)0x20001100 = 0xfffffffa;
  *(uint32_t*)0x20001104 = 0xfffffeff;
  *(uint64_t*)0x20001108 = 5;
  *(uint64_t*)0x20001110 = 1;
  *(uint64_t*)0x20001118 = 0x8001;
  *(uint64_t*)0x20001120 = 4;
  *(uint64_t*)0x20001128 = 0;
  *(uint64_t*)0x20001130 = 0;
  *(uint64_t*)0x20001138 = 0;
  *(uint8_t*)0x20001140 = 0;
  *(uint8_t*)0x20001141 = 0;
  *(uint8_t*)0x20001142 = 0;
  *(uint8_t*)0x20001143 = 0;
  *(uint8_t*)0x20001144 = 0;
  *(uint8_t*)0x20001145 = 0;
  *(uint8_t*)0x20001146 = 0;
  *(uint8_t*)0x20001147 = 0;
  *(uint8_t*)0x20001148 = 0;
  *(uint8_t*)0x20001149 = 0;
  *(uint8_t*)0x2000114a = 0;
  *(uint8_t*)0x2000114b = 0;
  *(uint8_t*)0x2000114c = 0;
  *(uint8_t*)0x2000114d = 0;
  *(uint8_t*)0x2000114e = 0;
  *(uint8_t*)0x2000114f = 0;
  *(uint8_t*)0x20001150 = 0;
  *(uint8_t*)0x20001151 = 0;
  *(uint8_t*)0x20001152 = 0;
  *(uint8_t*)0x20001153 = 0;
  *(uint8_t*)0x20001154 = 0;
  *(uint8_t*)0x20001155 = 0;
  *(uint8_t*)0x20001156 = 0;
  *(uint8_t*)0x20001157 = 0;
  *(uint8_t*)0x20001158 = 0;
  *(uint8_t*)0x20001159 = 0;
  *(uint8_t*)0x2000115a = 0;
  *(uint8_t*)0x2000115b = 0;
  *(uint8_t*)0x2000115c = 0;
  *(uint8_t*)0x2000115d = 0;
  *(uint8_t*)0x2000115e = 0;
  *(uint8_t*)0x2000115f = 0;
  *(uint8_t*)0x20001160 = 0;
  *(uint8_t*)0x20001161 = 0;
  *(uint8_t*)0x20001162 = 0;
  *(uint8_t*)0x20001163 = 0;
  *(uint8_t*)0x20001164 = 0;
  *(uint8_t*)0x20001165 = 0;
  *(uint8_t*)0x20001166 = 0;
  *(uint8_t*)0x20001167 = 0;
  *(uint8_t*)0x20001168 = 0;
  *(uint8_t*)0x20001169 = 0;
  *(uint8_t*)0x2000116a = 0;
  *(uint8_t*)0x2000116b = 0;
  *(uint8_t*)0x2000116c = 0;
  *(uint8_t*)0x2000116d = 0;
  *(uint8_t*)0x2000116e = 0;
  *(uint8_t*)0x2000116f = 0;
  *(uint8_t*)0x20001170 = 0;
  *(uint8_t*)0x20001171 = 0;
  *(uint8_t*)0x20001172 = 0;
  *(uint8_t*)0x20001173 = 0;
  *(uint8_t*)0x20001174 = 0;
  *(uint8_t*)0x20001175 = 0;
  *(uint8_t*)0x20001176 = 0;
  *(uint8_t*)0x20001177 = 0;
  *(uint8_t*)0x20001178 = 0;
  *(uint8_t*)0x20001179 = 0;
  *(uint8_t*)0x2000117a = 0;
  *(uint8_t*)0x2000117b = 0;
  *(uint8_t*)0x2000117c = 0;
  *(uint8_t*)0x2000117d = 0;
  *(uint8_t*)0x2000117e = 0;
  *(uint8_t*)0x2000117f = 0;
  *(uint8_t*)0x20001180 = 0;
  *(uint8_t*)0x20001181 = 0;
  *(uint8_t*)0x20001182 = 0;
  *(uint8_t*)0x20001183 = 0;
  *(uint8_t*)0x20001184 = 0;
  *(uint8_t*)0x20001185 = 0;
  *(uint8_t*)0x20001186 = 0;
  *(uint8_t*)0x20001187 = 0;
  *(uint8_t*)0x20001188 = 0;
  *(uint8_t*)0x20001189 = 0;
  *(uint8_t*)0x2000118a = 0;
  *(uint8_t*)0x2000118b = 0;
  *(uint8_t*)0x2000118c = 0;
  *(uint8_t*)0x2000118d = 0;
  *(uint8_t*)0x2000118e = 0;
  *(uint8_t*)0x2000118f = 0;
  *(uint8_t*)0x20001190 = 0;
  *(uint8_t*)0x20001191 = 0;
  *(uint8_t*)0x20001192 = 0;
  *(uint8_t*)0x20001193 = 0;
  *(uint8_t*)0x20001194 = 0;
  *(uint8_t*)0x20001195 = 0;
  *(uint8_t*)0x20001196 = 0;
  *(uint8_t*)0x20001197 = 0;
  *(uint8_t*)0x20001198 = 0;
  *(uint8_t*)0x20001199 = 0;
  *(uint8_t*)0x2000119a = 0;
  *(uint8_t*)0x2000119b = 0;
  *(uint8_t*)0x2000119c = 0;
  *(uint8_t*)0x2000119d = 0;
  *(uint8_t*)0x2000119e = 0;
  *(uint8_t*)0x2000119f = 0;
  *(uint8_t*)0x200011a0 = 0;
  *(uint8_t*)0x200011a1 = 0;
  *(uint8_t*)0x200011a2 = 0;
  *(uint8_t*)0x200011a3 = 0;
  *(uint8_t*)0x200011a4 = 0;
  *(uint8_t*)0x200011a5 = 0;
  *(uint8_t*)0x200011a6 = 0;
  *(uint8_t*)0x200011a7 = 0;
  *(uint8_t*)0x200011a8 = 0;
  *(uint8_t*)0x200011a9 = 0;
  *(uint8_t*)0x200011aa = 0;
  *(uint8_t*)0x200011ab = 0;
  *(uint8_t*)0x200011ac = 0;
  *(uint8_t*)0x200011ad = 0;
  *(uint8_t*)0x200011ae = 0;
  *(uint8_t*)0x200011af = 0;
  *(uint8_t*)0x200011b0 = 0;
  *(uint8_t*)0x200011b1 = 0;
  *(uint8_t*)0x200011b2 = 0;
  *(uint8_t*)0x200011b3 = 0;
  *(uint8_t*)0x200011b4 = 0;
  *(uint8_t*)0x200011b5 = 0;
  *(uint8_t*)0x200011b6 = 0;
  *(uint8_t*)0x200011b7 = 0;
  *(uint8_t*)0x200011b8 = 0;
  *(uint8_t*)0x200011b9 = 0;
  *(uint8_t*)0x200011ba = 0;
  *(uint8_t*)0x200011bb = 0;
  *(uint8_t*)0x200011bc = 0;
  *(uint8_t*)0x200011bd = 0;
  *(uint8_t*)0x200011be = 0;
  *(uint8_t*)0x200011bf = 0;
  *(uint8_t*)0x200011c0 = 0;
  *(uint8_t*)0x200011c1 = 0;
  *(uint8_t*)0x200011c2 = 0;
  *(uint8_t*)0x200011c3 = 0;
  *(uint8_t*)0x200011c4 = 0;
  *(uint8_t*)0x200011c5 = 0;
  *(uint8_t*)0x200011c6 = 0;
  *(uint8_t*)0x200011c7 = 0;
  *(uint8_t*)0x200011c8 = 0;
  *(uint8_t*)0x200011c9 = 0;
  *(uint8_t*)0x200011ca = 0;
  *(uint8_t*)0x200011cb = 0;
  *(uint8_t*)0x200011cc = 0;
  *(uint8_t*)0x200011cd = 0;
  *(uint8_t*)0x200011ce = 0;
  *(uint8_t*)0x200011cf = 0;
  *(uint8_t*)0x200011d0 = 0;
  *(uint8_t*)0x200011d1 = 0;
  *(uint8_t*)0x200011d2 = 0;
  *(uint8_t*)0x200011d3 = 0;
  *(uint8_t*)0x200011d4 = 0;
  *(uint8_t*)0x200011d5 = 0;
  *(uint8_t*)0x200011d6 = 0;
  *(uint8_t*)0x200011d7 = 0;
  *(uint8_t*)0x200011d8 = 0;
  *(uint8_t*)0x200011d9 = 0;
  *(uint8_t*)0x200011da = 0;
  *(uint8_t*)0x200011db = 0;
  *(uint8_t*)0x200011dc = 0;
  *(uint8_t*)0x200011dd = 0;
  *(uint8_t*)0x200011de = 0;
  *(uint8_t*)0x200011df = 0;
  *(uint8_t*)0x200011e0 = 0;
  *(uint8_t*)0x200011e1 = 0;
  *(uint8_t*)0x200011e2 = 0;
  *(uint8_t*)0x200011e3 = 0;
  *(uint8_t*)0x200011e4 = 0;
  *(uint8_t*)0x200011e5 = 0;
  *(uint8_t*)0x200011e6 = 0;
  *(uint8_t*)0x200011e7 = 0;
  *(uint8_t*)0x200011e8 = 0;
  *(uint8_t*)0x200011e9 = 0;
  *(uint8_t*)0x200011ea = 0;
  *(uint8_t*)0x200011eb = 0;
  *(uint8_t*)0x200011ec = 0;
  *(uint8_t*)0x200011ed = 0;
  *(uint8_t*)0x200011ee = 0;
  *(uint8_t*)0x200011ef = 0;
  *(uint8_t*)0x200011f0 = 0;
  *(uint8_t*)0x200011f1 = 0;
  *(uint8_t*)0x200011f2 = 0;
  *(uint8_t*)0x200011f3 = 0;
  *(uint8_t*)0x200011f4 = 0;
  *(uint8_t*)0x200011f5 = 0;
  *(uint8_t*)0x200011f6 = 0;
  *(uint8_t*)0x200011f7 = 0;
  *(uint8_t*)0x200011f8 = 0;
  *(uint8_t*)0x200011f9 = 0;
  *(uint8_t*)0x200011fa = 0;
  *(uint8_t*)0x200011fb = 0;
  *(uint8_t*)0x200011fc = 0;
  *(uint8_t*)0x200011fd = 0;
  *(uint8_t*)0x200011fe = 0;
  *(uint8_t*)0x200011ff = 0;
  syscall(__NR_ioctl, r[1], 0xc0c0583b, 0x20001080ul);
  syscall(__NR_io_uring_enter, r[0], 0x450c, 0, 0ul, 0ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}