// general protection fault in rdma_destroy_id
// https://syzkaller.appspot.com/bug?id=556e390bf27cfe4ab863fa16072c21c1a0d19227
// status:dup
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

static void test();

void loop()
{
  while (1) {
    test();
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000040, "/dev/vcsa#", 11);
    res = syz_open_dev(0x20000040, 3, 0x80);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint16_t*)0x20000080 = 1;
    *(uint8_t*)0x20000082 = 0;
    *(uint32_t*)0x20000084 = 0x4e24;
    syscall(__NR_connect, r[0], 0x20000080, 0x6e);
    break;
  case 2:
    syscall(__NR_clock_gettime, 0, 0x20000100);
    break;
  case 3:
    *(uint32_t*)0x200001c0 = 0x80000000;
    *(uint32_t*)0x200001c4 = 0x80000001;
    *(uint32_t*)0x200001c8 = 4;
    *(uint64_t*)0x200001d0 = 0;
    *(uint64_t*)0x200001d8 = 0x989680;
    *(uint32_t*)0x200001e0 = 0x75a;
    *(uint32_t*)0x200001e4 = 0;
    *(uint8_t*)0x200001e8 = 0;
    *(uint8_t*)0x200001e9 = 0;
    *(uint8_t*)0x200001ea = 0;
    *(uint8_t*)0x200001eb = 0;
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
    *(uint8_t*)0x20000200 = 0;
    *(uint8_t*)0x20000201 = 0;
    *(uint8_t*)0x20000202 = 0;
    *(uint8_t*)0x20000203 = 0;
    *(uint8_t*)0x20000204 = 0;
    *(uint8_t*)0x20000205 = 0;
    *(uint8_t*)0x20000206 = 0;
    *(uint8_t*)0x20000207 = 0;
    *(uint8_t*)0x20000208 = 0;
    *(uint8_t*)0x20000209 = 0;
    *(uint8_t*)0x2000020a = 0;
    *(uint8_t*)0x2000020b = 0;
    *(uint8_t*)0x2000020c = 0;
    *(uint8_t*)0x2000020d = 0;
    *(uint8_t*)0x2000020e = 0;
    *(uint8_t*)0x2000020f = 0;
    *(uint8_t*)0x20000210 = 0;
    *(uint8_t*)0x20000211 = 0;
    *(uint8_t*)0x20000212 = 0;
    *(uint8_t*)0x20000213 = 0;
    *(uint8_t*)0x20000214 = 0;
    *(uint8_t*)0x20000215 = 0;
    *(uint8_t*)0x20000216 = 0;
    *(uint8_t*)0x20000217 = 0;
    *(uint8_t*)0x20000218 = 0;
    *(uint8_t*)0x20000219 = 0;
    *(uint8_t*)0x2000021a = 0;
    *(uint8_t*)0x2000021b = 0;
    *(uint8_t*)0x2000021c = 0;
    *(uint8_t*)0x2000021d = 0;
    *(uint8_t*)0x2000021e = 0;
    *(uint8_t*)0x2000021f = 0;
    *(uint8_t*)0x20000220 = 0;
    *(uint8_t*)0x20000221 = 0;
    *(uint8_t*)0x20000222 = 0;
    *(uint8_t*)0x20000223 = 0;
    *(uint8_t*)0x20000224 = 0;
    *(uint8_t*)0x20000225 = 0;
    *(uint8_t*)0x20000226 = 0;
    *(uint8_t*)0x20000227 = 0;
    syscall(__NR_ioctl, r[0], 0xc05c5340, 0x200001c0);
    break;
  case 4:
    memcpy((void*)0x20000180, "/dev/infiniband/rdma_cm", 24);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000180, 2, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 5:
    *(uint32_t*)0x20002940 = 0;
    *(uint16_t*)0x20002944 = 0x18;
    *(uint16_t*)0x20002946 = 0xfa00;
    *(uint64_t*)0x20002948 = 0;
    *(uint64_t*)0x20002950 = 0x20002900;
    *(uint16_t*)0x20002958 = 0x106;
    *(uint8_t*)0x2000295a = 0;
    *(uint8_t*)0x2000295b = 0;
    *(uint8_t*)0x2000295c = 0;
    *(uint8_t*)0x2000295d = 0;
    *(uint8_t*)0x2000295e = 0;
    *(uint8_t*)0x2000295f = 0;
    res = syscall(__NR_write, r[1], 0x20002940, 0x20);
    if (res != -1)
      r[2] = *(uint32_t*)0x20002900;
    break;
  case 6:
    *(uint32_t*)0x20000000 = 1;
    *(uint16_t*)0x20000004 = 0;
    *(uint16_t*)0x20000006 = 0xfa00;
    *(uint64_t*)0x20000008 = 0x200013c0;
    *(uint32_t*)0x20000010 = r[2];
    *(uint32_t*)0x20000014 = 0;
    syscall(__NR_write, r[1], 0x20000000, 0xa4);
    break;
  }
}

void test()
{
  execute(7);
  collide = 1;
  execute(7);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
