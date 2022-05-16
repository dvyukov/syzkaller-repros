// WARNING in kvm_mmu_notifier_invalidate_range_start (2)
// https://syzkaller.appspot.com/bug?id=6bde52d89cfdf9f61425
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000080, "/dev/kvm\000", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000080ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_ioctl, r[0], 0xae01, 0ul);
  memcpy((void*)0x20000000, "./bus\000", 6);
  syscall(__NR_creat, 0x20000000ul, 0ul);
  memcpy((void*)0x20000780, "./bus\000", 6);
  res = syscall(__NR_open, 0x20000780ul, 0ul, 0ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_mmap, 0x20000000ul, 0x600000ul, 0ul, 0x4002011ul, r[1], 0ul);
  syscall(__NR_mremap, 0x2000d000ul, 0xfffffffffffffe74ul, 0x1000ul, 3ul,
          0x20007000ul);
  return 0;
}
