// WARNING in bpf_test_run
// https://syzkaller.appspot.com/bug?id=774c590240616eaa3423
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  int collide = 0;
again:
  for (call = 0; call < 5; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 50);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  if (!collide) {
    collide = 1;
    goto again;
  }
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
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
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    *(uint32_t*)0x20000200 = 3;
    *(uint32_t*)0x20000204 = 0xe;
    *(uint64_t*)0x20000208 = 0x20000440;
    memcpy((void*)0x20000440,
           "\xb7\x02\x00\x00\x03\x00\x00\x00\xbf\xa3\x00\x00\x00\x00\x00\x00"
           "\x07\x03\x00\x00\x00\xfe\xff\xff\x7a\x0a\xf0\xff\xf8\xff\xff\xff"
           "\x79\xa4\xf0\xff\x00\x00\x00\x00\xb7\x06\x00\x00\xff\xff\xff\xff"
           "\x2d\x64\x05\x00\x00\x00\x00\x00\x65\x04\x04\x00\x01\x00\x00\x00"
           "\x04\x04\x00\x00\x01\x00\x7d\x60\xb7\x03\x00\x00\x00\x00\x00\x00"
           "\x6a\x0a\x00\xfe\x87\x68\x00\x00\x85\x00\x00\x00\x0d\x00\x00\x00"
           "\xb7\x00\x00\x00\x00\x00\x00\x00\x95\x00\x02\x00\x00\x00\x00\x00"
           "\xca\xcb\xf0\xb9\xc4\xb8\x84\x49\xc3\xa9\x26\x04\x25\x12\xe1\x7e"
           "\x46\xf7\x07\x1a\x46\x31\x43\xfb\x42\xc2\x0e\x01\x7d\xa8\x39\xee"
           "\xc4\x21\x07\xf2\xe6\xdd\xbe\x11\x50\x29\x6c\x6a\x6d\xb4\xaf\xa7"
           "\xc8\x1b\x26\x36\xb1\xc5\xfb\xe2\x4e\xc8\x83\x17",
           172);
    *(uint64_t*)0x20000210 = 0x20000340;
    memcpy((void*)0x20000340, "syzkaller\000", 10);
    *(uint32_t*)0x20000218 = 0;
    *(uint32_t*)0x2000021c = 0;
    *(uint64_t*)0x20000220 = 0;
    *(uint32_t*)0x20000228 = 0;
    *(uint32_t*)0x2000022c = 0;
    *(uint8_t*)0x20000230 = 0;
    *(uint8_t*)0x20000231 = 0;
    *(uint8_t*)0x20000232 = 0;
    *(uint8_t*)0x20000233 = 0;
    *(uint8_t*)0x20000234 = 0;
    *(uint8_t*)0x20000235 = 0;
    *(uint8_t*)0x20000236 = 0;
    *(uint8_t*)0x20000237 = 0;
    *(uint8_t*)0x20000238 = 0;
    *(uint8_t*)0x20000239 = 0;
    *(uint8_t*)0x2000023a = 0;
    *(uint8_t*)0x2000023b = 0;
    *(uint8_t*)0x2000023c = 0;
    *(uint8_t*)0x2000023d = 0;
    *(uint8_t*)0x2000023e = 0;
    *(uint8_t*)0x2000023f = 0;
    *(uint32_t*)0x20000240 = 0;
    *(uint32_t*)0x20000244 = 0;
    *(uint32_t*)0x20000248 = -1;
    *(uint32_t*)0x2000024c = 8;
    *(uint64_t*)0x20000250 = 0x20000000;
    *(uint32_t*)0x20000000 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000258 = 0x300;
    *(uint32_t*)0x2000025c = 0x10;
    *(uint64_t*)0x20000260 = 0x20000000;
    *(uint32_t*)0x20000000 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x2000000c = 0;
    *(uint32_t*)0x20000268 = 0;
    *(uint32_t*)0x2000026c = 0;
    *(uint32_t*)0x20000270 = -1;
    res = syscall(__NR_bpf, 5ul, 0x20000200ul, 0x48ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint32_t*)0x20000180 = 1;
    *(uint32_t*)0x20000184 = 0x70;
    *(uint8_t*)0x20000188 = 0;
    *(uint8_t*)0x20000189 = 0;
    *(uint8_t*)0x2000018a = 0;
    *(uint8_t*)0x2000018b = 0;
    *(uint32_t*)0x2000018c = 0;
    *(uint64_t*)0x20000190 = 0x1ff;
    *(uint64_t*)0x20000198 = 0;
    *(uint64_t*)0x200001a0 = 0;
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 29, 35);
    *(uint32_t*)0x200001b0 = 0;
    *(uint32_t*)0x200001b4 = 0;
    *(uint64_t*)0x200001b8 = 0;
    *(uint64_t*)0x200001c0 = 0xe;
    *(uint64_t*)0x200001c8 = 0;
    *(uint64_t*)0x200001d0 = 0;
    *(uint32_t*)0x200001d8 = 0;
    *(uint32_t*)0x200001dc = 0;
    *(uint64_t*)0x200001e0 = 0;
    *(uint32_t*)0x200001e8 = 0;
    *(uint16_t*)0x200001ec = 0xfffe;
    *(uint16_t*)0x200001ee = 0;
    syscall(__NR_perf_event_open, 0x20000180ul, 0, -1ul, -1, 0ul);
    break;
  case 2:
    *(uint32_t*)0x20000080 = r[0];
    *(uint32_t*)0x20000084 = 0x2a0;
    *(uint32_t*)0x20000088 = 0xe80;
    *(uint32_t*)0x2000008c = 0xe000000;
    *(uint64_t*)0x20000090 = 0x200000c0;
    memcpy(
        (void*)0x200000c0,
        "\xb9\xff\x03\x00\x00\xff\xff\xff\x7f\x9e\x14\xf0\x05\x05\x1f\xff\xff"
        "\xff\x00\x60\x40\x00\x63\x06\x77\xfb\xac\x14\x14\x33\xe0\x00\x03\x01"
        "\x62\x02\x9f\x4b\x4d\x2f\x87\xe5\xfe\xca\x6a\xab\x84\x04\x13\xf2\x32"
        "\x5f\x1a\x39\x01\x04\x05\x10\x01\x00\x01\x00\x00\x00\x02\x00\xdf\x74"
        "\xe3\x0d\x7e\xab\xe7\x73\xaf\xef\x6f\x6e\x47\x98\xab\x11\x7e\x9f\x84"
        "\xfa\x40\x6b\x91\x3d\xe8\xad\x82\x7a\x02\x2e\x1f\xae\xe5\x08\x87\xdc"
        "\x30\x28\x19\xa8\xa3\xd0\xcd\xe3\x6b\x67\xf3\x37\xce\x8e\xee\x12\x4e"
        "\x06\x1f\x8f\xea\x8a\xb9\x5f\x1e\x8f\x99\xc7\xed\xea\x98\x06\x97\x44"
        "\x9b\x78\x56\x9e\xa2\x93\xc3\xee\xd3\xb2\x8f\xc3\x20\x5d\xb6\x3b\x2c"
        "\x65\xe7\x7f\x19\xab\x28\xc6\x32\xcc\x80\xd9\xf2\xf3\x7f\x9b\xa6\x71"
        "\x74\xff\xfc\xb5\x24\x4b\x0c\x90\x9e\xb8\xe1\x21\x16\xbe\xbc\x47\xcf"
        "\x97\xd2\xea\x8a\xca\xdf\xb3\x4c\xa5\x80\xb6\x4d\xf7\xc8\x00\x11\x3e"
        "\x53\xba\xe4\x01\xcd\x22\xf5\x00\x72\xde\xab\xf9\x3d\xd4\xd3\xe6\x26",
        221);
    *(uint64_t*)0x20000098 = 0;
    *(uint32_t*)0x200000a0 = 0x100;
    *(uint32_t*)0x200000a4 = 0;
    *(uint32_t*)0x200000a8 = 0x296;
    *(uint32_t*)0x200000ac = 0;
    *(uint64_t*)0x200000b0 = 0x20000000;
    *(uint64_t*)0x200000b8 = 0x20000040;
    *(uint32_t*)0x200000c0 = 0;
    *(uint32_t*)0x200000c4 = 0;
    syscall(__NR_bpf, 0xaul, 0x20000080ul, 0x28ul);
    break;
  case 3:
    *(uint32_t*)0x20000200 = 3;
    *(uint32_t*)0x20000204 = 0xe;
    *(uint64_t*)0x20000208 = 0x200002c0;
    memcpy((void*)0x200002c0,
           "\xb7\x02\x00\x00\x03\x00\x00\x00\xbf\xa3\x00\x00\x00\x00\x00\x00"
           "\x07\x03\x00\x00\x00\xfe\xff\xff\x7a\x0a\xf0\xff\xf8\xff\xff\xff"
           "\x79\xa4\xf0\xff\x00\x00\x00\x00\xb7\x06\x00\x00\xff\xff\xff\xff"
           "\x2d\x64\x05\x00\x00\x00\x00\x00\x65\x04\x04\x00\x01\x00\x00\x00"
           "\x04\x04\x00\x00\x01\x00\x7d\x60\xb7\x03\x00\x00\x00\x00\x00\x00"
           "\x6a\x0a\x00\xfe\x00\x00\x00\x00\x85\x00\x00\x00\x0d\x00\x00\x00"
           "\xb7\x00\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
           "\x07\x0c\x24\xaf\xf3\xdd\x52\x39\x01\x13\xc1\xd1",
           124);
    *(uint64_t*)0x20000210 = 0x20000340;
    memcpy((void*)0x20000340, "syzkaller\000", 10);
    *(uint32_t*)0x20000218 = 0;
    *(uint32_t*)0x2000021c = 0;
    *(uint64_t*)0x20000220 = 0;
    *(uint32_t*)0x20000228 = 0;
    *(uint32_t*)0x2000022c = 0;
    *(uint8_t*)0x20000230 = 0;
    *(uint8_t*)0x20000231 = 0;
    *(uint8_t*)0x20000232 = 0;
    *(uint8_t*)0x20000233 = 0;
    *(uint8_t*)0x20000234 = 0;
    *(uint8_t*)0x20000235 = 0;
    *(uint8_t*)0x20000236 = 0;
    *(uint8_t*)0x20000237 = 0;
    *(uint8_t*)0x20000238 = 0;
    *(uint8_t*)0x20000239 = 0;
    *(uint8_t*)0x2000023a = 0;
    *(uint8_t*)0x2000023b = 0;
    *(uint8_t*)0x2000023c = 0;
    *(uint8_t*)0x2000023d = 0;
    *(uint8_t*)0x2000023e = 0;
    *(uint8_t*)0x2000023f = 0;
    *(uint32_t*)0x20000240 = 0;
    *(uint32_t*)0x20000244 = 0;
    *(uint32_t*)0x20000248 = -1;
    *(uint32_t*)0x2000024c = 8;
    *(uint64_t*)0x20000250 = 0x20000000;
    *(uint32_t*)0x20000000 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000258 = 0x300;
    *(uint32_t*)0x2000025c = 0x10;
    *(uint64_t*)0x20000260 = 0x20000000;
    *(uint32_t*)0x20000000 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x2000000c = 0;
    *(uint32_t*)0x20000268 = 0;
    *(uint32_t*)0x2000026c = 0;
    *(uint32_t*)0x20000270 = -1;
    res = syscall(__NR_bpf, 5ul, 0x20000200ul, 0x48ul);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    *(uint32_t*)0x20000080 = r[1];
    *(uint32_t*)0x20000084 = 0x2a0;
    *(uint32_t*)0x20000088 = 0x5ee;
    *(uint32_t*)0x2000008c = 0xe000000;
    *(uint64_t*)0x20000090 = 0x200000c0;
    memcpy(
        (void*)0x200000c0,
        "\xb9\xff\x03\x00\x00\xff\xff\xff\x7f\x9e\x14\xf0\x05\x05\x1f\xff\xff"
        "\xff\x00\x00\x40\x00\x63\x06\x77\xfb\xac\x14\x14\x33\xe0\x00\x00\x01"
        "\x62\x07\x9f\x4b\x4d\x2f\x87\xe5\xfe\xca\x6a\xab\x84\x02\x13\xf2\x32"
        "\x5f\x1a\x39\x01\x01\x05\x0a\x01\x00\x01\x00\x00\x00\x00\x00\xdf\x74"
        "\xe3\x0d\x7e\xab\xe7\x73\xaf\xef\x6f\x6e\x47\x98\xab\x11\x7e\x9f\x84"
        "\xfa\x40\x6b\x91\x3d\xe8\xad\x82\x7a\x02\x2e\x1f\xae\xe5\x08\x87\xdc"
        "\x30\x28\x19\xa8\xa3\xd0\xcd\xe3\x6b\x67\xf3\x37\xce\x8e\xee\x12\x4e"
        "\x06\x1f\x8f\xea\x8a\xb9\x5f\x1e\x8f\x99\xc7\xed\xea\x98\x06\x97\x44"
        "\x9b\x78\x56\x9e\xa2\x93\xc3\xee\xd3\xb2\x8f\xc3\x20\x5d\xb6\x3b\x2c"
        "\x65\xe7\x7f\x19\xab\x28\xc6\x32\xcc\x80\xd9\xf2\xf3\x7f\x9b\xa6\x71"
        "\x74\xff\xfc\xb5\x24\x4b\x0c\x90\x9e\xb8\xe1\x21\x16\xbe\xbc\x47\xcf"
        "\x97\xd2\xea\x8a\xca\xdf\xb3\x4c\xa5\x80\xb6\x4d\xf7\xc8\x00\x11\x3e"
        "\x53\xba\xe4\x01\xcd\x22\xf5\x00\x72\xde\xab\xf9\x3d\xd4\xd3\xe6\x26",
        221);
    *(uint64_t*)0x20000098 = 0;
    *(uint32_t*)0x200000a0 = 0x100;
    *(uint32_t*)0x200000a4 = 0;
    *(uint32_t*)0x200000a8 = 0x2e0;
    *(uint32_t*)0x200000ac = 0;
    *(uint64_t*)0x200000b0 = 0x20000380;
    memcpy((void*)0x20000380,
           "\xff\x82\xc0\xee\xe0\xfb\xd2\x48\x19\xda\xf3\x25\x7f\x77\xca\x52"
           "\xc6\x05\xde\xf2\x0d\xf2\xfe\xdf\x60\xe9\x12\x10\x3b\xd5\x98\x3f"
           "\x4f\xa6\x83\x4a\x67\xb6\x47\xa1\x3e\xd9\x11\x5c\xd9\x9f\x59\xa4"
           "\x24\x92\x4a\x2c\x3f\xe1\x1d\x1e\x51\xfe\xe9\x66\x9e\x5c\x80\x6d"
           "\x41\x9b\x42\x2a\x12\x6b\xa8\xdc\x80\xcb\xc6\xbf\xeb\xed\xfc\x8e"
           "\xe1\x3e\x9f\xf4\x93\x90\x5b\x1d\x22\xde\xa4\x0e\xd0\xe3\xf6\xa7"
           "\x9d\xa9\x3f\x2d\xd8\xe0\x31\x16\xdb\xdc",
           106);
    *(uint64_t*)0x200000b8 = 0x20000040;
    *(uint32_t*)0x200000c0 = 0;
    *(uint32_t*)0x200000c4 = 0;
    syscall(__NR_bpf, 0xaul, 0x20000080ul, 0x28ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
