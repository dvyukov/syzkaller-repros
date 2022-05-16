// possible deadlock in xt_find_target
// https://syzkaller.appspot.com/bug?id=682e88cf0cbf9bf5d093
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
  for (call = 0; call < 7; call++) {
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
      event_timedwait(&th->done, 45);
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 2ul, 2ul, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy((void*)0x20000540, "filter\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000",
           32);
    *(uint32_t*)0x20000560 = 0xe;
    *(uint32_t*)0x20000564 = 4;
    *(uint32_t*)0x20000568 = 0x430;
    *(uint32_t*)0x2000056c = 0x218;
    *(uint32_t*)0x20000570 = 0x218;
    *(uint32_t*)0x20000574 = 0x2b0;
    *(uint32_t*)0x20000578 = 0;
    *(uint32_t*)0x2000057c = 0x398;
    *(uint32_t*)0x20000580 = 0x398;
    *(uint32_t*)0x20000584 = 0x398;
    *(uint32_t*)0x20000588 = 0x398;
    *(uint32_t*)0x2000058c = 0x398;
    *(uint32_t*)0x20000590 = 0x398;
    *(uint32_t*)0x20000594 = 4;
    *(uint64_t*)0x20000598 = 0;
    *(uint32_t*)0x200005a0 = htobe32(0x7f000001);
    *(uint8_t*)0x200005a4 = 0xac;
    *(uint8_t*)0x200005a5 = 0x14;
    *(uint8_t*)0x200005a6 = 0x14;
    *(uint8_t*)0x200005a7 = 0xbb;
    *(uint32_t*)0x200005a8 = htobe32(0);
    *(uint32_t*)0x200005ac = htobe32(0);
    memcpy((void*)0x200005b0, "gretap0\000\000\000\000\000\000\000\000\000",
           16);
    memcpy((void*)0x200005c0,
           "sit0\000\000\000\000\000\000\000\000\000\000\000\000", 16);
    *(uint8_t*)0x200005d0 = 0;
    *(uint8_t*)0x200005e0 = 0;
    *(uint16_t*)0x200005f0 = 0;
    *(uint8_t*)0x200005f2 = 0;
    *(uint8_t*)0x200005f3 = 0;
    *(uint32_t*)0x200005f4 = 0;
    *(uint16_t*)0x200005f8 = 0x1f0;
    *(uint16_t*)0x200005fa = 0x218;
    *(uint32_t*)0x200005fc = 0;
    *(uint64_t*)0x20000600 = 0;
    *(uint64_t*)0x20000608 = 0;
    *(uint16_t*)0x20000610 = 0x30;
    memcpy((void*)0x20000612, "addrtype\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000",
           29);
    *(uint8_t*)0x2000062f = 0;
    *(uint16_t*)0x20000630 = 0;
    *(uint16_t*)0x20000632 = 0;
    *(uint32_t*)0x20000634 = 0;
    *(uint32_t*)0x20000638 = 0;
    *(uint16_t*)0x20000640 = 0x150;
    memcpy((void*)0x20000642, "hashlimit\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000",
           29);
    *(uint8_t*)0x2000065f = 2;
    memcpy((void*)0x20000660,
           "bridge0\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000\000\000\000\000\000",
           255);
    *(uint64_t*)0x20000760 = 0;
    *(uint64_t*)0x20000768 = 1;
    *(uint32_t*)0x20000770 = 0;
    *(uint32_t*)0x20000774 = 0xfffffffd;
    *(uint32_t*)0x20000778 = 0;
    *(uint32_t*)0x2000077c = 0x80000000;
    *(uint32_t*)0x20000780 = 0x7fff;
    *(uint8_t*)0x20000784 = 0;
    *(uint8_t*)0x20000785 = 0;
    *(uint64_t*)0x20000788 = 0;
    *(uint16_t*)0x20000790 = 0x28;
    memcpy((void*)0x20000792, "REJECT\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000",
           29);
    *(uint8_t*)0x200007af = 0;
    *(uint32_t*)0x200007b0 = 0;
    *(uint32_t*)0x200007b8 = htobe32(0xe0000001);
    *(uint8_t*)0x200007bc = 0xac;
    *(uint8_t*)0x200007bd = 0x1e;
    *(uint8_t*)0x200007be = 0;
    *(uint8_t*)0x200007bf = 1 + procid * 1;
    *(uint32_t*)0x200007c0 = htobe32(0);
    *(uint32_t*)0x200007c4 = htobe32(0);
    memcpy((void*)0x200007c8, "veth1_to_bridge\000", 16);
    memcpy((void*)0x200007d8, "veth0_vlan\000\000\000\000\000\000", 16);
    *(uint8_t*)0x200007e8 = 0;
    *(uint8_t*)0x200007f8 = 0;
    *(uint16_t*)0x20000808 = 0;
    *(uint8_t*)0x2000080a = 0;
    *(uint8_t*)0x2000080b = 0;
    *(uint32_t*)0x2000080c = 0;
    *(uint16_t*)0x20000810 = 0x70;
    *(uint16_t*)0x20000812 = 0x98;
    *(uint32_t*)0x20000814 = 0;
    *(uint64_t*)0x20000818 = 0;
    *(uint64_t*)0x20000820 = 0;
    *(uint16_t*)0x20000828 = 0x28;
    memcpy((void*)0x2000082a, "REJECT\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000",
           29);
    *(uint8_t*)0x20000847 = 0;
    *(uint32_t*)0x20000848 = 0;
    *(uint8_t*)0x20000850 = 0;
    *(uint8_t*)0x20000851 = 0;
    *(uint8_t*)0x20000852 = 0;
    *(uint8_t*)0x20000853 = 0;
    *(uint8_t*)0x20000854 = 0;
    *(uint8_t*)0x20000855 = 0;
    *(uint8_t*)0x20000856 = 0;
    *(uint8_t*)0x20000857 = 0;
    *(uint8_t*)0x20000858 = 0;
    *(uint8_t*)0x20000859 = 0;
    *(uint8_t*)0x2000085a = 0;
    *(uint8_t*)0x2000085b = 0;
    *(uint8_t*)0x2000085c = 0;
    *(uint8_t*)0x2000085d = 0;
    *(uint8_t*)0x2000085e = 0;
    *(uint8_t*)0x2000085f = 0;
    *(uint8_t*)0x20000860 = 0;
    *(uint8_t*)0x20000861 = 0;
    *(uint8_t*)0x20000862 = 0;
    *(uint8_t*)0x20000863 = 0;
    *(uint8_t*)0x20000864 = 0;
    *(uint8_t*)0x20000865 = 0;
    *(uint8_t*)0x20000866 = 0;
    *(uint8_t*)0x20000867 = 0;
    *(uint8_t*)0x20000868 = 0;
    *(uint8_t*)0x20000869 = 0;
    *(uint8_t*)0x2000086a = 0;
    *(uint8_t*)0x2000086b = 0;
    *(uint8_t*)0x2000086c = 0;
    *(uint8_t*)0x2000086d = 0;
    *(uint8_t*)0x2000086e = 0;
    *(uint8_t*)0x2000086f = 0;
    *(uint8_t*)0x20000870 = 0;
    *(uint8_t*)0x20000871 = 0;
    *(uint8_t*)0x20000872 = 0;
    *(uint8_t*)0x20000873 = 0;
    *(uint8_t*)0x20000874 = 0;
    *(uint8_t*)0x20000875 = 0;
    *(uint8_t*)0x20000876 = 0;
    *(uint8_t*)0x20000877 = 0;
    *(uint8_t*)0x20000878 = 0;
    *(uint8_t*)0x20000879 = 0;
    *(uint8_t*)0x2000087a = 0;
    *(uint8_t*)0x2000087b = 0;
    *(uint8_t*)0x2000087c = 0;
    *(uint8_t*)0x2000087d = 0;
    *(uint8_t*)0x2000087e = 0;
    *(uint8_t*)0x2000087f = 0;
    *(uint8_t*)0x20000880 = 0;
    *(uint8_t*)0x20000881 = 0;
    *(uint8_t*)0x20000882 = 0;
    *(uint8_t*)0x20000883 = 0;
    *(uint8_t*)0x20000884 = 0;
    *(uint8_t*)0x20000885 = 0;
    *(uint8_t*)0x20000886 = 0;
    *(uint8_t*)0x20000887 = 0;
    *(uint8_t*)0x20000888 = 0;
    *(uint8_t*)0x20000889 = 0;
    *(uint8_t*)0x2000088a = 0;
    *(uint8_t*)0x2000088b = 0;
    *(uint8_t*)0x2000088c = 0;
    *(uint8_t*)0x2000088d = 0;
    *(uint8_t*)0x2000088e = 0;
    *(uint8_t*)0x2000088f = 0;
    *(uint8_t*)0x20000890 = 0;
    *(uint8_t*)0x20000891 = 0;
    *(uint8_t*)0x20000892 = 0;
    *(uint8_t*)0x20000893 = 0;
    *(uint8_t*)0x20000894 = 0;
    *(uint8_t*)0x20000895 = 0;
    *(uint8_t*)0x20000896 = 0;
    *(uint8_t*)0x20000897 = 0;
    *(uint8_t*)0x20000898 = 0;
    *(uint8_t*)0x20000899 = 0;
    *(uint8_t*)0x2000089a = 0;
    *(uint8_t*)0x2000089b = 0;
    *(uint8_t*)0x2000089c = 0;
    *(uint8_t*)0x2000089d = 0;
    *(uint8_t*)0x2000089e = 0;
    *(uint8_t*)0x2000089f = 0;
    *(uint8_t*)0x200008a0 = 0;
    *(uint8_t*)0x200008a1 = 0;
    *(uint8_t*)0x200008a2 = 0;
    *(uint8_t*)0x200008a3 = 0;
    *(uint32_t*)0x200008a4 = 0;
    *(uint16_t*)0x200008a8 = 0xa0;
    *(uint16_t*)0x200008aa = 0xe8;
    *(uint32_t*)0x200008ac = 0;
    *(uint64_t*)0x200008b0 = 0;
    *(uint64_t*)0x200008b8 = 0;
    *(uint16_t*)0x200008c0 = 0x30;
    memcpy((void*)0x200008c2, "addrtype\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000",
           29);
    *(uint8_t*)0x200008df = 0;
    *(uint16_t*)0x200008e0 = 0;
    *(uint16_t*)0x200008e2 = 0;
    *(uint32_t*)0x200008e4 = 0;
    *(uint32_t*)0x200008e8 = 0;
    *(uint16_t*)0x200008f0 = 0x48;
    memcpy((void*)0x200008f2, "TEE\000\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000",
           29);
    *(uint8_t*)0x2000090f = 1;
    *(uint8_t*)0x20000910 = 0xac;
    *(uint8_t*)0x20000911 = 0x14;
    *(uint8_t*)0x20000912 = 0x14;
    *(uint8_t*)0x20000913 = 0xbb;
    memcpy((void*)0x20000920, "veth0_to_bond\000\000\000", 16);
    *(uint64_t*)0x20000930 = 0;
    *(uint8_t*)0x20000938 = 0;
    *(uint8_t*)0x20000939 = 0;
    *(uint8_t*)0x2000093a = 0;
    *(uint8_t*)0x2000093b = 0;
    *(uint8_t*)0x2000093c = 0;
    *(uint8_t*)0x2000093d = 0;
    *(uint8_t*)0x2000093e = 0;
    *(uint8_t*)0x2000093f = 0;
    *(uint8_t*)0x20000940 = 0;
    *(uint8_t*)0x20000941 = 0;
    *(uint8_t*)0x20000942 = 0;
    *(uint8_t*)0x20000943 = 0;
    *(uint8_t*)0x20000944 = 0;
    *(uint8_t*)0x20000945 = 0;
    *(uint8_t*)0x20000946 = 0;
    *(uint8_t*)0x20000947 = 0;
    *(uint8_t*)0x20000948 = 0;
    *(uint8_t*)0x20000949 = 0;
    *(uint8_t*)0x2000094a = 0;
    *(uint8_t*)0x2000094b = 0;
    *(uint8_t*)0x2000094c = 0;
    *(uint8_t*)0x2000094d = 0;
    *(uint8_t*)0x2000094e = 0;
    *(uint8_t*)0x2000094f = 0;
    *(uint8_t*)0x20000950 = 0;
    *(uint8_t*)0x20000951 = 0;
    *(uint8_t*)0x20000952 = 0;
    *(uint8_t*)0x20000953 = 0;
    *(uint8_t*)0x20000954 = 0;
    *(uint8_t*)0x20000955 = 0;
    *(uint8_t*)0x20000956 = 0;
    *(uint8_t*)0x20000957 = 0;
    *(uint8_t*)0x20000958 = 0;
    *(uint8_t*)0x20000959 = 0;
    *(uint8_t*)0x2000095a = 0;
    *(uint8_t*)0x2000095b = 0;
    *(uint8_t*)0x2000095c = 0;
    *(uint8_t*)0x2000095d = 0;
    *(uint8_t*)0x2000095e = 0;
    *(uint8_t*)0x2000095f = 0;
    *(uint8_t*)0x20000960 = 0;
    *(uint8_t*)0x20000961 = 0;
    *(uint8_t*)0x20000962 = 0;
    *(uint8_t*)0x20000963 = 0;
    *(uint8_t*)0x20000964 = 0;
    *(uint8_t*)0x20000965 = 0;
    *(uint8_t*)0x20000966 = 0;
    *(uint8_t*)0x20000967 = 0;
    *(uint8_t*)0x20000968 = 0;
    *(uint8_t*)0x20000969 = 0;
    *(uint8_t*)0x2000096a = 0;
    *(uint8_t*)0x2000096b = 0;
    *(uint8_t*)0x2000096c = 0;
    *(uint8_t*)0x2000096d = 0;
    *(uint8_t*)0x2000096e = 0;
    *(uint8_t*)0x2000096f = 0;
    *(uint8_t*)0x20000970 = 0;
    *(uint8_t*)0x20000971 = 0;
    *(uint8_t*)0x20000972 = 0;
    *(uint8_t*)0x20000973 = 0;
    *(uint8_t*)0x20000974 = 0;
    *(uint8_t*)0x20000975 = 0;
    *(uint8_t*)0x20000976 = 0;
    *(uint8_t*)0x20000977 = 0;
    *(uint8_t*)0x20000978 = 0;
    *(uint8_t*)0x20000979 = 0;
    *(uint8_t*)0x2000097a = 0;
    *(uint8_t*)0x2000097b = 0;
    *(uint8_t*)0x2000097c = 0;
    *(uint8_t*)0x2000097d = 0;
    *(uint8_t*)0x2000097e = 0;
    *(uint8_t*)0x2000097f = 0;
    *(uint8_t*)0x20000980 = 0;
    *(uint8_t*)0x20000981 = 0;
    *(uint8_t*)0x20000982 = 0;
    *(uint8_t*)0x20000983 = 0;
    *(uint8_t*)0x20000984 = 0;
    *(uint8_t*)0x20000985 = 0;
    *(uint8_t*)0x20000986 = 0;
    *(uint8_t*)0x20000987 = 0;
    *(uint8_t*)0x20000988 = 0;
    *(uint8_t*)0x20000989 = 0;
    *(uint8_t*)0x2000098a = 0;
    *(uint8_t*)0x2000098b = 0;
    *(uint32_t*)0x2000098c = 0;
    *(uint16_t*)0x20000990 = 0x70;
    *(uint16_t*)0x20000992 = 0x98;
    *(uint32_t*)0x20000994 = 0;
    *(uint64_t*)0x20000998 = 0;
    *(uint64_t*)0x200009a0 = 0;
    *(uint16_t*)0x200009a8 = 0x28;
    memcpy((void*)0x200009aa, "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000",
           29);
    *(uint8_t*)0x200009c7 = 0;
    *(uint32_t*)0x200009c8 = 0xfffffffe;
    syscall(__NR_setsockopt, r[0], 0, 0x40, 0x20000540ul, 0x490ul);
    break;
  case 2:
    res = syscall(__NR_pipe, 0x200003c0ul);
    if (res != -1) {
      r[1] = *(uint32_t*)0x200003c0;
      r[2] = *(uint32_t*)0x200003c4;
    }
    break;
  case 3:
    *(uint64_t*)0x20000300 = 0;
    *(uint32_t*)0x20000308 = 0;
    *(uint64_t*)0x20000310 = 0x200002c0;
    *(uint64_t*)0x200002c0 = 0x20000000;
    memcpy((void*)0x20000000,
           "\x60\x00\x00\x00\x30\x00\x3d\xfa\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x4c\x00\x01\x00\x48\x00\x01\x00\x07\x00\x01\x00"
           "\x78\x74\x00\x00\x38\x00\x02\x80\x2a\x00\x06\x00\x26\x00\x00\x00"
           "\x6e\x67\x6c\x65\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x47\x3c\x06\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x08\x00\x02",
           87);
    *(uint64_t*)0x200002c8 = 1;
    *(uint64_t*)0x20000318 = 1;
    *(uint64_t*)0x20000320 = 0;
    *(uint64_t*)0x20000328 = 0;
    *(uint32_t*)0x20000330 = 0;
    syscall(__NR_sendmsg, -1, 0x20000300ul, 0ul);
    break;
  case 4:
    syscall(__NR_write, r[2], 0x20000000ul, 0xfffffeccul);
    break;
  case 5:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0);
    if (res != -1)
      r[3] = res;
    break;
  case 6:
    syscall(__NR_splice, r[1], 0ul, r[3], 0ul, 0x4ffe0ul, 0ul);
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
