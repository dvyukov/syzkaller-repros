// INFO: task hung in usb_register_dev
// https://syzkaller.appspot.com/bug?id=e761775e8f4a28711f19
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/usb/ch9.h>

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

#define MAX_FDS 30

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32
#define USB_MAX_FDS 6

struct usb_endpoint_index {
  struct usb_endpoint_descriptor desc;
  int handle;
};

struct usb_iface_index {
  struct usb_interface_descriptor* iface;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bInterfaceClass;
  struct usb_endpoint_index eps[USB_MAX_EP_NUM];
  int eps_num;
};

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  uint8_t bDeviceClass;
  uint8_t bMaxPower;
  int config_length;
  struct usb_iface_index ifaces[USB_MAX_IFACE_NUM];
  int ifaces_num;
  int iface_cur;
};

struct usb_info {
  int fd;
  struct usb_device_index index;
};

static struct usb_info usb_devices[USB_MAX_FDS];
static int usb_devices_num;

static bool parse_usb_descriptor(const char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length < sizeof(*index->dev) + sizeof(*index->config))
    return false;
  memset(index, 0, sizeof(*index));
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->bDeviceClass = index->dev->bDeviceClass;
  index->bMaxPower = index->config->bMaxPower;
  index->config_length = length - sizeof(*index->dev);
  index->iface_cur = -1;
  size_t offset = 0;
  while (true) {
    if (offset + 1 >= length)
      break;
    uint8_t desc_length = buffer[offset];
    uint8_t desc_type = buffer[offset + 1];
    if (desc_length <= 2)
      break;
    if (offset + desc_length > length)
      break;
    if (desc_type == USB_DT_INTERFACE &&
        index->ifaces_num < USB_MAX_IFACE_NUM) {
      struct usb_interface_descriptor* iface =
          (struct usb_interface_descriptor*)(buffer + offset);
      index->ifaces[index->ifaces_num].iface = iface;
      index->ifaces[index->ifaces_num].bInterfaceNumber =
          iface->bInterfaceNumber;
      index->ifaces[index->ifaces_num].bAlternateSetting =
          iface->bAlternateSetting;
      index->ifaces[index->ifaces_num].bInterfaceClass = iface->bInterfaceClass;
      index->ifaces_num++;
    }
    if (desc_type == USB_DT_ENDPOINT && index->ifaces_num > 0) {
      struct usb_iface_index* iface = &index->ifaces[index->ifaces_num - 1];
      if (iface->eps_num < USB_MAX_EP_NUM) {
        memcpy(&iface->eps[iface->eps_num].desc, buffer + offset,
               sizeof(iface->eps[iface->eps_num].desc));
        iface->eps_num++;
      }
    }
    offset += desc_length;
  }
  return true;
}

static struct usb_device_index* add_usb_index(int fd, const char* dev,
                                              size_t dev_len)
{
  int i = __atomic_fetch_add(&usb_devices_num, 1, __ATOMIC_RELAXED);
  if (i >= USB_MAX_FDS)
    return NULL;
  if (!parse_usb_descriptor(dev, dev_len, &usb_devices[i].index))
    return NULL;
  __atomic_store_n(&usb_devices[i].fd, fd, __ATOMIC_RELEASE);
  return &usb_devices[i].index;
}

static struct usb_device_index* lookup_usb_index(int fd)
{
  for (int i = 0; i < USB_MAX_FDS; i++) {
    if (__atomic_load_n(&usb_devices[i].fd, __ATOMIC_ACQUIRE) == fd) {
      return &usb_devices[i].index;
    }
  }
  return NULL;
}

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

static const char default_string[] = {8, USB_DT_STRING, 's', 0, 'y', 0, 'z', 0};

static const char default_lang_id[] = {4, USB_DT_STRING, 0x09, 0x04};

