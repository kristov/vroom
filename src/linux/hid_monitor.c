#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>

#include "hid_device.h"
#include <linux/hidraw.h>

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define HID_MONITOR_MAX_DEVICE_FDS 255

// sudo apt-get install libudev-dev

typedef struct hid_monitor_device hid_monitor_device_t;
struct hid_monitor_device {
    struct udev_device* udev_dev;
    int fd;
    int16_t vendor_id;
    int16_t product_id;
    char* devnode;
    hid_device_t* hid_device;
};

typedef struct hid_monitor hid_monitor_t;
struct hid_monitor {
    int hotplug_fd;
    struct udev* udev;
    struct udev_monitor* udev_mon;
    fd_set* fds;
    int max_fd;
    uint16_t nr_devices;
    hid_monitor_device_t* devices;
    hid_monitor_device_t* fd_lookup[HID_MONITOR_MAX_DEVICE_FDS];
};

void hid_monitor_dump_devices(hid_monitor_t* monitor) {
    uint16_t idx;
    hid_monitor_device_t* device;

    printf("number of devices: %d\n", monitor->nr_devices);

    for (idx = 0; idx < monitor->nr_devices; idx++) {
        device = &monitor->devices[idx];
        printf("  device: %s\n", device->devnode);
        printf(" vid|pid: %02x|%02x\n", device->vendor_id, device->product_id);
        printf("      fd: %d\n", device->fd);
        printf("\n");
    }
}

void hid_monitor_device_destroy(hid_monitor_device_t* device) {
    if (NULL != device->hid_device) {
        hid_device_destroy(device->hid_device);
    }

    if (device->fd > 0) {
        close(device->fd);
    }

    if (NULL != device->devnode) {
        free(device->devnode);
    }

    if (NULL != device->udev_dev) {
        udev_device_unref(device->udev_dev);
    }
}

uint16_t hid_monitor_add_device(hid_monitor_t* monitor, hid_monitor_device_t* device) {
    debug_print("Adding device\n");
    if (NULL == device) {
        return 0;
    }

    monitor->devices = realloc(monitor->devices, sizeof(hid_monitor_device_t) * (monitor->nr_devices + 1));
    memcpy(&monitor->devices[monitor->nr_devices], device, sizeof(hid_monitor_device_t));
    if (device->fd <= HID_MONITOR_MAX_DEVICE_FDS) {
        monitor->fd_lookup[device->fd] = &monitor->devices[monitor->nr_devices];
    }
    
    monitor->nr_devices++;
    return monitor->nr_devices;
}

void hid_monitor_fd_set_destroy(hid_monitor_t* monitor) {
    if (NULL != monitor->fds) {
        free(monitor->fds);
        return;
    }
}

void hid_monitor_fd_set_rebuild(hid_monitor_t* monitor) {
    uint16_t idx;
    hid_monitor_device_t* device;

    if (NULL != monitor->fds) {
        hid_monitor_fd_set_destroy(monitor);
    }

    monitor->fds = malloc(sizeof(fd_set));
    FD_ZERO(monitor->fds);

    debug_print("Getting %d devices\n", monitor->nr_devices);
    for (idx = 0; idx < monitor->nr_devices; idx++) {
        device = &monitor->devices[idx];
        debug_print("Device fd: %d\n", device->fd);
        FD_SET(device->fd, monitor->fds);
        if (device->fd > monitor->max_fd) {
            monitor->max_fd = device->fd;
        }
    }

    if (monitor->hotplug_fd > 0) {
        debug_print("Hotplug fd: %d\n", monitor->hotplug_fd);
        FD_SET(monitor->hotplug_fd, monitor->fds);
        if (monitor->hotplug_fd > monitor->max_fd) {
            monitor->max_fd = monitor->hotplug_fd;
        }
    }
}

uint16_t hid_monitor_udev_device_remove(hid_monitor_t* monitor, struct udev_device *dev) {
    uint16_t idx;
    uint16_t idx_new;
    uint16_t idx_to_remove;
    uint8_t found_device;
    hid_monitor_device_t* device;
    const char* devnode;
    hid_monitor_device_t* devices_new;

    devnode = udev_device_get_devnode(dev);

    idx_to_remove = 0;
    found_device = 0;
    for (idx = 0; idx < monitor->nr_devices; idx++) {
        device = &monitor->devices[idx];
        fprintf(stderr, "removed devnode: %s, this devnode: %s\n", devnode, device->devnode);
        if (strcmp(devnode, device->devnode) == 0) {
            found_device = 1;
            idx_to_remove = idx;
        }
    }

    if (!found_device) {
        fprintf(stderr, "Did not find devnode: %s in devices\n", devnode);
        return monitor->nr_devices;
    }

    debug_print("Allocating space for new device list\n");
    devices_new = malloc(sizeof(hid_monitor_device_t) * (monitor->nr_devices - 1));

    idx_new = 0;
    for (idx = 0; idx < monitor->nr_devices; idx++) {
        if (idx == idx_to_remove) {
            debug_print("Destroying device at %d\n", idx);
            hid_monitor_device_destroy(&monitor->devices[idx]);
        }
        else {
            debug_print("Copying device at %d into %d\n", idx, idx_new);
            memcpy(&devices_new[idx_new], &monitor->devices[idx], sizeof(hid_monitor_device_t));
            idx_new++;
        }
    }
    debug_print("Freeing devices\n");
    free(monitor->devices);

    monitor->devices = devices_new;
    monitor->nr_devices--;

    return monitor->nr_devices;
}

