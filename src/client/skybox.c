#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vrms_client.h"
#include "vrms_geometry.h"
#include "esm.h"
#include <time.h>

#define NANO_SECOND_MULTIPLIER 1000000
const long INTERVAL_MS = 100 * NANO_SECOND_MULTIPLIER;

int main(void) {
    uint32_t scene_id;
    uint32_t skybox_id;
    uint32_t memory_id;
    uint32_t matrix_size;
    uint32_t render_size;
    vrms_client_t* client;
    uint8_t* shared_mem;
    float* model_matrix;
    uint32_t* render_buffer;

    client = vrms_connect();
    if (NULL == client) {
        fprintf(stderr, "Unable to connect\n");
        exit(1);
    }

    scene_id = vrms_create_scene(client, "Test scene");
    if (scene_id == 0) {
        fprintf(stderr, "Unable to create scene\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    skybox_id = vrms_geometry_skybox(client, "temp/miramar_large_o.png");
    if (skybox_id == 0) {
        fprintf(stderr, "Unable to create skybox\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    matrix_size = sizeof(float) * 16;
    render_size = sizeof(uint32_t) * 3;
    memory_id = vrms_client_create_memory(client, &shared_mem, matrix_size + render_size);

    if (NULL == shared_mem) {
        fprintf(stderr, "Unable to initialize shared memory\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    model_matrix = (float*)shared_mem;
    render_buffer = (uint32_t*)&shared_mem[matrix_size];

    render_buffer[0] = memory_id;
    render_buffer[1] = 0;
    render_buffer[2] = skybox_id;

    esmLoadIdentity(model_matrix);
    vrms_client_render_buffer_set(client, memory_id, matrix_size, render_size);

    sleep(60);

    vrms_destroy_scene(client);
    return 0;
}
