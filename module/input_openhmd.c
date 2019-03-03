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
    hmd->ohmd_ctx = ohmd_ctx_create();

    int num_devices = ohmd_ctx_probe(hmd->ohmd_ctx);
    if (num_devices < 0) {
        module->interface.log(module, "failed to probe devices: %s", ohmd_ctx_get_error(hmd->ohmd_ctx));
        return 0;
    }
    module->interface.log(module, "found %d active devices", num_devices);

    if (num_devices == 0) {
        module->interface.log(module, "no devices found");
        return 0;
    }

    hmd->ohmd_active_hmd = ohmd_list_open_device(hmd->ohmd_ctx, 0);

    if(!hmd->ohmd_active_hmd){
        module->interface.log(module, "failed to open device: %s", ohmd_ctx_get_error(hmd->ohmd_ctx));
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
