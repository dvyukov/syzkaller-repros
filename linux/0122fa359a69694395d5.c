// WARNING in input_mt_init_slots
// https://syzkaller.appspot.com/bug?id=0122fa359a69694395d5
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000000, "/dev/uinput\000", 12);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000340 = 0x2f;
  *(uint32_t*)0x20000344 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint32_t*)0x2000034c = 0;
  *(uint32_t*)0x20000350 = 0;
  *(uint32_t*)0x20000354 = 0;
  *(uint32_t*)0x20000358 = 0;
  syscall(__NR_ioctl, r[0], 0x401c5504, 0x20000340ul);
  syscall(__NR_ioctl, r[0], 0x40045564, 3ul);
  memcpy((void*)0x200005c0,
         "syz0\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000",
         80);
  *(uint16_t*)0x20000610 = 0;
  *(uint16_t*)0x20000612 = 0;
  *(uint16_t*)0x20000614 = 0;
  *(uint16_t*)0x20000616 = 0;
  *(uint32_t*)0x20000618 = 0;
  *(uint32_t*)0x2000061c = 0;
  *(uint32_t*)0x20000620 = 0;
  *(uint32_t*)0x20000624 = 0;
  *(uint32_t*)0x20000628 = 0;
  *(uint32_t*)0x2000062c = 0;
  *(uint32_t*)0x20000630 = 0;
  *(uint32_t*)0x20000634 = 0;
  *(uint32_t*)0x20000638 = 0;
  *(uint32_t*)0x2000063c = 0;
  *(uint32_t*)0x20000640 = 0;
  *(uint32_t*)0x20000644 = 0;
  *(uint32_t*)0x20000648 = 0;
  *(uint32_t*)0x2000064c = 0;
  *(uint32_t*)0x20000650 = 0;
  *(uint32_t*)0x20000654 = 0;
  *(uint32_t*)0x20000658 = 0;
  *(uint32_t*)0x2000065c = 0;
  *(uint32_t*)0x20000660 = 0;
  *(uint32_t*)0x20000664 = 0;
  *(uint32_t*)0x20000668 = 0;
  *(uint32_t*)0x2000066c = 0;
  *(uint32_t*)0x20000670 = 0;
  *(uint32_t*)0x20000674 = 0;
  *(uint32_t*)0x20000678 = 0;
  *(uint32_t*)0x2000067c = 0;
  *(uint32_t*)0x20000680 = 0;
  *(uint32_t*)0x20000684 = 0;
  *(uint32_t*)0x20000688 = 0;
  *(uint32_t*)0x2000068c = 0;
  *(uint32_t*)0x20000690 = 0;
  *(uint32_t*)0x20000694 = 0;
  *(uint32_t*)0x20000698 = 0;
  *(uint32_t*)0x2000069c = 0;
  *(uint32_t*)0x200006a0 = 0;
  *(uint32_t*)0x200006a4 = 0;
  *(uint32_t*)0x200006a8 = 0;
  *(uint32_t*)0x200006ac = 0;
  *(uint32_t*)0x200006b0 = 0;
  *(uint32_t*)0x200006b4 = 0;
  *(uint32_t*)0x200006b8 = 0;
  *(uint32_t*)0x200006bc = 0;
  *(uint32_t*)0x200006c0 = 0;
  *(uint32_t*)0x200006c4 = 0;
  *(uint32_t*)0x200006c8 = 0;
  *(uint32_t*)0x200006cc = 0;
  *(uint32_t*)0x200006d0 = 0;
  *(uint32_t*)0x200006d4 = 0;
  *(uint32_t*)0x200006d8 = 0x10000;
  *(uint32_t*)0x200006dc = 0;
  *(uint32_t*)0x200006e0 = 0;
  *(uint32_t*)0x200006e4 = 0;
  *(uint32_t*)0x200006e8 = 0;
  *(uint32_t*)0x200006ec = 0;
  *(uint32_t*)0x200006f0 = 0;
  *(uint32_t*)0x200006f4 = 0;
  *(uint32_t*)0x200006f8 = 0;
  *(uint32_t*)0x200006fc = 0;
  *(uint32_t*)0x20000700 = 0;
  *(uint32_t*)0x20000704 = 0;
  *(uint32_t*)0x20000708 = 0;
  *(uint32_t*)0x2000070c = 0;
  *(uint32_t*)0x20000710 = 0;
  *(uint32_t*)0x20000714 = 0;
  *(uint32_t*)0x20000718 = 0;
  *(uint32_t*)0x2000071c = 0;
  *(uint32_t*)0x20000720 = 0;
  *(uint32_t*)0x20000724 = 0;
  *(uint32_t*)0x20000728 = 0;
  *(uint32_t*)0x2000072c = 0;
  *(uint32_t*)0x20000730 = 0;
  *(uint32_t*)0x20000734 = 0;
  *(uint32_t*)0x20000738 = 0;
  *(uint32_t*)0x2000073c = 0;
  *(uint32_t*)0x20000740 = 0;
  *(uint32_t*)0x20000744 = 0;
  *(uint32_t*)0x20000748 = 0;
  *(uint32_t*)0x2000074c = 0;
  *(uint32_t*)0x20000750 = 0;
  *(uint32_t*)0x20000754 = 0;
  *(uint32_t*)0x20000758 = 0;
  *(uint32_t*)0x2000075c = 0;
  *(uint32_t*)0x20000760 = 0;
  *(uint32_t*)0x20000764 = 0;
  *(uint32_t*)0x20000768 = 0;
  *(uint32_t*)0x2000076c = 0;
  *(uint32_t*)0x20000770 = 0;
  *(uint32_t*)0x20000774 = 0;
  *(uint32_t*)0x20000778 = 0;
  *(uint32_t*)0x2000077c = 0;
  *(uint32_t*)0x20000780 = 0;
  *(uint32_t*)0x20000784 = 0;
  *(uint32_t*)0x20000788 = 0;
  *(uint32_t*)0x2000078c = 0;
  *(uint32_t*)0x20000790 = 0;
  *(uint32_t*)0x20000794 = 0;
  *(uint32_t*)0x20000798 = 0;
  *(uint32_t*)0x2000079c = 0;
  *(uint32_t*)0x200007a0 = 0;
  *(uint32_t*)0x200007a4 = 0;
  *(uint32_t*)0x200007a8 = 0;
  *(uint32_t*)0x200007ac = 0;
  *(uint32_t*)0x200007b0 = 0;
  *(uint32_t*)0x200007b4 = 0;
  *(uint32_t*)0x200007b8 = 0;
  *(uint32_t*)0x200007bc = 0;
  *(uint32_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c4 = 0;
  *(uint32_t*)0x200007c8 = 0;
  *(uint32_t*)0x200007cc = 0;
  *(uint32_t*)0x200007d0 = 0;
  *(uint32_t*)0x200007d4 = 0;
  *(uint32_t*)0x200007d8 = 0;
  *(uint32_t*)0x200007dc = 0;
  *(uint32_t*)0x200007e0 = 0;
  *(uint32_t*)0x200007e4 = 0;
  *(uint32_t*)0x200007e8 = 0;
  *(uint32_t*)0x200007ec = 0;
  *(uint32_t*)0x200007f0 = 0;
  *(uint32_t*)0x200007f4 = 0;
  *(uint32_t*)0x200007f8 = 0;
  *(uint32_t*)0x200007fc = 0;
  *(uint32_t*)0x20000800 = 0;
  *(uint32_t*)0x20000804 = 0;
  *(uint32_t*)0x20000808 = 0;
  *(uint32_t*)0x2000080c = 0;
  *(uint32_t*)0x20000810 = 0;
  *(uint32_t*)0x20000814 = 0;
  *(uint32_t*)0x20000818 = 0;
  *(uint32_t*)0x2000081c = 0;
  *(uint32_t*)0x20000820 = 0;
  *(uint32_t*)0x20000824 = 0;
  *(uint32_t*)0x20000828 = 0;
  *(uint32_t*)0x2000082c = 0;
  *(uint32_t*)0x20000830 = 0;
  *(uint32_t*)0x20000834 = 0;
  *(uint32_t*)0x20000838 = 0;
  *(uint32_t*)0x2000083c = 0;
  *(uint32_t*)0x20000840 = 0;
  *(uint32_t*)0x20000844 = 0;
  *(uint32_t*)0x20000848 = 0;
  *(uint32_t*)0x2000084c = 0;
  *(uint32_t*)0x20000850 = 0;
  *(uint32_t*)0x20000854 = 0;
  *(uint32_t*)0x20000858 = 0;
  *(uint32_t*)0x2000085c = 0;
  *(uint32_t*)0x20000860 = 0;
  *(uint32_t*)0x20000864 = 0;
  *(uint32_t*)0x20000868 = 0;
  *(uint32_t*)0x2000086c = 0;
  *(uint32_t*)0x20000870 = 0;
  *(uint32_t*)0x20000874 = 0;
  *(uint32_t*)0x20000878 = 0;
  *(uint32_t*)0x2000087c = 0;
  *(uint32_t*)0x20000880 = 0;
  *(uint32_t*)0x20000884 = 0;
  *(uint32_t*)0x20000888 = 0;
  *(uint32_t*)0x2000088c = 0;
  *(uint32_t*)0x20000890 = 0;
  *(uint32_t*)0x20000894 = 0;
  *(uint32_t*)0x20000898 = 0;
  *(uint32_t*)0x2000089c = 0;
  *(uint32_t*)0x200008a0 = 0;
  *(uint32_t*)0x200008a4 = 0;
  *(uint32_t*)0x200008a8 = 0;
  *(uint32_t*)0x200008ac = 0;
  *(uint32_t*)0x200008b0 = 0;
  *(uint32_t*)0x200008b4 = 0;
  *(uint32_t*)0x200008b8 = 0;
  *(uint32_t*)0x200008bc = 0;
  *(uint32_t*)0x200008c0 = 0;
  *(uint32_t*)0x200008c4 = 0;
  *(uint32_t*)0x200008c8 = 0;
  *(uint32_t*)0x200008cc = 0;
  *(uint32_t*)0x200008d0 = 0;
  *(uint32_t*)0x200008d4 = 0;
  *(uint32_t*)0x200008d8 = 0;
  *(uint32_t*)0x200008dc = 0;
  *(uint32_t*)0x200008e0 = 0;
  *(uint32_t*)0x200008e4 = 0;
  *(uint32_t*)0x200008e8 = 0;
  *(uint32_t*)0x200008ec = 0;
  *(uint32_t*)0x200008f0 = 0;
  *(uint32_t*)0x200008f4 = 0;
  *(uint32_t*)0x200008f8 = 0;
  *(uint32_t*)0x200008fc = 0;
  *(uint32_t*)0x20000900 = 0;
  *(uint32_t*)0x20000904 = 0;
  *(uint32_t*)0x20000908 = 0;
  *(uint32_t*)0x2000090c = 0;
  *(uint32_t*)0x20000910 = 0;
  *(uint32_t*)0x20000914 = 0;
  *(uint32_t*)0x20000918 = 0;
  *(uint32_t*)0x2000091c = 0;
  *(uint32_t*)0x20000920 = 0;
  *(uint32_t*)0x20000924 = 0;
  *(uint32_t*)0x20000928 = 0;
  *(uint32_t*)0x2000092c = 0;
  *(uint32_t*)0x20000930 = 0;
  *(uint32_t*)0x20000934 = 0;
  *(uint32_t*)0x20000938 = 0;
  *(uint32_t*)0x2000093c = 0;
  *(uint32_t*)0x20000940 = 0;
  *(uint32_t*)0x20000944 = 0;
  *(uint32_t*)0x20000948 = 0;
  *(uint32_t*)0x2000094c = 0;
  *(uint32_t*)0x20000950 = 0;
  *(uint32_t*)0x20000954 = 0;
  *(uint32_t*)0x20000958 = 0;
  *(uint32_t*)0x2000095c = 0;
  *(uint32_t*)0x20000960 = 0;
  *(uint32_t*)0x20000964 = 0;
  *(uint32_t*)0x20000968 = 0;
  *(uint32_t*)0x2000096c = 0;
  *(uint32_t*)0x20000970 = 0;
  *(uint32_t*)0x20000974 = 0;
  *(uint32_t*)0x20000978 = 0;
  *(uint32_t*)0x2000097c = 0;
  *(uint32_t*)0x20000980 = 0;
  *(uint32_t*)0x20000984 = 0;
  *(uint32_t*)0x20000988 = 0;
  *(uint32_t*)0x2000098c = 0;
  *(uint32_t*)0x20000990 = 0;
  *(uint32_t*)0x20000994 = 0;
  *(uint32_t*)0x20000998 = 0;
  *(uint32_t*)0x2000099c = 0;
  *(uint32_t*)0x200009a0 = 0;
  *(uint32_t*)0x200009a4 = 0;
  *(uint32_t*)0x200009a8 = 0;
  *(uint32_t*)0x200009ac = 0;
  *(uint32_t*)0x200009b0 = 0;
  *(uint32_t*)0x200009b4 = 0;
  *(uint32_t*)0x200009b8 = 0;
  *(uint32_t*)0x200009bc = 0;
  *(uint32_t*)0x200009c0 = 0;
  *(uint32_t*)0x200009c4 = 0;
  *(uint32_t*)0x200009c8 = 0;
  *(uint32_t*)0x200009cc = 0;
  *(uint32_t*)0x200009d0 = 0;
  *(uint32_t*)0x200009d4 = 0;
  *(uint32_t*)0x200009d8 = 0;
  *(uint32_t*)0x200009dc = 0;
  *(uint32_t*)0x200009e0 = 0;
  *(uint32_t*)0x200009e4 = 0;
  *(uint32_t*)0x200009e8 = 0;
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = 0;
  *(uint32_t*)0x200009f4 = 0;
  *(uint32_t*)0x200009f8 = 0;
  *(uint32_t*)0x200009fc = 0;
  *(uint32_t*)0x20000a00 = 0;
  *(uint32_t*)0x20000a04 = 0;
  *(uint32_t*)0x20000a08 = 0;
  *(uint32_t*)0x20000a0c = 0;
  *(uint32_t*)0x20000a10 = 0;
  *(uint32_t*)0x20000a14 = 0;
  *(uint32_t*)0x20000a18 = 0;
  syscall(__NR_write, r[0], 0x200005c0ul, 0x45cul);
  syscall(__NR_ioctl, r[0], 0x5501, 0);
  return 0;
}