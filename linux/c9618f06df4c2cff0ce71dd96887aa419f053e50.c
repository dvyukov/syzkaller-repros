// KASAN: stack-out-of-bounds Read in string
// https://syzkaller.appspot.com/bug?id=c9618f06df4c2cff0ce71dd96887aa419f053e50
// status:fixed
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/usb/ch9.h>

unsigned long long procid;

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
}

#define USB_MAX_EP_NUM 32

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  unsigned config_length;
  struct usb_interface_descriptor* iface;
  struct usb_endpoint_descriptor* eps[USB_MAX_EP_NUM];
  unsigned eps_num;
};

static bool parse_usb_descriptor(char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length <
      sizeof(*index->dev) + sizeof(*index->config) + sizeof(*index->iface))
    return false;
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->config_length = length - sizeof(*index->dev);
  index->iface =
      (struct usb_interface_descriptor*)(buffer + sizeof(*index->dev) +
                                         sizeof(*index->config));
  index->eps_num = 0;
  size_t offset = 0;
  while (true) {
    if (offset == length)
      break;
    if (offset + 1 < length)
      break;
    uint8_t length = buffer[offset];
    uint8_t type = buffer[offset + 1];
    if (type == USB_DT_ENDPOINT) {
      index->eps[index->eps_num] =
          (struct usb_endpoint_descriptor*)(buffer + offset);
      index->eps_num++;
    }
    if (index->eps_num == USB_MAX_EP_NUM)
      break;
    offset += length;
  }
  return true;
}

enum usb_fuzzer_event_type {
  USB_FUZZER_EVENT_INVALID,
  USB_FUZZER_EVENT_CONNECT,
  USB_FUZZER_EVENT_DISCONNECT,
  USB_FUZZER_EVENT_SUSPEND,
  USB_FUZZER_EVENT_RESUME,
  USB_FUZZER_EVENT_CONTROL,
};

struct usb_fuzzer_event {
  uint32_t type;
  uint32_t length;
  char data[0];
};

struct usb_fuzzer_init {
  uint64_t speed;
  const char* driver_name;
  const char* device_name;
};

struct usb_fuzzer_ep_io {
  uint16_t ep;
  uint16_t flags;
  uint32_t length;
  char data[0];
};

#define USB_FUZZER_IOCTL_INIT _IOW('U', 0, struct usb_fuzzer_init)
#define USB_FUZZER_IOCTL_RUN _IO('U', 1)
#define USB_FUZZER_IOCTL_EP0_READ _IOWR('U', 2, struct usb_fuzzer_event)
#define USB_FUZZER_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_ENABLE _IOW('U', 4, struct usb_endpoint_descriptor)
#define USB_FUZZER_IOCTL_EP_WRITE _IOW('U', 6, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_CONFIGURE _IO('U', 8)
#define USB_FUZZER_IOCTL_VBUS_DRAW _IOW('U', 9, uint32_t)

int usb_fuzzer_open()
{
  return open("/sys/kernel/debug/usb-fuzzer", O_RDWR);
}

int usb_fuzzer_init(int fd, uint32_t speed, const char* driver,
                    const char* device)
{
  struct usb_fuzzer_init arg;
  arg.speed = speed;
  arg.driver_name = driver;
  arg.device_name = device;
  return ioctl(fd, USB_FUZZER_IOCTL_INIT, &arg);
}

int usb_fuzzer_run(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_RUN, 0);
}

int usb_fuzzer_ep0_read(int fd, struct usb_fuzzer_event* event)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_READ, event);
}

int usb_fuzzer_ep0_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_WRITE, io);
}

int usb_fuzzer_ep_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_WRITE, io);
}

int usb_fuzzer_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_ENABLE, desc);
}

int usb_fuzzer_configure(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_CONFIGURE, 0);
}

int usb_fuzzer_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_FUZZER_IOCTL_VBUS_DRAW, power);
}

#define USB_MAX_PACKET_SIZE 1024

struct usb_fuzzer_control_event {
  struct usb_fuzzer_event inner;
  struct usb_ctrlrequest ctrl;
};

struct usb_fuzzer_ep_io_data {
  struct usb_fuzzer_ep_io inner;
  char data[USB_MAX_PACKET_SIZE];
};

struct vusb_connect_string_descriptor {
  uint32_t len;
  char* str;
} __attribute__((packed));

