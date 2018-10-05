#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include "esm.h"
#include "vrms_runtime.h"
#include "libinput.h"

static void log_handler(struct libinput *li, enum libinput_log_priority priority, const char *format, va_list args) {
    vprintf(format, args);
}

static int open_restricted(const char *path, int flags, void *user_data) {
    int fd = open(path, flags);
    return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data) {
    close(fd);
}

const static struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

static struct libinput* input_libinput_open_udev(const char *seat) {
    struct libinput* li;
    struct udev* udev = udev_new();
    bool grab;

    if (!udev) {
        fprintf(stderr, "Failed to initialize udev\n");
        return NULL;
    }

    grab = 1;
    li = libinput_udev_create_context(&interface, &grab, udev);
    if (!li) {
        fprintf(stderr, "Failed to initialize context from udev\n");
        return li;
    }

    libinput_log_set_handler(li, log_handler);
    libinput_log_set_priority(li, LIBINPUT_LOG_PRIORITY_DEBUG);

    if (libinput_udev_assign_seat(li, seat)) {
        fprintf(stderr, "Failed to set seat\n");
        libinput_unref(li);
        li = NULL;
        return li;
    }

    udev_unref(udev);
    return li;
}

static int handle_and_print_events(struct libinput *li) {
    int rc = -1;
    struct libinput_event *ev;

    libinput_dispatch(li);
    while ((ev = libinput_get_event(li))) {
        fprintf(stderr, "EVENT\n");
        rc = 0;
    }
    return rc;
}

void* run_module(void* data) {
    struct libinput *li;
    struct pollfd fds;

    li = input_libinput_open_udev("Vroom input_libinput module");

    fds.fd = libinput_get_fd(li);
    fds.events = POLLIN;
    fds.revents = 0;

    if (handle_and_print_events(li)) {
        fprintf(stderr, "unable to initialise input_libinput module (perms)\n");
        return NULL;
    }

    while (poll(&fds, 1, -1) > -1) {
        handle_and_print_events(li);
    }

    libinput_unref(li);
    return NULL;       
}
