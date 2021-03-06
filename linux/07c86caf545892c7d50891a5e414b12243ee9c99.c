// KASAN: wild-memory-access Read in sg_read
// https://syzkaller.appspot.com/bug?id=07c86caf545892c7d50891a5e414b12243ee9c99
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static uintptr_t syz_open_procfs(uintptr_t a0, uintptr_t a1)
{

  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == (uintptr_t)-1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

uint64_t r[6] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff, 0x0};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000100, "./cgroup.net", 13);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000100, 0x200002, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    syscall(__NR_fchdir, r[0]);
    break;
  case 2:
    res = syscall(__NR_pipe2, 0x20000000, 0);
    if (res != -1)
      r[1] = *(uint32_t*)0x20000004;
    break;
  case 3:
    syscall(__NR_gettid);
    break;
  case 4:
    memcpy((void*)0x2076c000, "pagemap", 8);
    res = syz_open_procfs(0, 0x2076c000);
    if (res != -1)
      r[2] = res;
    break;
  case 5:
    *(uint64_t*)0x20014000 = 0;
    syscall(__NR_sendfile, r[1], r[2], 0x20014000, 0xffff);
    break;
  case 6:
    *(uint8_t*)0x20000040 = 0xfe;
    *(uint8_t*)0x20000041 = 0x80;
    *(uint8_t*)0x20000042 = 0;
    *(uint8_t*)0x20000043 = 0;
    *(uint8_t*)0x20000044 = 0;
    *(uint8_t*)0x20000045 = 0;
    *(uint8_t*)0x20000046 = 0;
    *(uint8_t*)0x20000047 = 0;
    *(uint8_t*)0x20000048 = 0;
    *(uint8_t*)0x20000049 = 0;
    *(uint8_t*)0x2000004a = 0;
    *(uint8_t*)0x2000004b = 0;
    *(uint8_t*)0x2000004c = 0;
    *(uint8_t*)0x2000004d = 0;
    *(uint8_t*)0x2000004e = 0;
    *(uint8_t*)0x2000004f = 0xbb;
    *(uint32_t*)0x20000050 = 0x200;
    *(uint8_t*)0x20000054 = 0;
    *(uint8_t*)0x20000055 = 0;
    *(uint16_t*)0x20000056 = 0;
    *(uint16_t*)0x20000058 = 0;
    *(uint16_t*)0x2000005a = 2;
    *(uint32_t*)0x2000005c = 0x100;
    syscall(__NR_setsockopt, -1, 0x29, 0x20, 0x20000040, 0x20);
    break;
  case 7:
    memcpy((void*)0x20000080, "./file0", 8);
    res = syscall(__NR_creat, 0x20000080, 0);
    if (res != -1)
      r[3] = res;
    break;
  case 8:
    memcpy((void*)0x20000040, "threaded", 9);
    syscall(__NR_write, -1, 0x20000040, 0xffc5);
    break;
  case 9:
    memcpy((void*)0x20000140, "/dev/sg#", 9);
    syz_open_dev(0x20000140, 0, 0);
    break;
  case 10:
    memcpy((void*)0x20000880, "/dev/loop-control", 18);
    syscall(__NR_openat, 0xffffffffffffff9c, 0x20000880, 0xfffffffffffffffe, 0);
    break;
  case 11:
    *(uint32_t*)0x20000200 = 0xfffffff9;
    syscall(__NR_getsockopt, -1, 0, 0, 0x20000540, 0x20000200);
    break;
  case 12:
    *(uint64_t*)0x20000500 = 0x20000180;
    *(uint16_t*)0x20000180 = 0x10;
    *(uint16_t*)0x20000182 = 0;
    *(uint32_t*)0x20000184 = 0;
    *(uint32_t*)0x20000188 = 0x40000;
    *(uint32_t*)0x20000508 = 0xc;
    *(uint64_t*)0x20000510 = 0x200002c0;
    *(uint64_t*)0x200002c0 = 0x20000c00;
    *(uint64_t*)0x200002c8 = 0;
    *(uint64_t*)0x20000518 = 1;
    *(uint64_t*)0x20000520 = 0;
    *(uint64_t*)0x20000528 = 0;
    *(uint32_t*)0x20000530 = 0;
    syscall(__NR_sendmsg, -1, 0x20000500, 0x91);
    break;
  case 13:
    syscall(__NR_fallocate, r[3], 3, 0, 0xffff);
    break;
  case 14:
    *(uint32_t*)0x20000000 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 6;
    syscall(__NR_setsockopt, -1, 0, 0x48a, 0x20000000, 0xffffffffffffff15);
    break;
  case 15:
    syscall(__NR_ioctl, -1, 0x4c07);
    break;
  case 16:
    *(uint8_t*)0x20000280 = 0;
    *(uint8_t*)0x20000281 = 0x1a;
    *(uint16_t*)0x20000282 = 6;
    *(uint32_t*)0x20000284 = 0x400;
    memcpy((void*)0x20000288, "\xfd\x6e\x10\xe9\xd1\x75\x4c\xd7\x3b\x0a\x3a\xa4"
                              "\x53\xf0\xc6\x23\x85\x89\xbe\x94\x84\xb9\x80\x3f"
                              "\x4d\x1a\xad\x7d\xd5\xe7\xea\x14",
           32);
    syscall(__NR_ioctl, -1, 0x40284504, 0x20000280);
    break;
  case 17:
    memcpy((void*)0x20001000, "/dev/sg#", 9);
    res = syz_open_dev(0x20001000, 0, 2);
    if (res != -1)
      r[4] = res;
    break;
  case 18:
    syscall(__NR_ioctl, r[4], 0x2283, 0x20979fff);
    break;
  case 19:
    res = syscall(__NR_clock_gettime, 0, 0x20000040);
    if (res != -1)
      r[5] = *(uint64_t*)0x20000040;
    break;
  case 20:
    syscall(__NR_clock_gettime, 0, 0x200000c0);
    break;
  case 21:
    *(uint64_t*)0x200003c0 = 0x20000600;
    *(uint64_t*)0x200003c8 = 0xdc;
    *(uint64_t*)0x200003d0 = 0x20000700;
    *(uint64_t*)0x200003d8 = 0xc6;
    *(uint64_t*)0x200003e0 = 0x200009c0;
    *(uint64_t*)0x200003e8 = 0xf8;
    syscall(__NR_readv, r[4], 0x200003c0, 3);
    break;
  case 22:
    *(uint64_t*)0x200001c0 = r[5];
    *(uint64_t*)0x200001c8 = 0;
    *(uint16_t*)0x200001d0 = 1;
    *(uint16_t*)0x200001d2 = 0;
    *(uint32_t*)0x200001d4 = 0;
    *(uint64_t*)0x200001d8 = 0;
    *(uint64_t*)0x200001e0 = 0;
    *(uint16_t*)0x200001e8 = 7;
    *(uint16_t*)0x200001ea = 0;
    *(uint32_t*)0x200001ec = 6;
    *(uint64_t*)0x200001f0 = 0;
    *(uint64_t*)0x200001f8 = 0;
    *(uint16_t*)0x20000200 = 3;
    *(uint16_t*)0x20000202 = 5;
    *(uint32_t*)0x20000204 = 0x3a3d;
    *(uint64_t*)0x20000208 = 0;
    *(uint64_t*)0x20000210 = 0;
    *(uint16_t*)0x20000218 = 0x20;
    *(uint16_t*)0x2000021a = 0;
    *(uint32_t*)0x2000021c = 5;
    *(uint64_t*)0x20000220 = 0;
    *(uint64_t*)0x20000228 = 0;
    *(uint16_t*)0x20000230 = 0;
    *(uint16_t*)0x20000232 = 7;
    *(uint32_t*)0x20000234 = 0;
    *(uint64_t*)0x20000238 = 0;
    *(uint64_t*)0x20000240 = 0;
    *(uint16_t*)0x20000248 = 6;
    *(uint16_t*)0x2000024a = 0;
    *(uint32_t*)0x2000024c = 9;
    syscall(__NR_write, r[4], 0x200001c0, 0x90);
    break;
  }
}

void execute_one()
{
  execute(23);
  collide = 1;
  execute(23);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