struct vusb_connect_descriptors {
  uint32_t qual_len;
  char* qual;
  uint32_t bos_len;
  char* bos;
  uint32_t strs_len;
  struct vusb_connect_string_descriptor strs[0];
} __attribute__((packed));

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  int64_t speed = a0;
  int64_t dev_len = a1;
  char* dev = (char*)a2;
  struct vusb_connect_descriptors* conn_descs =
      (struct vusb_connect_descriptors*)a3;
  if (!dev)
    return -1;
  struct usb_device_index index;
  memset(&index, 0, sizeof(index));
  int rv = parse_usb_descriptor(dev, dev_len, &index);
  if (!rv)
    return -1;
  int fd = usb_fuzzer_open();
  if (fd < 0)
    return -1;
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  rv = usb_fuzzer_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0)
    return -1;
  rv = usb_fuzzer_run(fd);
  if (rv < 0)
    return -1;
  bool done = false;
  while (!done) {
    char* response_data = NULL;
    uint32_t response_length = 0;
    unsigned ep;
    uint8_t str_idx;
    struct usb_fuzzer_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_event*)&event);
    if (rv < 0)
      return -1;
    if (event.inner.type != USB_FUZZER_EVENT_CONTROL)
      continue;
    switch (event.ctrl.bRequestType & USB_TYPE_MASK) {
    case USB_TYPE_STANDARD:
      switch (event.ctrl.bRequest) {
      case USB_REQ_GET_DESCRIPTOR:
        switch (event.ctrl.wValue >> 8) {
        case USB_DT_DEVICE:
          response_data = (char*)index.dev;
          response_length = sizeof(*index.dev);
          goto reply;
        case USB_DT_CONFIG:
          response_data = (char*)index.config;
          response_length = index.config_length;
          goto reply;
        case USB_DT_STRING:
          str_idx = (uint8_t)event.ctrl.wValue;
          if (str_idx >= conn_descs->strs_len)
            goto reply;
          response_data = conn_descs->strs[str_idx].str;
          response_length = conn_descs->strs[str_idx].len;
          goto reply;
        case USB_DT_BOS:
          response_data = conn_descs->bos;
          response_length = conn_descs->bos_len;
          goto reply;
        case USB_DT_DEVICE_QUALIFIER:
          response_data = conn_descs->qual;
          response_length = conn_descs->qual_len;
          goto reply;
        default:
          exit(1);
          continue;
        }
        break;
      case USB_REQ_SET_CONFIGURATION:
        rv = usb_fuzzer_vbus_draw(fd, index.config->bMaxPower);
        if (rv < 0)
          return -1;
        rv = usb_fuzzer_configure(fd);
        if (rv < 0)
          return -1;
        for (ep = 0; ep < index.eps_num; ep++) {
          rv = usb_fuzzer_ep_enable(fd, index.eps[ep]);
          if (rv < 0)
            exit(1);
        }
        done = true;
        goto reply;
      default:
        exit(1);
        continue;
      }
      break;
    default:
      exit(1);
      continue;
    }
    struct usb_fuzzer_ep_io_data response;
  reply:
    response.inner.ep = 0;
    response.inner.flags = 0;
    if (response_length > sizeof(response.data))
      response_length = 0;
    response.inner.length = response_length;
    if (response_data)
      memcpy(&response.data[0], response_data, response_length);
    if (event.ctrl.wLength < response.inner.length)
      response.inner.length = event.ctrl.wLength;
    usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
  }
  sleep_ms(200);
  return fd;
}

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  use_temporary_dir();
  NONFAILING(*(uint8_t*)0x20000040 = 0x12);
  NONFAILING(*(uint8_t*)0x20000041 = 1);
  NONFAILING(*(uint16_t*)0x20000042 = 0);
  NONFAILING(*(uint8_t*)0x20000044 = 0xea);
  NONFAILING(*(uint8_t*)0x20000045 = 0x89);
  NONFAILING(*(uint8_t*)0x20000046 = 0x7a);
  NONFAILING(*(uint8_t*)0x20000047 = 0x40);
  NONFAILING(*(uint16_t*)0x20000048 = 0x424);
  NONFAILING(*(uint16_t*)0x2000004a = 0x12c);
  NONFAILING(*(uint16_t*)0x2000004c = 0x1a78);
  NONFAILING(*(uint8_t*)0x2000004e = 0);
  NONFAILING(*(uint8_t*)0x2000004f = 0);
  NONFAILING(*(uint8_t*)0x20000050 = 0);
  NONFAILING(*(uint8_t*)0x20000051 = 1);
  NONFAILING(*(uint8_t*)0x20000052 = 9);
  NONFAILING(*(uint8_t*)0x20000053 = 2);
  NONFAILING(*(uint16_t*)0x20000054 = 0x1b);
  NONFAILING(*(uint8_t*)0x20000056 = 1);
  NONFAILING(*(uint8_t*)0x20000057 = 0);
  NONFAILING(*(uint8_t*)0x20000058 = 0);
  NONFAILING(*(uint8_t*)0x20000059 = 0);
  NONFAILING(*(uint8_t*)0x2000005a = 0);
  NONFAILING(*(uint8_t*)0x2000005b = 9);
  NONFAILING(*(uint8_t*)0x2000005c = 4);
  NONFAILING(*(uint8_t*)0x2000005d = 0x9e);
  NONFAILING(*(uint8_t*)0x2000005e = 0);
  NONFAILING(*(uint8_t*)0x2000005f = 1);
  NONFAILING(*(uint8_t*)0x20000060 = 0xb4);
  NONFAILING(*(uint8_t*)0x20000061 = 0x11);
  NONFAILING(*(uint8_t*)0x20000062 = 0xd1);
  NONFAILING(*(uint8_t*)0x20000063 = 0);
  NONFAILING(*(uint8_t*)0x20000064 = 7);
  NONFAILING(*(uint8_t*)0x20000065 = 5);
  NONFAILING(*(uint8_t*)0x20000066 = 0x8c);
  NONFAILING(*(uint8_t*)0x20000067 = 0);
  NONFAILING(*(uint16_t*)0x20000068 = 0);
  NONFAILING(*(uint8_t*)0x2000006a = 0);
  NONFAILING(*(uint8_t*)0x2000006b = 0);
  NONFAILING(*(uint8_t*)0x2000006c = 0);
  syz_usb_connect(4, 0x2d, 0x20000040, 0);
  return 0;
}