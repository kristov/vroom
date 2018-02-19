#include <sys/select.h>

/**
 * @file hid_monitor.h
 * @author Chris Eade
 * @date 19th January 2018
 *
 * @brief Monitor HID devices
 *
 * This defines the interface for receiving USB HID plug and unplug, and report events:
 *
 * @code
 * int device_add_callback(hid_monitor_device_t* device) {
 *     // Return 0 to ignore the device and not monitor it
 *     return 1;
 * }
 * 
 * int device_rem_callback(hid_monitor_device_t* device) {
 *     // Return value is ignored
 *     return 0;
 * }
 * 
 * void report_callback(hid_monitor_device_t* device, void* data, uint32_t data_length) {
 *     // Parse the report using the report descriptor and do something with the data
 * }
 * 
 * int main (void) {
 *     hid_monitor_t* monitor;
 * 
 *     monitor = hid_monitor_create();
 * 
 *     hid_monitor_set_device_add_callback(monitor, device_add_callback);
 *     hid_monitor_set_device_rem_callback(monitor, device_rem_callback);
 *     hid_monitor_set_report_callback(monitor, report_callback);
 * 
 *     hid_monitor_init(monitor);
 *     hid_monitor_run(monitor);
 * 
 *     return 0;       
 * }
 * @endcode
 */

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

/**
 * @brief Specification for the device add/remove callback
 *
 * Attach a function of this specification to the add and remove callbacks (via
 * {@link hid_monitor_set_device_add_callback} and {@link
 * hid_monitor_set_device_rem_callback} respectively) to receive device add and
 * removal events. The only argument to this function is the
 * hid_monitor_device_t* that has either just been added, or is juat about to
 * be destroyed.
 */
typedef int (*hid_monitor_device_callback_t)(hid_monitor_device_t* device);

/**
 * @brief Specification for the device report callback
 *
 * Attach a function of this specification to report callback (via {@link
 * hid_monitor_set_device_report_callback}) to receive the device reports. The
 * arguments to the callback are the {@link hid_monitor_device_t} the report is
 * for, a data buffer of the raw report and the length of that buffer in bytes.
 */
typedef void (*hid_monitor_report_callback_t)(hid_monitor_device_t* device, void* data, uint32_t data_length);

typedef struct hid_monitor hid_monitor_t;
struct hid_monitor {
    int hotplug_fd;
    struct udev* udev;
    struct udev_monitor* udev_mon;
    fd_set* fds;
    int max_fd;
    hid_monitor_device_callback_t device_add_callback;
    hid_monitor_device_callback_t device_rem_callback;
    hid_monitor_report_callback_t report_callback;
    uint16_t nr_devices;
    hid_monitor_device_t* devices;
    hid_monitor_device_t** fd_lookup;
};

/**
 * @brief Create a new HID Monitor instance
 *
 * Creates a new HID Monitor instance, initializes it and returns.
 * @code{.c}
 * hid_monitor_t* monitor = hid_monitor_create();
 * @endcode
 * @return Returns an a pointer to a new hid_monitor_t.
 */
hid_monitor_t* hid_monitor_create();

/**
 * @brief Initializes a HID Monitor instance
 *
 * This initializes a HID Monitor instance. It will do the initial device
 * enumeration, and call the device add callback for every device found plugged
 * in.
 * @code{.c}
 * hid_monitor_init(monitor);
 * @endcode
 */
void hid_monitor_init(hid_monitor_t* monitor);

/**
 * @brief Destroy a HID Monitor instance
 *
 * Destroyes a HID Monitor instance, and all attached devices. Closes and open
 * file handes to devices and stops listening to the USB hotplug handle.
 * @code{.c}
 * hid_monitor_destroy(monitor);
 * @endcode
 */
void hid_monitor_destroy(hid_monitor_t* monitor);

/**
 * @brief Process any pending events
 *
 * Processes any events for configured devices. This is non-blocking and should
 * be used inside an loop of some form. Advisable to have some delay in that
 * event loop to avoid hammering the CPU. Any appropriate events will be send
 * via the callbacks.
 * @code{.c}
 * hid_monitor_process_events(monitor);
 * @endcode
 */
void hid_monitor_process_events(hid_monitor_t* monitor);

/**
 * @brief Run an infinite loop for event handling
 *
 * Internally calls {@link hid_monitor_process_events} in a loop with a delay
 * of XXX.
 * @code{.c}
 * hid_monitor_run(monitor);
 * @endcode
 */
void hid_monitor_run(hid_monitor_t* monitor);

/**
 * @brief Dump known devices to stderr
 *
 * Prints a string representation of all monitored devices to stderr.
 * @code{.c}
 * hid_monitor_dump(monitor);
 * @endcode
 */
void hid_monitor_dump_devices(hid_monitor_t* monitor);

/**
 * @brief Set a callback for when a new device is plugged in
 *
 * Used to set the callback function for when a new USB HID device is plugged
 * in. The argument of the callback is the device object that has just been
 * initialized. Returning 1 from the callback will add the device to the
 * monitor, and reports will become available via the report callback.
 * Returning 0 will stop the device being monitored and not reports will
 * arrive.
 * @code{.c}
 * hid_monitor_set_device_add_callback(monitor, my_add_func);
 * @endcode
 * @note If this is not set, the default behaviour is to add all plugged in devices.
 */
void hid_monitor_set_device_add_callback(hid_monitor_t* monitor, hid_monitor_device_callback_t callback);

/**
 * @brief Set a callback for when a device is removed
 *
 * Used to set the callback function for when a USB HID device is unplugged.
 * The argument of the callback is the device object that is just about to be
 * destroyed (all local references to that device must be removed). There is an
 * int return value but it is ignored by the monitor, since it is not possible
 * to prevent anything from happening.
 * @code{.c}
 * hid_monitor_set_device_rem_callback(monitor, my_remove_func);
 * @endcode
 */
void hid_monitor_set_device_rem_callback(hid_monitor_t* monitor, hid_monitor_device_callback_t callback);

/**
 * @brief Set a callback for device reports
 *
 * Used to set the callback function for when there is a new report available
 * for a device. The first argument of this callback is the device, the second
 * argument a pointer to the raw data and the third the byte-size length of
 * that buffer.
 * @code{.c}
 * hid_monitor_set_report_callback(monitor, my_report_func);
 * @endcode
 */
void hid_monitor_set_report_callback(hid_monitor_t* monitor, hid_monitor_report_callback_t callback);