static bool
lookup_connect_response_in(int fd, const struct vusb_connect_descriptors* descs,
                           const struct usb_ctrlrequest* ctrl,
                           char** response_data, uint32_t* response_length)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  uint8_t str_idx;
  if (!index)
    return false;
  switch (ctrl->bRequestType & USB_TYPE_MASK) {
  case USB_TYPE_STANDARD:
    switch (ctrl->bRequest) {
    case USB_REQ_GET_DESCRIPTOR:
      switch (ctrl->wValue >> 8) {
      case USB_DT_DEVICE:
        *response_data = (char*)index->dev;
        *response_length = sizeof(*index->dev);
        return true;
      case USB_DT_CONFIG:
        *response_data = (char*)index->config;
        *response_length = index->config_length;
        return true;
      case USB_DT_STRING:
        str_idx = (uint8_t)ctrl->wValue;
        if (descs && str_idx < descs->strs_len) {
          *response_data = descs->strs[str_idx].str;
          *response_length = descs->strs[str_idx].len;
          return true;
        }
        if (str_idx == 0) {
          *response_data = (char*)&default_lang_id[0];
          *response_length = default_lang_id[0];
          return true;
        }
        *response_data = (char*)&default_string[0];
        *response_length = default_string[0];
        return true;
      case USB_DT_BOS:
        *response_data = descs->bos;
        *response_length = descs->bos_len;
        return true;
      case USB_DT_DEVICE_QUALIFIER:
        if (!descs->qual) {
          struct usb_qualifier_descriptor* qual =
              (struct usb_qualifier_descriptor*)response_data;
          qual->bLength = sizeof(*qual);
          qual->bDescriptorType = USB_DT_DEVICE_QUALIFIER;
          qual->bcdUSB = index->dev->bcdUSB;
          qual->bDeviceClass = index->dev->bDeviceClass;
          qual->bDeviceSubClass = index->dev->bDeviceSubClass;
          qual->bDeviceProtocol = index->dev->bDeviceProtocol;
          qual->bMaxPacketSize0 = index->dev->bMaxPacketSize0;
          qual->bNumConfigurations = index->dev->bNumConfigurations;
          qual->bRESERVED = 0;
          *response_length = sizeof(*qual);
          return true;
        }
        *response_data = descs->qual;
        *response_length = descs->qual_len;
        return true;
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  return false;
}

typedef bool (*lookup_connect_out_response_t)(
    int fd, const struct vusb_connect_descriptors* descs,
    const struct usb_ctrlrequest* ctrl, bool* done);

static bool lookup_connect_response_out_generic(
    int fd, const struct vusb_connect_descriptors* descs,
    const struct usb_ctrlrequest* ctrl, bool* done)
{
  switch (ctrl->bRequestType & USB_TYPE_MASK) {
  case USB_TYPE_STANDARD:
    switch (ctrl->bRequest) {
    case USB_REQ_SET_CONFIGURATION:
      *done = true;
      return true;
    default:
      break;
    }
    break;
  }
  return false;
}

#define UDC_NAME_LENGTH_MAX 128

struct usb_raw_init {
  __u8 driver_name[UDC_NAME_LENGTH_MAX];
  __u8 device_name[UDC_NAME_LENGTH_MAX];
  __u8 speed;
};

enum usb_raw_event_type {
  USB_RAW_EVENT_INVALID = 0,
  USB_RAW_EVENT_CONNECT = 1,
  USB_RAW_EVENT_CONTROL = 2,
};

struct usb_raw_event {
  __u32 type;
  __u32 length;
  __u8 data[0];
};

struct usb_raw_ep_io {
  __u16 ep;
  __u16 flags;
  __u32 length;
  __u8 data[0];
};

#define USB_RAW_EPS_NUM_MAX 30
#define USB_RAW_EP_NAME_MAX 16
#define USB_RAW_EP_ADDR_ANY 0xff

struct usb_raw_ep_caps {
  __u32 type_control : 1;
  __u32 type_iso : 1;
  __u32 type_bulk : 1;
  __u32 type_int : 1;
  __u32 dir_in : 1;
  __u32 dir_out : 1;
};

struct usb_raw_ep_limits {
  __u16 maxpacket_limit;
  __u16 max_streams;
  __u32 reserved;
};

struct usb_raw_ep_info {
  __u8 name[USB_RAW_EP_NAME_MAX];
  __u32 addr;
  struct usb_raw_ep_caps caps;
  struct usb_raw_ep_limits limits;
};

struct usb_raw_eps_info {
  struct usb_raw_ep_info eps[USB_RAW_EPS_NUM_MAX];
};

#define USB_RAW_IOCTL_INIT _IOW('U', 0, struct usb_raw_init)
#define USB_RAW_IOCTL_RUN _IO('U', 1)
#define USB_RAW_IOCTL_EVENT_FETCH _IOR('U', 2, struct usb_raw_event)
#define USB_RAW_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP0_READ _IOWR('U', 4, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_ENABLE _IOW('U', 5, struct usb_endpoint_descriptor)
#define USB_RAW_IOCTL_EP_DISABLE _IOW('U', 6, __u32)
#define USB_RAW_IOCTL_EP_WRITE _IOW('U', 7, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_READ _IOWR('U', 8, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_CONFIGURE _IO('U', 9)
#define USB_RAW_IOCTL_VBUS_DRAW _IOW('U', 10, __u32)
#define USB_RAW_IOCTL_EPS_INFO _IOR('U', 11, struct usb_raw_eps_info)
#define USB_RAW_IOCTL_EP0_STALL _IO('U', 12)
#define USB_RAW_IOCTL_EP_SET_HALT _IOW('U', 13, __u32)
#define USB_RAW_IOCTL_EP_CLEAR_HALT _IOW('U', 14, __u32)
#define USB_RAW_IOCTL_EP_SET_WEDGE _IOW('U', 15, __u32)

static int usb_raw_open()
{
  return open("/dev/raw-gadget", O_RDWR);
}

static int usb_raw_init(int fd, uint32_t speed, const char* driver,
                        const char* device)
{
  struct usb_raw_init arg;
  strncpy((char*)&arg.driver_name[0], driver, sizeof(arg.driver_name));
  strncpy((char*)&arg.device_name[0], device, sizeof(arg.device_name));
  arg.speed = speed;
  return ioctl(fd, USB_RAW_IOCTL_INIT, &arg);
}

static int usb_raw_run(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_RUN, 0);
}

static int usb_raw_event_fetch(int fd, struct usb_raw_event* event)
{
  return ioctl(fd, USB_RAW_IOCTL_EVENT_FETCH, event);
}

static int usb_raw_ep0_write(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_WRITE, io);
}

static int usb_raw_ep0_read(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_READ, io);
}

static int usb_raw_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_ENABLE, desc);
}

static int usb_raw_ep_disable(int fd, int ep)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_DISABLE, ep);
}

static int usb_raw_configure(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_CONFIGURE, 0);
}

static int usb_raw_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_RAW_IOCTL_VBUS_DRAW, power);
}

static int usb_raw_ep0_stall(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_STALL, 0);
}

static void set_interface(int fd, int n)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return;
  if (index->iface_cur >= 0 && index->iface_cur < index->ifaces_num) {
    for (int ep = 0; ep < index->ifaces[index->iface_cur].eps_num; ep++) {
      int rv = usb_raw_ep_disable(
          fd, index->ifaces[index->iface_cur].eps[ep].handle);
      if (rv < 0) {
      } else {
      }
    }
  }
  if (n >= 0 && n < index->ifaces_num) {
    for (int ep = 0; ep < index->ifaces[n].eps_num; ep++) {
      int rv = usb_raw_ep_enable(fd, &index->ifaces[n].eps[ep].desc);
      if (rv < 0) {
      } else {
        index->ifaces[n].eps[ep].handle = rv;
      }
    }
    index->iface_cur = n;
  }
}

