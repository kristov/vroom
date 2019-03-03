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

    for (int i = 0; i < num_devices; i++) {
        int device_class = 0, device_flags = 0;
        const char* device_class_s[] = {"HMD", "Controller", "Generic Tracker", "Unknown"};

        ohmd_list_geti(ohmd_ctx, i, OHMD_DEVICE_CLASS, &device_class);
        ohmd_list_geti(ohmd_ctx, i, OHMD_DEVICE_FLAGS, &device_flags);

        printf("device %d\n", i);
        printf("  vendor:  %s\n", ohmd_list_gets(ohmd_ctx, i, OHMD_VENDOR));
        printf("  product: %s\n", ohmd_list_gets(ohmd_ctx, i, OHMD_PRODUCT));
        printf("  path:    %s\n", ohmd_list_gets(ohmd_ctx, i, OHMD_PATH));
        printf("  class:   %s\n", device_class_s[device_class > OHMD_DEVICE_CLASS_GENERIC_TRACKER ? 4 : device_class]);
        printf("  flags:   %02x\n",  device_flags);
        printf("    null device:         %s\n", device_flags & OHMD_DEVICE_FLAGS_NULL_DEVICE ? "yes" : "no");
        printf("    rotational tracking: %s\n", device_flags & OHMD_DEVICE_FLAGS_ROTATIONAL_TRACKING ? "yes" : "no");
        printf("    positional tracking: %s\n", device_flags & OHMD_DEVICE_FLAGS_POSITIONAL_TRACKING ? "yes" : "no");
        printf("    left controller:     %s\n", device_flags & OHMD_DEVICE_FLAGS_LEFT_CONTROLLER ? "yes" : "no");
        printf("    right controller:    %s\n\n", device_flags & OHMD_DEVICE_FLAGS_RIGHT_CONTROLLER ? "yes" : "no");
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
