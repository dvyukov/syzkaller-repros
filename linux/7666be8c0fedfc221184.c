// KASAN: use-after-free Read in cma_cancel_operation
// https://syzkaller.appspot.com/bug?id=7666be8c0fedfc221184
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
  int i;
  for (i = 0; i < 100; i++) {
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

static long syz_open_procfs(volatile long a0, volatile long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
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
  for (call = 0; call < 8; call++) {
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
      event_timedwait(&th->done, 45);
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    syscall(__NR_perf_event_open, 0ul, 0, 0ul, -1, 0ul);
    break;
  case 1:
    memcpy((void*)0x200002c0, "/dev/infiniband/rdma_cm\000", 24);
    res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x200002c0ul, 2ul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    syscall(__NR_pipe, 0ul);
    break;
  case 3:
    syz_open_procfs(0, 0);
    break;
  case 4:
    syscall(__NR_sendmsg, -1, 0ul, 0x4000ul);
    break;
  case 5:
    *(uint32_t*)0x20000340 = 0;
    *(uint16_t*)0x20000344 = 0x18;
    *(uint16_t*)0x20000346 = 0xfa00;
    *(uint64_t*)0x20000348 = 0;
    *(uint64_t*)0x20000350 = 0x20000380;
    *(uint16_t*)0x20000358 = 0x13f;
    *(uint8_t*)0x2000035a = 0;
    *(uint8_t*)0x2000035b = 0;
    *(uint8_t*)0x2000035c = 0;
    *(uint8_t*)0x2000035d = 0;
    *(uint8_t*)0x2000035e = 0;
    *(uint8_t*)0x2000035f = 0;
    res = syscall(__NR_write, r[0], 0x20000340ul, 0x20ul);
    if (res != -1)
      r[1] = *(uint32_t*)0x20000380;
    break;
  case 6:
    *(uint32_t*)0x200003c0 = 3;
    *(uint16_t*)0x200003c4 = 0x40;
    *(uint16_t*)0x200003c6 = 0xfa00;
    *(uint16_t*)0x200003c8 = 0xa;
    *(uint16_t*)0x200003ca = htobe16(0x4e21);
    *(uint32_t*)0x200003cc = htobe32(0);
    *(uint8_t*)0x200003d0 = 0;
    *(uint8_t*)0x200003d1 = 0;
    *(uint8_t*)0x200003d2 = 0;
    *(uint8_t*)0x200003d3 = 0;
    *(uint8_t*)0x200003d4 = 0;
    *(uint8_t*)0x200003d5 = 0;
    *(uint8_t*)0x200003d6 = 0;
    *(uint8_t*)0x200003d7 = 0;
    *(uint8_t*)0x200003d8 = 0;
    *(uint8_t*)0x200003d9 = 0;
    *(uint8_t*)0x200003da = -1;
    *(uint8_t*)0x200003db = -1;
    *(uint32_t*)0x200003dc = htobe32(0xe0000001);
    *(uint32_t*)0x200003e0 = 0;
    *(uint16_t*)0x200003e4 = 0xa;
    *(uint16_t*)0x200003e6 = htobe16(0);
    *(uint32_t*)0x200003e8 = htobe32(0);
    memcpy((void*)0x200003ec,
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
           16);
    *(uint32_t*)0x200003fc = 0;
    *(uint32_t*)0x20000400 = r[1];
    *(uint32_t*)0x20000404 = 0;
    syscall(__NR_write, r[0], 0x200003c0ul, 0x48ul);
    break;
  case 7:
    *(uint32_t*)0x200001c0 = 7;
    *(uint16_t*)0x200001c4 = 8;
    *(uint16_t*)0x200001c6 = 0xfa00;
    *(uint32_t*)0x200001c8 = r[1];
    *(uint32_t*)0x200001cc = 0;
    syscall(__NR_write, r[0], 0x200001c0ul, 0x10ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}