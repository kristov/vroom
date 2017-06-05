#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <openhmd/openhmd.h>
#include <time.h>

#ifdef RASPBERRYPI
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else /* not RASPBERRYPI */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif /* RASPBERRYPI */

#include "esm.h"
#include "vrms_hmd.h"

#define NANO_SECOND_MULTIPLIER 1000000
const long INTERVAL_MS = 50 * NANO_SECOND_MULTIPLIER;

vrms_hmd_t* vrms_hmd_create() {
    vrms_hmd_t* hmd = malloc(sizeof(vrms_hmd_t));
    memset(hmd, 0, sizeof(vrms_hmd_t));
    hmd->matrix_lock = malloc(sizeof(pthread_mutex_t));
    memset(hmd->matrix_lock, 0, sizeof(pthread_mutex_t));
    hmd->matrix = esmCreate();
    return hmd;
}

int32_t vrms_hmd_init(vrms_hmd_t* hmd) {
	hmd->ohmd_ctx = ohmd_ctx_create();

	int num_devices = ohmd_ctx_probe(hmd->ohmd_ctx);
	if(num_devices < 0){
		fprintf(stderr, "failed to probe devices: %s\n", ohmd_ctx_get_error(hmd->ohmd_ctx));
		return -1;
	}

	hmd->ohmd_active_hmd = ohmd_list_open_device(hmd->ohmd_ctx, 0);

	if(!hmd->ohmd_active_hmd){
		fprintf(stderr, "failed to open device: %s\n", ohmd_ctx_get_error(hmd->ohmd_ctx));
		return -1;
	}

    return 0;
}

void vrms_hmd_run(vrms_hmd_t* hmd) {
    float rotation_values[4];
    float zero[] = { 0.0, 0.0, 0.0, 1.0 };
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = INTERVAL_MS;
    while (1) {
		ohmd_ctx_update(hmd->ohmd_ctx);
		ohmd_device_setf(hmd->ohmd_active_hmd, OHMD_ROTATION_QUAT, zero);
		ohmd_device_setf(hmd->ohmd_active_hmd, OHMD_POSITION_VECTOR, zero);
	    ohmd_device_getf(hmd->ohmd_active_hmd, OHMD_ROTATION_QUAT, rotation_values);

        pthread_mutex_lock(hmd->matrix_lock);
        esmLoadIdentity(hmd->matrix);
        esmRotatef(hmd->matrix, rotation_values[0], 1, 0, 0);
        esmRotatef(hmd->matrix, rotation_values[1], 0, 1, 0);
        esmRotatef(hmd->matrix, rotation_values[2], 0, 0, 1);
        pthread_mutex_unlock(hmd->matrix_lock);

        nanosleep(&ts, NULL);
    }
    int nanosleep(const struct timespec *req, struct timespec *rem);
}

void vrms_hmd_destroy(vrms_hmd_t* hmd) {
    ohmd_ctx_destroy(hmd->ohmd_ctx);
    free(hmd);
}