void hid_monitor_udev_device_create(hid_monitor_t* monitor, struct udev_device *dev) {
    int fd;
    uint32_t desc_size;
    struct hidraw_report_descriptor rpt_desc;
    struct hidraw_devinfo info;
    uint8_t res;
    hid_monitor_device_t device;
    const char* devnode;

    devnode = udev_device_get_devnode(dev);

    debug_print("Found %s\n", devnode);
    dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
    if (!dev) {
        debug_print("Unable to find parent usb device\n");
        return;
    }

    fd = open(devnode, O_RDWR|O_NONBLOCK);
    if (fd < 0) {
        debug_print("Unable to open device\n");
        return;
    }

    res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
    if (res < 0) {
        debug_print("ioctl failed on: HIDIOCGRDESCSIZE\n");
        return;
    }

    rpt_desc.size = desc_size;
    res = ioctl(fd, HIDIOCGRDESC, &rpt_desc);
    if (res < 0) {
        debug_print("ioctl failed on: HIDIOCGRDESC\n");
        return;
    }

    res = ioctl(fd, HIDIOCGRAWINFO, &info);
    if (res < 0) {
        debug_print("ioctl failed on: HIDIOCGRAWINFO\n");
        return;
    }

    fprintf(stderr, "length: %d\n", rpt_desc.size);
    device.hid_device = hid_device_report_descriptor(rpt_desc.value, rpt_desc.size);
    device.vendor_id = info.vendor;
    device.product_id = info.product;
    device.devnode = malloc(strlen(devnode) + 1);
    strcpy(device.devnode, devnode);
    device.fd = fd;
    device.udev_dev = dev;

    hid_monitor_add_device(monitor, &device);
}

void hid_monitor_populate_devices(hid_monitor_t* monitor) {
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    
    enumerate = udev_enumerate_new(monitor->udev);
    udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char* path;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(monitor->udev, path);

        hid_monitor_udev_device_create(monitor, dev);
    }

    udev_enumerate_unref(enumerate);

    hid_monitor_dump_devices(monitor);
    debug_print("Rebuilding fd_set\n");
    hid_monitor_fd_set_rebuild(monitor);
}

void hid_monitor_populate_hotplug_fd(hid_monitor_t* monitor) {
    uint8_t res;

    monitor->udev_mon = udev_monitor_new_from_netlink(monitor->udev, "udev");

    res = udev_monitor_filter_add_match_subsystem_devtype(monitor->udev_mon, "hidraw", NULL);
    if (res < 0) {
        debug_print("Unable to add hidraw filter to monitor\n");
        return;
    }

    res = udev_monitor_enable_receiving(monitor->udev_mon);
    if (res < 0) {
        debug_print("Unable to enable receiving\n");
        return;
    }
    monitor->hotplug_fd = udev_monitor_get_fd(monitor->udev_mon);
}

hid_monitor_t* hid_monitor_create() {
    hid_monitor_t* monitor;

    monitor = malloc(sizeof(hid_monitor_t));
    memset(monitor, 0, sizeof(hid_monitor_t));

    monitor->udev = udev_new();
    if (NULL == monitor->udev) {
        fprintf(stderr, "Can not initialize udev\n");
        return NULL;
    }

    debug_print("Populating hotplug file descriptor\n");
    hid_monitor_populate_hotplug_fd(monitor);

    debug_print("Initial device population\n");
    hid_monitor_populate_devices(monitor);

    return monitor;
}

void hid_monitor_destroy(hid_monitor_t* monitor) {
    uint8_t idx;

    for (idx = 0; idx < monitor->nr_devices; idx++) {
        hid_monitor_device_destroy(&monitor->devices[idx]);
    }
    hid_monitor_fd_set_destroy(monitor);
    free(monitor->devices);

    if (monitor->hotplug_fd > 0) {
        close(monitor->hotplug_fd);
    }

    udev_monitor_unref(monitor->udev_mon);
    udev_unref(monitor->udev);

    free(monitor);
}

