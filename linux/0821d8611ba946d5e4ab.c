// INFO: task hung in fuse_reverse_inval_entry
// https://syzkaller.appspot.com/bug?id=0821d8611ba946d5e4ab
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

static void setup_sysctl()
{
  char mypid[32];
  snprintf(mypid, sizeof(mypid), "%d", getpid());
  struct {
    const char* name;
    const char* data;
  } files[] = {
      {"/sys/kernel/debug/x86/nmi_longest_ns", "10000000000"},
      {"/proc/sys/kernel/hung_task_check_interval_secs", "20"},
      {"/proc/sys/net/core/bpf_jit_kallsyms", "1"},
      {"/proc/sys/net/core/bpf_jit_harden", "0"},
      {"/proc/sys/kernel/kptr_restrict", "0"},
      {"/proc/sys/kernel/softlockup_all_cpu_backtrace", "1"},
      {"/proc/sys/fs/mount-max", "100"},
      {"/proc/sys/vm/oom_dump_tasks", "0"},
      {"/proc/sys/debug/exception-trace", "0"},
      {"/proc/sys/kernel/printk", "7 4 1 3"},
      {"/proc/sys/net/ipv4/ping_group_range", "0 65535"},
      {"/proc/sys/kernel/keys/gc_delay", "1"},
      {"/proc/sys/vm/oom_kill_allocating_task", "1"},
      {"/proc/sys/kernel/ctrl-alt-del", "0"},
      {"/proc/sys/kernel/cad_pid", mypid},
  };
  for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].name, files[i].data))
      printf("write to %s failed: %s\n", files[i].name, strerror(errno));
  }
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
  for (call = 0; call < 10; call++) {
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
      event_timedwait(&th->done, 50);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
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
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0x0};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x20000180, "./file0\000", 8);
    syscall(__NR_mkdir, 0x20000180ul, 0ul);
    break;
  case 1:
    memcpy((void*)0x20000100, "./file0\000", 8);
    memcpy((void*)0x200016c0, "fusectl\000", 8);
    syscall(__NR_mount, 0ul, 0x20000100ul, 0x200016c0ul, 0ul, 0ul);
    break;
  case 2:
    memcpy((void*)0x200001c0, "./file0\000", 8);
    syscall(__NR_mount, 0ul, 0x200001c0ul, 0ul, 0x6ca5ul, 0ul);
    break;
  case 3:
    memcpy((void*)0x20002080, "/dev/fuse\000", 10);
    res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20002080ul, 2ul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 4:
    memcpy((void*)0x200020c0, "./file0\000", 8);
    memcpy((void*)0x20002100, "fuse\000", 5);
    memcpy((void*)0x20002140, "fd", 2);
    *(uint8_t*)0x20002142 = 0x3d;
    sprintf((char*)0x20002143, "0x%016llx", (long long)r[0]);
    *(uint8_t*)0x20002155 = 0x2c;
    memcpy((void*)0x20002156, "rootmode", 8);
    *(uint8_t*)0x2000215e = 0x3d;
    sprintf((char*)0x2000215f, "%023llo", (long long)0x4000);
    *(uint8_t*)0x20002176 = 0x2c;
    memcpy((void*)0x20002177, "user_id", 7);
    *(uint8_t*)0x2000217e = 0x3d;
    sprintf((char*)0x2000217f, "%020llu", (long long)0);
    *(uint8_t*)0x20002193 = 0x2c;
    memcpy((void*)0x20002194, "group_id", 8);
    *(uint8_t*)0x2000219c = 0x3d;
    sprintf((char*)0x2000219d, "%020llu", (long long)0);
    *(uint8_t*)0x200021b1 = 0x2c;
    *(uint8_t*)0x200021b2 = 0;
    syscall(__NR_mount, 0ul, 0x200020c0ul, 0x20002100ul, 0ul, 0x20002140ul);
    break;
  case 5:
    res = syscall(__NR_read, r[0], 0x200021c0ul, 0x2020ul);
    if (res != -1)
      r[1] = *(uint64_t*)0x200021c8;
    break;
  case 6:
    syscall(__NR_pread64, r[0], 0x20004200ul, 0xecul, 0ul);
    break;
  case 7:
    *(uint32_t*)0x20004300 = 0x50;
    *(uint32_t*)0x20004304 = 0;
    *(uint64_t*)0x20004308 = r[1];
    *(uint32_t*)0x20004310 = 7;
    *(uint32_t*)0x20004314 = 0x23;
    *(uint32_t*)0x20004318 = 0;
    *(uint32_t*)0x2000431c = 0;
    *(uint16_t*)0x20004320 = 0;
    *(uint16_t*)0x20004322 = 0;
    *(uint32_t*)0x20004324 = 0;
    *(uint32_t*)0x20004328 = 0;
    *(uint16_t*)0x2000432c = 0;
    *(uint16_t*)0x2000432e = 0;
    memset((void*)0x20004330, 0, 32);
    syscall(__NR_write, r[0], 0x20004300ul, 0x50ul);
    break;
  case 8:
    memcpy((void*)0x20004380, "./file0/file0\000", 14);
    syscall(__NR_mkdirat, 0xffffff9c, 0x20004380ul, 0ul);
    break;
  case 9:
    *(uint32_t*)0x200043c0 = 0x29;
    *(uint32_t*)0x200043c4 = 3;
    *(uint64_t*)0x200043c8 = 0;
    *(uint64_t*)0x200043d0 = 1;
    *(uint32_t*)0x200043d8 = 8;
    *(uint32_t*)0x200043dc = 0;
    memcpy((void*)0x200043e0, "group_id", 8);
    *(uint8_t*)0x200043e8 = 0;
    syscall(__NR_write, r[0], 0x200043c0ul, 0x29ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_sysctl();
  loop();
  return 0;
}