static int configure_device(int fd)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return -1;
  int rv = usb_raw_vbus_draw(fd, index->bMaxPower);
  if (rv < 0) {
    return rv;
  }
  rv = usb_raw_configure(fd);
  if (rv < 0) {
    return rv;
  }
  set_interface(fd, 0);
  return 0;
}

#define USB_MAX_PACKET_SIZE 4096

struct usb_raw_control_event {
  struct usb_raw_event inner;
  struct usb_ctrlrequest ctrl;
  char data[USB_MAX_PACKET_SIZE];
};

struct usb_raw_ep_io_data {
  struct usb_raw_ep_io inner;
  char data[USB_MAX_PACKET_SIZE];
};

static volatile long
syz_usb_connect_impl(uint64_t speed, uint64_t dev_len, const char* dev,
                     const struct vusb_connect_descriptors* descs,
                     lookup_connect_out_response_t lookup_connect_response_out)
{
  if (!dev) {
    return -1;
  }
  int fd = usb_raw_open();
  if (fd < 0) {
    return fd;
  }
  if (fd >= MAX_FDS) {
    close(fd);
    return -1;
  }
  struct usb_device_index* index = add_usb_index(fd, dev, dev_len);
  if (!index) {
    return -1;
  }
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  int rv = usb_raw_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0) {
    return rv;
  }
  rv = usb_raw_run(fd);
  if (rv < 0) {
    return rv;
  }
  bool done = false;
  while (!done) {
    struct usb_raw_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_raw_event_fetch(fd, (struct usb_raw_event*)&event);
    if (rv < 0) {
      return rv;
    }
    if (event.inner.type != USB_RAW_EVENT_CONTROL)
      continue;
    char* response_data = NULL;
    uint32_t response_length = 0;
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      if (!lookup_connect_response_in(fd, descs, &event.ctrl, &response_data,
                                      &response_length)) {
        usb_raw_ep0_stall(fd);
        continue;
      }
    } else {
      if (!lookup_connect_response_out(fd, descs, &event.ctrl, &done)) {
        usb_raw_ep0_stall(fd);
        continue;
      }
      response_data = NULL;
      response_length = event.ctrl.wLength;
    }
    if ((event.ctrl.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD &&
        event.ctrl.bRequest == USB_REQ_SET_CONFIGURATION) {
      rv = configure_device(fd);
      if (rv < 0) {
        return rv;
      }
    }
    struct usb_raw_ep_io_data response;
    response.inner.ep = 0;
    response.inner.flags = 0;
    if (response_length > sizeof(response.data))
      response_length = 0;
    if (event.ctrl.wLength < response_length)
      response_length = event.ctrl.wLength;
    response.inner.length = response_length;
    if (response_data)
      memcpy(&response.data[0], response_data, response_length);
    else
      memset(&response.data[0], 0, response_length);
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      rv = usb_raw_ep0_write(fd, (struct usb_raw_ep_io*)&response);
    } else {
      rv = usb_raw_ep0_read(fd, (struct usb_raw_ep_io*)&response);
    }
    if (rv < 0) {
      return rv;
    }
  }
  sleep_ms(200);
  return fd;
}

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  uint64_t speed = a0;
  uint64_t dev_len = a1;
  const char* dev = (const char*)a2;
  const struct vusb_connect_descriptors* descs =
      (const struct vusb_connect_descriptors*)a3;
  return syz_usb_connect_impl(speed, dev_len, dev, descs,
                              &lookup_connect_response_out_generic);
}

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
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

