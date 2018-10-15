#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "vrms_runtime.h"
#include "esm.h"

#define NANO_SECOND_MULTIPLIER 1000000
const long INTERVAL_MS = 50 * NANO_SECOND_MULTIPLIER;

void* run_module(vrms_runtime_t* vrms_runtime) {
    float matrix[16];
    struct timespec ts;
    float angle;

    ts.tv_sec = 0;
    ts.tv_nsec = INTERVAL_MS;

    angle = 0;
    while (1) {
        esmLoadIdentity(matrix);
        esmRotatef(matrix, angle, 0, 1, 0);
        vrms_runtime_update_system_matrix_module(vrms_runtime, VRMS_MATRIX_HEAD, VRMS_UPDATE_SET, matrix);
        angle += 0.01;
        if (angle > 6.2) {
            angle = 0;
        }
        nanosleep(&ts, NULL);
    }

    return NULL;
}