hid_monitor_device_t* hid_monitor_find_device_by_fd(hid_monitor_t* monitor, int fd) {
    if (fd <= HID_MONITOR_MAX_DEVICE_FDS) {
        if (NULL != monitor->fd_lookup[fd]) {
            return monitor->fd_lookup[fd];
        }
    }
    return NULL;
}

void hid_monitor_device_read_report(hid_monitor_device_t* device, int fd) {
    uint8_t buf[1024];
    int res;
    int res_expected;
    hid_input_report_t* report;
    uint32_t report_id;
    //uint32_t idx;

    res = read(fd, buf, 1024);
    if (res <= 0) {
        // Can happen normally right after a device is unplugged
        return;
    }

    if (hid_device_nr_reports(device->hid_device) > 1) {
        report_id = (uint32_t)buf[0];
        report = hid_device_get_report_by_id(device->hid_device, report_id);
        memmove(buf, buf + 1, res);
        res--;
    }
    else {
        report_id = 0;
        report = &device->hid_device->reports[0];
    }

    if (NULL == report) {
        fprintf(stderr, "Unable to find report id: %d\n", report_id);
        return;
    }

    res_expected = report->byte_size;
    if (res_expected != res) {
        fprintf(stderr, "Read %d bytes but expected %d bytes!!\n", res, res_expected);
    }

    //for (idx = 0; idx < res; idx++) {
    //    printf("%02x ", buf[idx]);
    //}
    //printf("\n");
}

void hid_monitor_process_hotplug(hid_monitor_t* monitor) {
    struct udev_device *dev;
    const char* action;

    fprintf(stderr, "Event from hotplug monitor\n");
    dev = udev_monitor_receive_device(monitor->udev_mon);

    action = udev_device_get_action(dev);

    if (strcmp(action, "add") == 0) {
        hid_monitor_udev_device_create(monitor, dev);
        hid_monitor_dump_devices(monitor);
        hid_monitor_fd_set_rebuild(monitor);
        return;
    }
    else if (strcmp(action, "remove") == 0) {
        hid_monitor_udev_device_remove(monitor, dev);
        hid_monitor_dump_devices(monitor);
        hid_monitor_fd_set_rebuild(monitor);
        return;
    }
    else if (strcmp(action, "change") == 0) {
        fprintf(stderr, "  - change\n");
    }
    else if (strcmp(action, "move") == 0) {
        fprintf(stderr, "  - move\n");
    }
    else {
        fprintf(stderr, "  - Unknown hotplug action: %s\n", action);
    }
}

void hid_monitor_run(hid_monitor_t* monitor) {
    int fd;
    struct timeval tv;
    int res;
    fd_set dup;
    hid_monitor_device_t* device;

    while (1) {
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        dup = *monitor->fds;
        res = select(monitor->max_fd + 1, &dup, NULL, NULL, &tv);

        if (res > 0) {
            for (fd = 0; fd <= monitor->max_fd; fd++) {
                if (FD_ISSET(fd, &dup)) {
                    if (monitor->hotplug_fd == fd) {
                        hid_monitor_process_hotplug(monitor);
                        continue;
                    }
                    device = hid_monitor_find_device_by_fd(monitor, fd);
                    if (NULL == device) {
                        fprintf(stderr, "Event on fd but no device found\n");
                        continue;
                    }
                    hid_monitor_device_read_report(device, fd);
                }
            }
        }
        usleep(4000);
    }
}

int main (void) {
    hid_monitor_t* monitor;

    monitor = hid_monitor_create();
    hid_monitor_run(monitor);

    //hid_monitor_dump_devices(monitor);

    return 0;       
}

/*
    buf[0] = 0x9;
    buf[1] = 0xff;
    buf[2] = 0xff;
    buf[3] = 0xff;
    res = ioctl(fd, HIDIOCSFEATURE(4), buf);
    if (res < 0)
        perror("HIDIOCSFEATURE");
    else
        printf("ioctl HIDIOCGFEATURE returned: %d\n", res);

    buf[0] = 0x9;
    res = ioctl(fd, HIDIOCGFEATURE(256), buf);
    if (res < 0) {
        perror("HIDIOCGFEATURE");
    } else {
        printf("ioctl HIDIOCGFEATURE returned: %d\n", res);
        printf("Report data (not containing the report number):\n\t");
        for (i = 0; i < res; i++)
            printf("%hhx ", buf[i]);
        puts("\n");
    }

    buf[0] = 0x1;
    buf[1] = 0x77;
    res = write(fd, buf, 2);
    if (res < 0) {
        printf("Error: %d\n", errno);
        perror("write");
    } else {
        printf("write() wrote %d bytes\n", res);
    }

    res = read(fd, buf, 16);
    if (res < 0) {
        perror("read");
    } else {
        printf("read() read %d bytes:\n\t", res);
        for (i = 0; i < res; i++)
            printf("%hhx ", buf[i]);
        puts("\n");
    }
    close(fd);
    return 0;
}

*/
