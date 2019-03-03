#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "runtime.h"
#include "esm.h"
#include <openhmd/openhmd.h>

#define NANO_SECOND_MULTIPLIER 1000000
const long INTERVAL_MS = 30 * NANO_SECOND_MULTIPLIER;

typedef struct hmd {
    ohmd_context* ohmd_ctx;
    ohmd_device* ohmd_active_hmd;
} hmd_t;

int8_t hmd_init(vrms_module_t* module, hmd_t* hmd) {
    ohmd_context* ohmd_ctx = ohmd_ctx_create();

    if (!ohmd_ctx) {
        module->interface.error(module, "unable to create OHMD context");
        return 0;
    }
    hmd->ohmd_ctx = ohmd_ctx;

    int num_devices = ohmd_ctx_probe(ohmd_ctx);
    if (num_devices < 0) {
        module->interface.error(module, "failed to probe devices: %s", ohmd_ctx_get_error(ohmd_ctx));
        return 0;
    }
    module->interface.debug(module, "found %d active devices", num_devices);

    if (num_devices == 0) {
        module->interface.debug(module, "no devices found");
        return 0;
    }

    uint8_t found_hmd = 0;
    for (int i = 0; i < num_devices; i++) {
        int device_class = 0, device_flags = 0;
        const char* device_class_s[] = {"HMD", "Controller", "Generic Tracker", "Unknown"};

        ohmd_list_geti(ohmd_ctx, i, OHMD_DEVICE_CLASS, &device_class);
        ohmd_list_geti(ohmd_ctx, i, OHMD_DEVICE_FLAGS, &device_flags);

        if (device_class == 0 && !(device_flags & OHMD_DEVICE_FLAGS_NULL_DEVICE)) {
            found_hmd = 1;
        }

        module->interface.debug(module, "device %d", i);
        module->interface.debug(module, "  vendor:  %s", ohmd_list_gets(ohmd_ctx, i, OHMD_VENDOR));
        module->interface.debug(module, "  product: %s", ohmd_list_gets(ohmd_ctx, i, OHMD_PRODUCT));
        module->interface.debug(module, "  path:    %s", ohmd_list_gets(ohmd_ctx, i, OHMD_PATH));
        module->interface.debug(module, "  class:   %s [%d]", device_class_s[device_class > OHMD_DEVICE_CLASS_GENERIC_TRACKER ? 4 : device_class], device_class);
        module->interface.debug(module, "  flags:   %02x",  device_flags);
        module->interface.debug(module, "    null device:         %s", device_flags & OHMD_DEVICE_FLAGS_NULL_DEVICE ? "yes" : "no");
        module->interface.debug(module, "    rotational tracking: %s", device_flags & OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING ? "yes" : "no");
        module->interface.debug(module, "    positional tracking: %s", device_flags & OHMD_DEVICE_FLAGS_POSITIONAL_TRACKING ? "yes" : "no");
        module->interface.debug(module, "    left controller:     %s", device_flags & OHMD_DEVICE_FLAGS_LEFT_CONTROLLER ? "yes" : "no");
        module->interface.debug(module, "    right controller:    %s", device_flags & OHMD_DEVICE_FLAGS_RIGHT_CONTROLLER ? "yes" : "no");
    }

    if (!found_hmd) {
        module->interface.debug(module, "did not find a non-dummy HMD device");
        return 0;
    }

    hmd->ohmd_active_hmd = ohmd_list_open_device(ohmd_ctx, 0);

    if(!hmd->ohmd_active_hmd){
        module->interface.error(module, "failed to open device: %s", ohmd_ctx_get_error(ohmd_ctx));
        return 0;
    }

    return 1;
}

void hmd_destroy(hmd_t* hmd) {
    ohmd_ctx_destroy(hmd->ohmd_ctx);
}

void* run_module(vrms_module_t* module) {
    float matrix[16];
    float rotation_values[4];
    float zero[] = { 0.0, 0.0, 0.0, 1.0 };
    struct timespec ts;
    hmd_t hmd;

    if (!hmd_init(module, &hmd)) {
        hmd_destroy(&hmd);
        return NULL;
    }

    ts.tv_sec = 0;
    ts.tv_nsec = INTERVAL_MS;

    while (1) {
        ohmd_ctx_update(hmd.ohmd_ctx);
        ohmd_device_setf(hmd.ohmd_active_hmd, OHMD_ROTATION_QUAT, zero);
        ohmd_device_setf(hmd.ohmd_active_hmd, OHMD_POSITION_VECTOR, zero);
        ohmd_device_getf(hmd.ohmd_active_hmd, OHMD_ROTATION_QUAT, rotation_values);

        esmQuaternionToMatrix(matrix, rotation_values[0], rotation_values[1], rotation_values[2], rotation_values[3]);
        //esmDump(matrix, "input_openhmd matrix");

        module->interface.update_system_matrix(module, VRMS_MATRIX_HEAD, VRMS_UPDATE_SET, matrix);
        nanosleep(&ts, NULL);
    }

    return NULL;
}
