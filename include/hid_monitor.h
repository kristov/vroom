#include <sys/select.h>

typedef struct hid_monitor_device hid_monitor_device_t;
struct hid_monitor_device {
    struct udev_device* udev_dev;
    int fd;
    int16_t vendor_id;
    int16_t product_id;
    uint32_t report_size;
    uint8_t* report_desc;
    char* devnode;
};

typedef int (*hid_monitor_device_plug_callback_t)(hid_monitor_device_t* device);
typedef void (*hid_monitor_report_callback_t)(hid_monitor_device_t* device, void* data, uint32_t data_length);

typedef struct hid_monitor hid_monitor_t;
struct hid_monitor {
    int hotplug_fd;
    struct udev* udev;
    struct udev_monitor* udev_mon;
    fd_set* fds;
    int max_fd;
    hid_monitor_device_plug_callback_t device_add_callback;
    hid_monitor_device_plug_callback_t device_rem_callback;
    hid_monitor_report_callback_t report_callback;
    uint16_t nr_devices;
    hid_monitor_device_t* devices;
    hid_monitor_device_t** fd_lookup;
};

hid_monitor_t* hid_monitor_create();
void hid_monitor_init(hid_monitor_t* monitor);
void hid_monitor_destroy(hid_monitor_t* monitor);
void hid_monitor_process_events(hid_monitor_t* monitor);
void hid_monitor_run(hid_monitor_t* monitor);
void hid_monitor_dump_devices(hid_monitor_t* monitor);
void hid_monitor_set_device_add_callback(hid_monitor_t* monitor, hid_monitor_device_plug_callback_t callback);
void hid_monitor_set_device_rem_callback(hid_monitor_t* monitor, hid_monitor_device_plug_callback_t callback);
void hid_monitor_set_report_callback(hid_monitor_t* monitor, hid_monitor_report_callback_t callback);
