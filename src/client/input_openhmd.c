#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "vrms_client.h"
#include "esm.h"
#include <openhmd/openhmd.h>

#define NANO_SECOND_MULTIPLIER 1000000
const long INTERVAL_MS = 50 * NANO_SECOND_MULTIPLIER;

typedef struct hmd {
    ohmd_context* ohmd_ctx;
    ohmd_device* ohmd_active_hmd;
    vrms_client_t* client;
    uint32_t memory_id;
    uint32_t data_size;
    float* matrix;
} hmd_t;

hmd_t* hmd_create() {
    hmd_t* hmd = malloc(sizeof(hmd_t));
    if (NULL == hmd) {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }
    memset(hmd, 0, sizeof(hmd_t));
    return hmd;
}

int8_t hmd_init(hmd_t* hmd) {
	hmd->ohmd_ctx = ohmd_ctx_create();

	int num_devices = ohmd_ctx_probe(hmd->ohmd_ctx);
	if(num_devices < 0){
		fprintf(stderr, "failed to probe devices: %s\n", ohmd_ctx_get_error(hmd->ohmd_ctx));
		return 0;
	}

	hmd->ohmd_active_hmd = ohmd_list_open_device(hmd->ohmd_ctx, 0);

	if(!hmd->ohmd_active_hmd){
		fprintf(stderr, "failed to open device: %s\n", ohmd_ctx_get_error(hmd->ohmd_ctx));
		return 0;
	}

    return 1;
}

void hmd_run(hmd_t* hmd) {
    float rotation_values[4];
    float zero[] = { 0.0, 0.0, 0.0, 1.0 };
    struct timespec ts;
    float* matrix;

    ts.tv_sec = 0;
    ts.tv_nsec = INTERVAL_MS;

    matrix = hmd->matrix;
    while (1) {
		ohmd_ctx_update(hmd->ohmd_ctx);
		ohmd_device_setf(hmd->ohmd_active_hmd, OHMD_ROTATION_QUAT, zero);
		ohmd_device_setf(hmd->ohmd_active_hmd, OHMD_POSITION_VECTOR, zero);
	    ohmd_device_getf(hmd->ohmd_active_hmd, OHMD_ROTATION_QUAT, rotation_values);

        esmLoadIdentity(matrix);
        esmRotatef(matrix, rotation_values[0], 1, 0, 0);
        esmRotatef(matrix, rotation_values[1], 0, 1, 0);
        esmRotatef(matrix, rotation_values[2], 0, 0, 1);

        vrms_client_update_system_matrix(hmd->client, VRMS_MATRIX_HEAD, VRMS_UPDATE_SET, hmd->memory_id, 0, hmd->data_size);
        nanosleep(&ts, NULL);
    }
}

void hmd_destroy(hmd_t* hmd) {
    ohmd_ctx_destroy(hmd->ohmd_ctx);
    free(hmd);
}

int main(void) {
    uint32_t scene_id;
    uint32_t memory_id;
    vrms_client_t* client;
    uint8_t* matrix;
    hmd_t* hmd;

    hmd = hmd_create();
    if (!hmd_init(hmd)) {
        hmd_destroy(hmd);
        return 1;
    }
    hmd->data_size = sizeof(float) * 16;

    client = vrms_connect();
    if (NULL == client) {
        fprintf(stderr, "Unable to connect\n");
        exit(1);
    }
    hmd->client = client;

    scene_id = vrms_create_scene(client, "OpenHMD Input");
    if (0 == scene_id) {
        fprintf(stderr, "Unable to create scene\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    memory_id = vrms_client_create_memory(client, &matrix, hmd->data_size);
    if (0 == memory_id) {
        fprintf(stderr, "Unable to create shared memory\n");
        vrms_destroy_scene(client);
        exit(1);
    }
    hmd->memory_id = memory_id;

    hmd->matrix = (float*)matrix;
    hmd_run(hmd);
    fprintf(stderr, "Out of memory!\n");

    vrms_destroy_scene(client);
    return 0;
}
