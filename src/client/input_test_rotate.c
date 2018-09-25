#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "vrms_client.h"
#include "esm.h"

#define NANO_SECOND_MULTIPLIER 1000000
const long INTERVAL_MS = 50 * NANO_SECOND_MULTIPLIER;

int main(void) {
    uint32_t scene_id;
    uint32_t memory_id;
    uint32_t data_id;
    vrms_client_t* client;
    uint8_t* matrix;
    uint8_t data_size;
    struct timespec ts;
    float angle;

    data_size = sizeof(float) * 16;

    client = vrms_connect();
    if (NULL == client) {
        fprintf(stderr, "Unable to connect\n");
        exit(1);
    }

    scene_id = vrms_create_scene(client, "OpenHMD Input");
    if (0 == scene_id) {
        fprintf(stderr, "Unable to create scene\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    memory_id = vrms_client_create_memory(client, &matrix, data_size);
    if (0 == memory_id) {
        fprintf(stderr, "Unable to create shared memory\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    data_id = vrms_client_create_data_object(client, memory_id, 0, data_size, data_size, sizeof(float), VRMS_MATRIX);
    if (0 == data_id) {
        fprintf(stderr, "Unable to create data object for matrix\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    ts.tv_sec = 0;
    ts.tv_nsec = INTERVAL_MS;

    angle = 0;
    while (1) {
        esmLoadIdentity((float*)matrix);
        esmRotatef((float*)matrix, angle, 0, 1, 0);
        vrms_client_update_system_matrix(client, data_id, 0, VRMS_MATRIX_HEAD, VRMS_UPDATE_SET);
        angle += 0.01;
        if (angle > 6.2) {
            angle = 0;
        }
        nanosleep(&ts, NULL);
    }

    vrms_destroy_scene(client);
    return 0;
}
