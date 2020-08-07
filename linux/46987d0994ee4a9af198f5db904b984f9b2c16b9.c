// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
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

uint64_t r[4] = {0x0, 0x0, 0xffffffffffffffff, 0x0};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_clock_gettime, 0, 0x20001ac0);
    if (res != -1) {
      r[0] = *(uint64_t*)0x20001ac0;
      r[1] = *(uint64_t*)0x20001ac8;
    }
    break;
  case 1:
    *(uint64_t*)0x20001a00 = 0;
    *(uint32_t*)0x20001a08 = 0;
    *(uint64_t*)0x20001a10 = 0x20000040;
    *(uint64_t*)0x20000040 = 0x20000100;
    *(uint64_t*)0x20000048 = 0x61;
    *(uint64_t*)0x20000050 = 0x20000180;
    *(uint64_t*)0x20000058 = 0xb0;
    *(uint64_t*)0x20000060 = 0x20000240;
    *(uint64_t*)0x20000068 = 0xce;
    *(uint64_t*)0x20000070 = 0x20000340;
    *(uint64_t*)0x20000078 = 0x7a;
    *(uint64_t*)0x20001a18 = 4;
    *(uint64_t*)0x20001a20 = 0x200003c0;
    *(uint64_t*)0x20001a28 = 0x6d;
    *(uint32_t*)0x20001a30 = 0x101;
    *(uint32_t*)0x20001a38 = 0xfffffc01;
    *(uint64_t*)0x20001a40 = 0;
    *(uint32_t*)0x20001a48 = 0;
    *(uint64_t*)0x20001a50 = 0x20000580;
    *(uint64_t*)0x20000580 = 0x20000440;
    *(uint64_t*)0x20000588 = 0xbd;
    *(uint64_t*)0x20000590 = 0x20000500;
    *(uint64_t*)0x20000598 = 0x70;
    *(uint64_t*)0x20001a58 = 2;
    *(uint64_t*)0x20001a60 = 0x200005c0;
    *(uint64_t*)0x20001a68 = 0xf0;
    *(uint32_t*)0x20001a70 = 8;
    *(uint32_t*)0x20001a78 = 0xc2;
    *(uint64_t*)0x20001a80 = 0x200006c0;
    *(uint32_t*)0x20001a88 = 0x80;
    *(uint64_t*)0x20001a90 = 0x20001900;
    *(uint64_t*)0x20001900 = 0x20000740;
    *(uint64_t*)0x20001908 = 0x1000;
    *(uint64_t*)0x20001910 = 0x20001740;
    *(uint64_t*)0x20001918 = 0x86;
    *(uint64_t*)0x20001920 = 0x20001800;
    *(uint64_t*)0x20001928 = 0x72;
    *(uint64_t*)0x20001930 = 0x20001880;
    *(uint64_t*)0x20001938 = 0x52;
    *(uint64_t*)0x20001a98 = 4;
    *(uint64_t*)0x20001aa0 = 0x20001940;
    *(uint64_t*)0x20001aa8 = 0xa6;
    *(uint32_t*)0x20001ab0 = 0x8001;
    *(uint32_t*)0x20001ab8 = 9;
    *(uint64_t*)0x20001b00 = r[0];
    *(uint64_t*)0x20001b08 = r[1] + 30000000;
    res =
        syscall(__NR_recvmmsg, 0xffffff9c, 0x20001a00, 3, 0x10000, 0x20001b00);
    if (res != -1)
      r[2] = *(uint32_t*)0x200006ca;
    break;
  case 2:
    *(uint32_t*)0x20001b40 = 7;
    syscall(__NR_setsockopt, r[2], 0x11, 0x64, 0x20001b40, 4);
    break;
  case 3:
    *(uint32_t*)0x20000080 = 0;
    *(uint32_t*)0x20000084 = 0x70;
    *(uint8_t*)0x20000088 = 0;
    *(uint8_t*)0x20000089 = 0;
    *(uint8_t*)0x2000008a = 0;
    *(uint8_t*)0x2000008b = 0;
    *(uint32_t*)0x2000008c = 0;
    *(uint64_t*)0x20000090 = 0x7fff;
    *(uint64_t*)0x20000098 = 0;
    *(uint64_t*)0x200000a0 = 0;
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 29, 35);
    *(uint32_t*)0x200000b0 = 0;
    *(uint32_t*)0x200000b4 = 0;
    *(uint64_t*)0x200000b8 = 0x20000000;
    *(uint64_t*)0x200000c0 = 0;
    *(uint64_t*)0x200000c8 = 0;
    *(uint64_t*)0x200000d0 = 0;
    *(uint32_t*)0x200000d8 = 0;
    *(uint32_t*)0x200000dc = 0;
    *(uint64_t*)0x200000e0 = 0;
    *(uint32_t*)0x200000e8 = 0;
    *(uint16_t*)0x200000ec = 0;
    *(uint16_t*)0x200000ee = 0;
    syscall(__NR_perf_event_open, 0x20000080, 0, -1, -1, 0);
    break;
  case 4:
    syscall(__NR_pwritev, -1, 0x20000300, 0, 0);
    break;
  case 5:
    syscall(__NR_ioctl, -1, 0x4c06, -1);
    break;
  case 6:
    res = syscall(__NR_shmget, 0, 0x3000, 0, 0x20ffb000);
    if (res != -1)
      r[3] = res;
    break;
  case 7:
    syscall(__NR_shmctl, r[3], 0);
    break;
  case 8:
    syscall(__NR_shmctl, r[3], 0);
    break;
  }
}

void execute_one()
{
  execute(9);
  collide = 1;
  execute(9);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}