void execute_one(void)
{
  syz_open_dev(0xc, 0xb4, 3);
  *(uint8_t*)0x200015c0 = 0x12;
  *(uint8_t*)0x200015c1 = 1;
  *(uint16_t*)0x200015c2 = 0x110;
  *(uint8_t*)0x200015c4 = 0xf;
  *(uint8_t*)0x200015c5 = 0xe1;
  *(uint8_t*)0x200015c6 = 0xa;
  *(uint8_t*)0x200015c7 = 8;
  *(uint16_t*)0x200015c8 = 0x711;
  *(uint16_t*)0x200015ca = 0x903;
  *(uint16_t*)0x200015cc = 0x5161;
  *(uint8_t*)0x200015ce = 1;
  *(uint8_t*)0x200015cf = 2;
  *(uint8_t*)0x200015d0 = 3;
  *(uint8_t*)0x200015d1 = 1;
  *(uint8_t*)0x200015d2 = 9;
  *(uint8_t*)0x200015d3 = 2;
  *(uint16_t*)0x200015d4 = 0x27a;
  *(uint8_t*)0x200015d6 = 3;
  *(uint8_t*)0x200015d7 = 4;
  *(uint8_t*)0x200015d8 = 0;
  *(uint8_t*)0x200015d9 = 0x10;
  *(uint8_t*)0x200015da = 0;
  *(uint8_t*)0x200015db = 9;
  *(uint8_t*)0x200015dc = 4;
  *(uint8_t*)0x200015dd = 0xfd;
  *(uint8_t*)0x200015de = 0x81;
  *(uint8_t*)0x200015df = 0xd;
  *(uint8_t*)0x200015e0 = 0x4d;
  *(uint8_t*)0x200015e1 = 0x88;
  *(uint8_t*)0x200015e2 = 0x55;
  *(uint8_t*)0x200015e3 = 0;
  *(uint8_t*)0x200015e4 = 5;
  *(uint8_t*)0x200015e5 = 0x24;
  *(uint8_t*)0x200015e6 = 6;
  *(uint8_t*)0x200015e7 = 0;
  *(uint8_t*)0x200015e8 = 1;
  *(uint8_t*)0x200015e9 = 5;
  *(uint8_t*)0x200015ea = 0x24;
  *(uint8_t*)0x200015eb = 0;
  *(uint16_t*)0x200015ec = 9;
  *(uint8_t*)0x200015ee = 0xd;
  *(uint8_t*)0x200015ef = 0x24;
  *(uint8_t*)0x200015f0 = 0xf;
  *(uint8_t*)0x200015f1 = 1;
  *(uint32_t*)0x200015f2 = 1;
  *(uint16_t*)0x200015f6 = 4;
  *(uint16_t*)0x200015f8 = 6;
  *(uint8_t*)0x200015fa = 0x20;
  *(uint8_t*)0x200015fb = 6;
  *(uint8_t*)0x200015fc = 0x24;
  *(uint8_t*)0x200015fd = 0x1a;
  *(uint16_t*)0x200015fe = 0x777;
  *(uint8_t*)0x20001600 = 0x12;
  *(uint8_t*)0x20001601 = 5;
  *(uint8_t*)0x20001602 = 0x24;
  *(uint8_t*)0x20001603 = 1;
  *(uint8_t*)0x20001604 = 3;
  *(uint8_t*)0x20001605 = 0xc7;
  *(uint8_t*)0x20001606 = 4;
  *(uint8_t*)0x20001607 = 0x24;
  *(uint8_t*)0x20001608 = 2;
  *(uint8_t*)0x20001609 = 5;
  *(uint8_t*)0x2000160a = 8;
  *(uint8_t*)0x2000160b = 0x24;
  *(uint8_t*)0x2000160c = 0x1c;
  *(uint16_t*)0x2000160d = 0;
  *(uint8_t*)0x2000160f = 7;
  *(uint16_t*)0x20001610 = 1;
  *(uint8_t*)0x20001612 = 2;
  *(uint8_t*)0x20001613 = 0x24;
  *(uint8_t*)0x20001614 = 9;
  *(uint8_t*)0x20001615 = 5;
  *(uint8_t*)0x20001616 = 0xb;
  *(uint8_t*)0x20001617 = 0x10;
  *(uint16_t*)0x20001618 = 0x3ff;
  *(uint8_t*)0x2000161a = 0x20;
  *(uint8_t*)0x2000161b = 0x97;
  *(uint8_t*)0x2000161c = 0;
  *(uint8_t*)0x2000161d = 2;
  *(uint8_t*)0x2000161e = 8;
  *(uint8_t*)0x2000161f = 2;
  *(uint8_t*)0x20001620 = 0x22;
  *(uint8_t*)0x20001621 = 9;
  *(uint8_t*)0x20001622 = 5;
  *(uint8_t*)0x20001623 = 0xc;
  *(uint8_t*)0x20001624 = 0;
  *(uint16_t*)0x20001625 = 0x10;
  *(uint8_t*)0x20001627 = 1;
  *(uint8_t*)0x20001628 = 9;
  *(uint8_t*)0x20001629 = 0x81;
  *(uint8_t*)0x2000162a = 7;
  *(uint8_t*)0x2000162b = 0x25;
  *(uint8_t*)0x2000162c = 1;
  *(uint8_t*)0x2000162d = 2;
  *(uint8_t*)0x2000162e = 0xe0;
  *(uint16_t*)0x2000162f = 9;
  *(uint8_t*)0x20001631 = 9;
  *(uint8_t*)0x20001632 = 5;
  *(uint8_t*)0x20001633 = 1;
  *(uint8_t*)0x20001634 = 0xc;
  *(uint16_t*)0x20001635 = 0x400;
  *(uint8_t*)0x20001637 = 0x28;
  *(uint8_t*)0x20001638 = 0xfb;
  *(uint8_t*)0x20001639 = 4;
  *(uint8_t*)0x2000163a = 2;
  *(uint8_t*)0x2000163b = 1;
  *(uint8_t*)0x2000163c = 2;
  *(uint8_t*)0x2000163d = 0xa;
  *(uint8_t*)0x2000163e = 9;
  *(uint8_t*)0x2000163f = 5;
  *(uint8_t*)0x20001640 = 4;
  *(uint8_t*)0x20001641 = 0x10;
  *(uint16_t*)0x20001642 = 0x3ff;
  *(uint8_t*)0x20001644 = 6;
  *(uint8_t*)0x20001645 = 4;
  *(uint8_t*)0x20001646 = 1;
  *(uint8_t*)0x20001647 = 9;
  *(uint8_t*)0x20001648 = 5;
  *(uint8_t*)0x20001649 = 4;
  *(uint8_t*)0x2000164a = 0x10;
  *(uint16_t*)0x2000164b = 0x1ff;
  *(uint8_t*)0x2000164d = -1;
  *(uint8_t*)0x2000164e = 0xec;
  *(uint8_t*)0x2000164f = 0x3f;
  *(uint8_t*)0x20001650 = 9;
  *(uint8_t*)0x20001651 = 5;
  *(uint8_t*)0x20001652 = 1;
  *(uint8_t*)0x20001653 = 0xc;
  *(uint16_t*)0x20001654 = 0x20;
  *(uint8_t*)0x20001656 = 2;
  *(uint8_t*)0x20001657 = 9;
  *(uint8_t*)0x20001658 = 4;
  *(uint8_t*)0x20001659 = 7;
  *(uint8_t*)0x2000165a = 0x25;
  *(uint8_t*)0x2000165b = 1;
  *(uint8_t*)0x2000165c = 0x81;
  *(uint8_t*)0x2000165d = 7;
  *(uint16_t*)0x2000165e = 6;
  *(uint8_t*)0x20001660 = 7;
  *(uint8_t*)0x20001661 = 0x25;
  *(uint8_t*)0x20001662 = 1;
  *(uint8_t*)0x20001663 = 1;
  *(uint8_t*)0x20001664 = 0;
  *(uint16_t*)0x20001665 = -1;
  *(uint8_t*)0x20001667 = 9;
  *(uint8_t*)0x20001668 = 5;
  *(uint8_t*)0x20001669 = 0;
  *(uint8_t*)0x2000166a = 0;
  *(uint16_t*)0x2000166b = 0x1f7;
  *(uint8_t*)0x2000166d = 0x20;
  *(uint8_t*)0x2000166e = 0x40;
  *(uint8_t*)0x2000166f = 0x81;
  *(uint8_t*)0x20001670 = 2;
  *(uint8_t*)0x20001671 = 0;
  *(uint8_t*)0x20001672 = 7;
  *(uint8_t*)0x20001673 = 0x25;
  *(uint8_t*)0x20001674 = 1;
  *(uint8_t*)0x20001675 = 0x80;
  *(uint8_t*)0x20001676 = 7;
  *(uint16_t*)0x20001677 = 5;
  *(uint8_t*)0x20001679 = 9;
  *(uint8_t*)0x2000167a = 5;
  *(uint8_t*)0x2000167b = 6;
  *(uint8_t*)0x2000167c = 2;
  *(uint16_t*)0x2000167d = 8;
  *(uint8_t*)0x2000167f = 0;
  *(uint8_t*)0x20001680 = 6;
  *(uint8_t*)0x20001681 = 0;
  *(uint8_t*)0x20001682 = 9;
  *(uint8_t*)0x20001683 = 5;
  *(uint8_t*)0x20001684 = 1;
  *(uint8_t*)0x20001685 = 0x10;
  *(uint16_t*)0x20001686 = 0x3ff;
  *(uint8_t*)0x20001688 = 6;
  *(uint8_t*)0x20001689 = 9;
  *(uint8_t*)0x2000168a = 9;
  *(uint8_t*)0x2000168b = 9;
  *(uint8_t*)0x2000168c = 5;
  *(uint8_t*)0x2000168d = 0xa;
  *(uint8_t*)0x2000168e = 0;
  *(uint16_t*)0x2000168f = 8;
  *(uint8_t*)0x20001691 = 2;
  *(uint8_t*)0x20001692 = 0;
  *(uint8_t*)0x20001693 = 0x8d;
  *(uint8_t*)0x20001694 = 2;
  *(uint8_t*)0x20001695 = 0x31;
  *(uint8_t*)0x20001696 = 7;
  *(uint8_t*)0x20001697 = 0x25;
  *(uint8_t*)0x20001698 = 1;
  *(uint8_t*)0x20001699 = 0;
  *(uint8_t*)0x2000169a = 0x80;
  *(uint16_t*)0x2000169b = 0x99;
  *(uint8_t*)0x2000169d = 9;
  *(uint8_t*)0x2000169e = 5;
  *(uint8_t*)0x2000169f = 5;
  *(uint8_t*)0x200016a0 = 0;
  *(uint16_t*)0x200016a1 = 0x200;
  *(uint8_t*)0x200016a3 = 1;
  *(uint8_t*)0x200016a4 = -1;
  *(uint8_t*)0x200016a5 = 6;
  *(uint8_t*)0x200016a6 = 2;
  *(uint8_t*)0x200016a7 = 0xb;
  *(uint8_t*)0x200016a8 = 9;
  *(uint8_t*)0x200016a9 = 5;
  *(uint8_t*)0x200016aa = 1;
  *(uint8_t*)0x200016ab = 0x10;
  *(uint16_t*)0x200016ac = 0x20;
  *(uint8_t*)0x200016ae = 1;
  *(uint8_t*)0x200016af = 0x1f;
  *(uint8_t*)0x200016b0 = 2;
  *(uint8_t*)0x200016b1 = 7;
  *(uint8_t*)0x200016b2 = 0x25;
  *(uint8_t*)0x200016b3 = 1;
  *(uint8_t*)0x200016b4 = 3;
  *(uint8_t*)0x200016b5 = 8;
  *(uint16_t*)0x200016b6 = 0x40;
  *(uint8_t*)0x200016b8 = 9;
  *(uint8_t*)0x200016b9 = 5;
  *(uint8_t*)0x200016ba = 0xc;
  *(uint8_t*)0x200016bb = 0;
  *(uint16_t*)0x200016bc = 0x20;
  *(uint8_t*)0x200016be = 2;
  *(uint8_t*)0x200016bf = 0;
  *(uint8_t*)0x200016c0 = 0xdf;
  *(uint8_t*)0x200016c1 = 9;
  *(uint8_t*)0x200016c2 = 4;
  *(uint8_t*)0x200016c3 = 0x10;
  *(uint8_t*)0x200016c4 = 0x81;
  *(uint8_t*)0x200016c5 = 7;
  *(uint8_t*)0x200016c6 = 0x76;
  *(uint8_t*)0x200016c7 = 0xb3;
  *(uint8_t*)0x200016c8 = 0x5d;
  *(uint8_t*)0x200016c9 = 0x69;
  *(uint8_t*)0x200016ca = 9;
  *(uint8_t*)0x200016cb = 0x21;
  *(uint16_t*)0x200016cc = 0xfffd;
  *(uint8_t*)0x200016ce = 0x40;
  *(uint8_t*)0x200016cf = 1;
  *(uint8_t*)0x200016d0 = 0x22;
  *(uint16_t*)0x200016d1 = 0x15;
  *(uint8_t*)0x200016d3 = 5;
  *(uint8_t*)0x200016d4 = 0x24;
  *(uint8_t*)0x200016d5 = 6;
  *(uint8_t*)0x200016d6 = 0;
  *(uint8_t*)0x200016d7 = 1;
  *(uint8_t*)0x200016d8 = 5;
  *(uint8_t*)0x200016d9 = 0x24;
  *(uint8_t*)0x200016da = 0;
  *(uint16_t*)0x200016db = 0x18;
  *(uint8_t*)0x200016dd = 0xd;
  *(uint8_t*)0x200016de = 0x24;
  *(uint8_t*)0x200016df = 0xf;
  *(uint8_t*)0x200016e0 = 1;
  *(uint32_t*)0x200016e1 = 0x919a;
  *(uint16_t*)0x200016e5 = 2;
  *(uint16_t*)0x200016e7 = 7;
  *(uint8_t*)0x200016e9 = 9;
  *(uint8_t*)0x200016ea = 6;
  *(uint8_t*)0x200016eb = 0x24;
  *(uint8_t*)0x200016ec = 0x1a;
  *(uint16_t*)0x200016ed = 7;
  *(uint8_t*)0x200016ef = 0;
  *(uint8_t*)0x200016f0 = 4;
  *(uint8_t*)0x200016f1 = 0x24;
  *(uint8_t*)0x200016f2 = 0x13;
  *(uint8_t*)0x200016f3 = -1;
  *(uint8_t*)0x200016f4 = 4;
  *(uint8_t*)0x200016f5 = 0x24;
  *(uint8_t*)0x200016f6 = 0x13;
  *(uint8_t*)0x200016f7 = 0;
  *(uint8_t*)0x200016f8 = 0xc;
  *(uint8_t*)0x200016f9 = 0x24;
  *(uint8_t*)0x200016fa = 7;
  *(uint8_t*)0x200016fb = 7;
  *(uint16_t*)0x200016fc = 0xfffc;
  *(uint16_t*)0x200016fe = 6;
  *(uint16_t*)0x20001700 = 3;
  *(uint16_t*)0x20001702 = 6;
  *(uint8_t*)0x20001704 = 9;
  *(uint8_t*)0x20001705 = 5;
  *(uint8_t*)0x20001706 = 0xd;
  *(uint8_t*)0x20001707 = 1;
  *(uint16_t*)0x20001708 = 0x200;
  *(uint8_t*)0x2000170a = 3;
  *(uint8_t*)0x2000170b = 0;
  *(uint8_t*)0x2000170c = 6;
  *(uint8_t*)0x2000170d = 7;
  *(uint8_t*)0x2000170e = 0x25;
  *(uint8_t*)0x2000170f = 1;
  *(uint8_t*)0x20001710 = 0;
  *(uint8_t*)0x20001711 = 0xf;
  *(uint16_t*)0x20001712 = 0x40;
  *(uint8_t*)0x20001714 = 9;
  *(uint8_t*)0x20001715 = 5;
  *(uint8_t*)0x20001716 = 4;
  *(uint8_t*)0x20001717 = 0;
  *(uint16_t*)0x20001718 = 0x400;
  *(uint8_t*)0x2000171a = 0x1f;
  *(uint8_t*)0x2000171b = 9;
  *(uint8_t*)0x2000171c = 6;
  *(uint8_t*)0x2000171d = 9;
  *(uint8_t*)0x2000171e = 5;
  *(uint8_t*)0x2000171f = 1;
  *(uint8_t*)0x20001720 = 8;
  *(uint16_t*)0x20001721 = 0x3ff;
  *(uint8_t*)0x20001723 = 0;
  *(uint8_t*)0x20001724 = 9;
  *(uint8_t*)0x20001725 = 4;
  *(uint8_t*)0x20001726 = 2;
  *(uint8_t*)0x20001727 = 0x10;
  *(uint8_t*)0x20001728 = 7;
  *(uint8_t*)0x20001729 = 0x25;
  *(uint8_t*)0x2000172a = 1;
  *(uint8_t*)0x2000172b = 0;
  *(uint8_t*)0x2000172c = 0x7b;
  *(uint16_t*)0x2000172d = 0x5c;
  *(uint8_t*)0x2000172f = 9;
  *(uint8_t*)0x20001730 = 5;
  *(uint8_t*)0x20001731 = 2;
  *(uint8_t*)0x20001732 = 0x10;
  *(uint16_t*)0x20001733 = 0x448;
  *(uint8_t*)0x20001735 = 0x3f;
  *(uint8_t*)0x20001736 = 0x7f;
  *(uint8_t*)0x20001737 = 7;
  *(uint8_t*)0x20001738 = 2;
  *(uint8_t*)0x20001739 = 7;
  *(uint8_t*)0x2000173a = 9;
  *(uint8_t*)0x2000173b = 5;
  *(uint8_t*)0x2000173c = 0;
  *(uint8_t*)0x2000173d = 0x10;
  *(uint16_t*)0x2000173e = 0x400;
  *(uint8_t*)0x20001740 = 4;
  *(uint8_t*)0x20001741 = 8;
  *(uint8_t*)0x20001742 = 0x1b;
  *(uint8_t*)0x20001743 = 7;
  *(uint8_t*)0x20001744 = 0x25;
  *(uint8_t*)0x20001745 = 1;
  *(uint8_t*)0x20001746 = 0;
  *(uint8_t*)0x20001747 = 0xfa;
  *(uint16_t*)0x20001748 = 5;
  *(uint8_t*)0x2000174a = 7;
  *(uint8_t*)0x2000174b = 0x25;
  *(uint8_t*)0x2000174c = 1;
  *(uint8_t*)0x2000174d = 0x82;
  *(uint8_t*)0x2000174e = 0x3f;
  *(uint16_t*)0x2000174f = 7;
  *(uint8_t*)0x20001751 = 9;
  *(uint8_t*)0x20001752 = 5;
  *(uint8_t*)0x20001753 = 9;
  *(uint8_t*)0x20001754 = 2;
  *(uint16_t*)0x20001755 = 0x20;
  *(uint8_t*)0x20001757 = 0x27;
  *(uint8_t*)0x20001758 = 8;
  *(uint8_t*)0x20001759 = 0x1b;
  *(uint8_t*)0x2000175a = 2;
  *(uint8_t*)0x2000175b = 0xe;
  *(uint8_t*)0x2000175c = 9;
  *(uint8_t*)0x2000175d = 5;
  *(uint8_t*)0x2000175e = 0xc;
  *(uint8_t*)0x2000175f = 0xc;
  *(uint16_t*)0x20001760 = 8;
  *(uint8_t*)0x20001762 = 8;
  *(uint8_t*)0x20001763 = 0x9b;
  *(uint8_t*)0x20001764 = 0;
  *(uint8_t*)0x20001765 = 7;
  *(uint8_t*)0x20001766 = 0x25;
  *(uint8_t*)0x20001767 = 1;
  *(uint8_t*)0x20001768 = 0x81;
  *(uint8_t*)0x20001769 = 9;
  *(uint16_t*)0x2000176a = 0x7d;
  *(uint8_t*)0x2000176c = 9;
  *(uint8_t*)0x2000176d = 4;
  *(uint8_t*)0x2000176e = 0xee;
  *(uint8_t*)0x2000176f = 0xf4;
  *(uint8_t*)0x20001770 = 0xe;
  *(uint8_t*)0x20001771 = 0xd1;
  *(uint8_t*)0x20001772 = 0xc2;
  *(uint8_t*)0x20001773 = 0xb0;
  *(uint8_t*)0x20001774 = 1;
  *(uint8_t*)0x20001775 = 9;
  *(uint8_t*)0x20001776 = 5;
  *(uint8_t*)0x20001777 = 0xd;
  *(uint8_t*)0x20001778 = 3;
  *(uint16_t*)0x20001779 = 0x200;
  *(uint8_t*)0x2000177b = 1;
  *(uint8_t*)0x2000177c = 2;
  *(uint8_t*)0x2000177d = 0x1f;
  *(uint8_t*)0x2000177e = 7;
  *(uint8_t*)0x2000177f = 0x25;
  *(uint8_t*)0x20001780 = 1;
  *(uint8_t*)0x20001781 = 0x81;
  *(uint8_t*)0x20001782 = 0x7f;
  *(uint16_t*)0x20001783 = 5;
  *(uint8_t*)0x20001785 = 9;
  *(uint8_t*)0x20001786 = 5;
  *(uint8_t*)0x20001787 = 0;
  *(uint8_t*)0x20001788 = 3;
  *(uint16_t*)0x20001789 = 0x200;
  *(uint8_t*)0x2000178b = 0x3f;
  *(uint8_t*)0x2000178c = 8;
  *(uint8_t*)0x2000178d = 0xbc;
  *(uint8_t*)0x2000178e = 9;
  *(uint8_t*)0x2000178f = 5;
  *(uint8_t*)0x20001790 = 7;
  *(uint8_t*)0x20001791 = 4;
  *(uint16_t*)0x20001792 = 0x10;
  *(uint8_t*)0x20001794 = 0x34;
  *(uint8_t*)0x20001795 = 0x80;
  *(uint8_t*)0x20001796 = 8;
  *(uint8_t*)0x20001797 = 9;
  *(uint8_t*)0x20001798 = 5;
  *(uint8_t*)0x20001799 = 0xe;
  *(uint8_t*)0x2000179a = 0x10;
  *(uint16_t*)0x2000179b = 0x400;
  *(uint8_t*)0x2000179d = 0x91;
  *(uint8_t*)0x2000179e = 2;
  *(uint8_t*)0x2000179f = 0x20;
  *(uint8_t*)0x200017a0 = 2;
  *(uint8_t*)0x200017a1 = 0x22;
  *(uint8_t*)0x200017a2 = 2;
  *(uint8_t*)0x200017a3 = 9;
  *(uint8_t*)0x200017a4 = 9;
  *(uint8_t*)0x200017a5 = 5;
  *(uint8_t*)0x200017a6 = 0xd;
  *(uint8_t*)0x200017a7 = 0;
  *(uint16_t*)0x200017a8 = 0x400;
  *(uint8_t*)0x200017aa = 2;
  *(uint8_t*)0x200017ab = 0x1f;
  *(uint8_t*)0x200017ac = 0x9b;
  *(uint8_t*)0x200017ad = 2;
  *(uint8_t*)0x200017ae = 9;
  *(uint8_t*)0x200017af = 2;
  *(uint8_t*)0x200017b0 = 0x31;
  *(uint8_t*)0x200017b1 = 9;
  *(uint8_t*)0x200017b2 = 5;
  *(uint8_t*)0x200017b3 = 0xb;
  *(uint8_t*)0x200017b4 = 8;
  *(uint16_t*)0x200017b5 = 8;
  *(uint8_t*)0x200017b7 = 2;
  *(uint8_t*)0x200017b8 = 0xf6;
  *(uint8_t*)0x200017b9 = 5;
  *(uint8_t*)0x200017ba = 9;
  *(uint8_t*)0x200017bb = 5;
  *(uint8_t*)0x200017bc = 0xa;
  *(uint8_t*)0x200017bd = 0x10;
  *(uint16_t*)0x200017be = 0x200;
  *(uint8_t*)0x200017c0 = 9;
  *(uint8_t*)0x200017c1 = 0x7e;
  *(uint8_t*)0x200017c2 = 4;
  *(uint8_t*)0x200017c3 = 7;
  *(uint8_t*)0x200017c4 = 0x25;
  *(uint8_t*)0x200017c5 = 1;
  *(uint8_t*)0x200017c6 = 0x81;
  *(uint8_t*)0x200017c7 = 0;
  *(uint16_t*)0x200017c8 = 4;
  *(uint8_t*)0x200017ca = 7;
  *(uint8_t*)0x200017cb = 0x25;
  *(uint8_t*)0x200017cc = 1;
  *(uint8_t*)0x200017cd = 1;
  *(uint8_t*)0x200017ce = -1;
  *(uint16_t*)0x200017cf = 4;
  *(uint8_t*)0x200017d1 = 9;
  *(uint8_t*)0x200017d2 = 5;
  *(uint8_t*)0x200017d3 = 1;
  *(uint8_t*)0x200017d4 = 0;
  *(uint16_t*)0x200017d5 = 0;
  *(uint8_t*)0x200017d7 = 0x7f;
  *(uint8_t*)0x200017d8 = 7;
  *(uint8_t*)0x200017d9 = 0x20;
  *(uint8_t*)0x200017da = 2;
  *(uint8_t*)0x200017db = 0xc;
  *(uint8_t*)0x200017dc = 9;
  *(uint8_t*)0x200017dd = 5;
  *(uint8_t*)0x200017de = 5;
  *(uint8_t*)0x200017df = 2;
  *(uint16_t*)0x200017e0 = 0x10;
  *(uint8_t*)0x200017e2 = 0xad;
  *(uint8_t*)0x200017e3 = 0x80;
  *(uint8_t*)0x200017e4 = 0;
  *(uint8_t*)0x200017e5 = 7;
  *(uint8_t*)0x200017e6 = 0x25;
  *(uint8_t*)0x200017e7 = 1;
  *(uint8_t*)0x200017e8 = 0x80;
  *(uint8_t*)0x200017e9 = 7;
  *(uint16_t*)0x200017ea = 0;
  *(uint8_t*)0x200017ec = 7;
  *(uint8_t*)0x200017ed = 0x25;
  *(uint8_t*)0x200017ee = 1;
  *(uint8_t*)0x200017ef = 3;
  *(uint8_t*)0x200017f0 = 0x34;
  *(uint16_t*)0x200017f1 = 8;
  *(uint8_t*)0x200017f3 = 9;
  *(uint8_t*)0x200017f4 = 5;
  *(uint8_t*)0x200017f5 = 1;
  *(uint8_t*)0x200017f6 = 0;
  *(uint16_t*)0x200017f7 = 0x3ff;
  *(uint8_t*)0x200017f9 = 4;
  *(uint8_t*)0x200017fa = 0x8e;
  *(uint8_t*)0x200017fb = 3;
  *(uint8_t*)0x200017fc = 7;
  *(uint8_t*)0x200017fd = 0x25;
  *(uint8_t*)0x200017fe = 1;
  *(uint8_t*)0x200017ff = 5;
  *(uint8_t*)0x20001800 = 0x2e;
  *(uint16_t*)0x20001801 = 6;
  *(uint8_t*)0x20001803 = 9;
  *(uint8_t*)0x20001804 = 5;
  *(uint8_t*)0x20001805 = 3;
  *(uint8_t*)0x20001806 = 1;
  *(uint16_t*)0x20001807 = 0x10;
  *(uint8_t*)0x20001809 = 4;
  *(uint8_t*)0x2000180a = 3;
  *(uint8_t*)0x2000180b = 5;
  *(uint8_t*)0x2000180c = 7;
  *(uint8_t*)0x2000180d = 0x25;
  *(uint8_t*)0x2000180e = 1;
  *(uint8_t*)0x2000180f = 1;
  *(uint8_t*)0x20001810 = 3;
  *(uint16_t*)0x20001811 = 7;
  *(uint8_t*)0x20001813 = 7;
  *(uint8_t*)0x20001814 = 0x25;
  *(uint8_t*)0x20001815 = 1;
  *(uint8_t*)0x20001816 = 1;
  *(uint8_t*)0x20001817 = 1;
  *(uint16_t*)0x20001818 = 0;
  *(uint8_t*)0x2000181a = 9;
  *(uint8_t*)0x2000181b = 5;
  *(uint8_t*)0x2000181c = 0xb;
  *(uint8_t*)0x2000181d = 3;
  *(uint16_t*)0x2000181e = 0x3ff;
  *(uint8_t*)0x20001820 = 0x7f;
  *(uint8_t*)0x20001821 = 4;
  *(uint8_t*)0x20001822 = 8;
  *(uint8_t*)0x20001823 = 2;
  *(uint8_t*)0x20001824 = 0x4d;
  *(uint8_t*)0x20001825 = 9;
  *(uint8_t*)0x20001826 = 5;
  *(uint8_t*)0x20001827 = 2;
  *(uint8_t*)0x20001828 = 3;
  *(uint16_t*)0x20001829 = 0x400;
  *(uint8_t*)0x2000182b = 0xc3;
  *(uint8_t*)0x2000182c = 0x3f;
  *(uint8_t*)0x2000182d = 4;
  *(uint8_t*)0x2000182e = 7;
  *(uint8_t*)0x2000182f = 0x25;
  *(uint8_t*)0x20001830 = 1;
  *(uint8_t*)0x20001831 = 0;
  *(uint8_t*)0x20001832 = 0;
  *(uint16_t*)0x20001833 = 8;
  *(uint8_t*)0x20001835 = 9;
  *(uint8_t*)0x20001836 = 5;
  *(uint8_t*)0x20001837 = 1;
  *(uint8_t*)0x20001838 = 0x10;
  *(uint16_t*)0x20001839 = 0x400;
  *(uint8_t*)0x2000183b = 0;
  *(uint8_t*)0x2000183c = 0;
  *(uint8_t*)0x2000183d = 4;
  *(uint8_t*)0x2000183e = 7;
  *(uint8_t*)0x2000183f = 0x25;
  *(uint8_t*)0x20001840 = 1;
  *(uint8_t*)0x20001841 = 0x80;
  *(uint8_t*)0x20001842 = 8;
  *(uint16_t*)0x20001843 = 3;
  *(uint8_t*)0x20001845 = 7;
  *(uint8_t*)0x20001846 = 0x25;
  *(uint8_t*)0x20001847 = 1;
  *(uint8_t*)0x20001848 = 0x81;
  *(uint8_t*)0x20001849 = 0x40;
  *(uint16_t*)0x2000184a = 0xa2d;
  syz_usb_connect(3, 0x28c, 0x200015c0, 0);
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
