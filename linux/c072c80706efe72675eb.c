// possible deadlock in sk_clone_lock
// https://syzkaller.appspot.com/bug?id=c072c80706efe72675eb
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
      {"/proc/sys/vm/nr_overcommit_hugepages", "4"},
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
  int collide = 0;
again:
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

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy(
        (void*)0x20000900,
        "Y\377\377\000\000\000\000\000K\262\002\200B\351\350\314\336\006\000l"
        "\250\032J\257\262M\272\270_\005U\315<|>\236\354^\016\276\030+-"
        "\233\2113\002\000\246\037+"
        "\263\305\220z5\340\337i\267\237\264QW\311\311\222\003\t\000\000\000"
        "\000\000\262\017\356\276\f8\314\177\000\000\000Z\201\000\000\000\000"
        "\030I\023\361\242x\004\201R\3245R\256\204\315N\367\366\b~\355_M^dQ:"
        "\274\257q\210\031nSF|;]\341A\214\212\230\327|\334LF\r\261\375\277!"
        "\307u\314P\335\023~\211\317\205\312\240%"
        "\306\307\021\000\000\000\000\000\000\000?M9\\\av~\'\331\260\254dya]"
        "8\235\267\v\3639\305{\234!\017/"
        "\270o8\271\215\031\342\312\001y\203\347\ng\207\3313<"
        "u\276\352g\322\004e\362s\336}o#\220\332\234s,r\375:"
        "\313\307\vTmPZJ\254\213\320\300\274RK\313\0368\316\275[N\260\264<"
        "\353\206[\354\217\024\n\203\236&\\\267!\355\240\303,"
        "\220m\017\000\000\000\000\000\000\377\320\263\224\000\000\000\000\000"
        "\000\000\235H\370\377Ro\243W\341\355\216S\256X\223\177\323\271a\344"
        "\340\275\331\365\344e\371\312m\343\003\344\255\325&"
        "A\002\310\251\376\313\207\331\323\214\f\3008T~\304\275\220?"
        "\037\234\2330\031 "
        "\237\321p`\300/"
        "\000u\031\035\254\350%"
        "\267\t\2374\273\211\351\301\201\260\326\367\005\026\271\351\334~\323-"
        "s\346;\224\034^\006\332v;\314D\225sj\340\325\276S\254\345N\352\236yl[["
        "\3165\356\005\234\226\253J^c\367\031\264\330\344\207\235p\352?/"
        "\313\232\2525\'Q\fuG\201\212@\227M=v\376\a\253p\363\223\235\177@"
        "9\300\322\361#\347N\211\370\350+"
        "\241\202\023\262\251\357\245\370\213k\263\236v\222\016\321\037\027Dx["
        "\255\260\235/"
        "\226\3442mO\237h\352\363\024\315\226\211_)\033\342\326\b\224b\257;"
        "\222T\257z\224\237G\373\375|$k\003\327;\232y\356\371W?\263\371\370I."
        "E4\303eWq\372\":$\320\027Q\224\365\345mk\216&\361;#z\357*"
        "\316\231\004\271\220\274\271#-"
        "\215i\256\305\337\023\322K\213\365\f\262x\335\325\264\251^~O<"
        "\022\343\303P|7\251\310\261r0Z\230\207^"
        "\310C\033\226\264lsyF\305\274\377OE\351\3270\347\376p\203\247\241\331"
        "\344\272\222\027\277\340\341\001\267\213\030j\031n\310\377\351\364\364"
        "\317 "
        "a\327_w\017\235F\256\314A-/"
        "\203\253U\325\2778\244O\264\357\310\314H\321=\276\265O\025\307;"
        "Lb\316\032\314\230\350\351e\340\302N\347\327\035\222\207F8\236\367\335"
        "\332W\365X\200\240f\316o\330\177\300\226\bSB\316&\004$"
        "\303\3431U\204\202\360{i\035\002\020\306C\001^\317\223?"
        "w\001\204\240\326\242\020\244\374G\253D\326dGZ\262Cx\033\375D9\027\346"
        "\312uM\343\005)\342[\342j$F1n@\336\n%t.[\255\265\f<"
        "o\333\244\220\304\320I\246\226\241\000\273\335\224\000t/"
        "1YekG\177\275rI\036\2124=\242\351\3701\255\356\365\244\005#"
        "\177\250\037\024\202ih\371?\003Q\365\304\225\231 "
        "\247v~:$"
        "\002O\343\252\360o\353k\'\322q\020\036\224n\327w\261\310G\211\364\214t"
        "\371R\205\0338OE\004\232\330\200\020x\277\n0\b,r\306r\367="
        "\376\301\026\346\356\273:-h\272\213\024\232?\216\003;"
        "\345\004\330\251\232\323E(\177|\255U\257e\207V.i4$6U,R\233\n{"
        "\'\3761\304\2609",
        965);
    res = syscall(__NR_memfd_create, 0x20000900ul, 4ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    syscall(__NR_mmap, 0x20200000ul, 0x400017ul, 0x2000006ul, 0x2012ul, r[0],
            0ul);
    break;
  case 2:
    res = syscall(__NR_socket, 2ul, 1ul, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    *(uint16_t*)0x200001c0 = 2;
    *(uint16_t*)0x200001c2 = htobe16(0x4e20);
    *(uint32_t*)0x200001c4 = htobe32(0xe0000002);
    syscall(__NR_bind, r[1], 0x200001c0ul, 0x10ul);
    break;
  case 4:
    *(uint32_t*)0x20000000 = 1;
    syscall(__NR_setsockopt, r[1], 1, 0x3c, 0x20000000ul, 0x1039cul);
    break;
  case 5:
    *(uint16_t*)0x20000080 = 2;
    *(uint16_t*)0x20000082 = htobe16(0x4e20);
    *(uint32_t*)0x20000084 = htobe32(0);
    syscall(__NR_sendto, r[1], 0ul, 0ul, 0x20000000ul, 0x20000080ul, 0x10ul);
    break;
  case 6:
    memcpy(
        (void*)0x20000140,
        "\x7c\x0d\x11\x13\x17\xb1\xff\x8e\xc8\xf2\x9f\x81\x31\x9e\xc5\xb1\x0d"
        "\x0d\x41\x3f\xfa\xef\xd9\x44\x8d\xbe\xf1\xff\xb4\xe3\xa6\xaf\x87\x13"
        "\x15\x12\xda\x52\x8f\x62\x35\xfe\x35\xd3\x05\x3b\x5c\xb4\x87\x7c\x1b"
        "\x89\xbb\xf8\x3f\x6d\x74\x9d\x5b\x45\x67\x96\x01\xa3\xab\xdc\x20\x4c"
        "\xf1\xad\x79\x72\x52\xbb\x0c\x7b\x43\xe3\xe9\x76\x32\x21\xcb\x2c\x8d"
        "\x76\xf8\x24\xf4\xbe\xa3\x38\xf5\x3a\xa7\x57\x03\xf4\x65\xe5\x0e\x43"
        "\x4e\x21\x93\x90\x0c\x39\xab\x79\xd3\x7c\xc0\xce\x7a",
        115);
    syscall(__NR_sendto, r[1], 0x20000140ul, 0xffffffffffffffd0ul, 0x401c005ul,
            0ul, 0xffffffffffffff36ul);
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
