#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vrms_client.h"
#include "vrms_geometry.h"
#include "esm.h"
#include <time.h>

#define NANO_SECOND_MULTIPLIER 1000000
const long INTERVAL_MS = 50 * NANO_SECOND_MULTIPLIER;

int main(void) {
    uint32_t scene_id;
    uint32_t cube_id;
    uint32_t memory_id;
    uint32_t matrix_size;
    uint32_t render_size;
    vrms_client_t* client;
    uint8_t* shared_mem;
    float* model_matrix;
    uint32_t* render_buffer;
    uint8_t nr_loops;
    float translation;
    float rotation;
    struct timespec ts;

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

    cube_id = vrms_geometry_cube(client, 2, 2, 2, 0.7, 1.0, 0.6, 1.0);
    if (cube_id == 0) {
        fprintf(stderr, "Unable to create cube\n");
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
    render_buffer[2] = cube_id;

    ts.tv_sec = 0;
    ts.tv_nsec = INTERVAL_MS;

    nr_loops = 0;
    translation = -10.0f;
    rotation = 0.0f;
    while (nr_loops < 300) {
        translation -= 0.1f;
        rotation += 0.1f;
        nr_loops++;
        esmLoadIdentity(model_matrix);
        esmTranslatef(model_matrix, 0.0f, 0.0f, translation);
        esmRotatef(model_matrix, rotation, 1, 0, 0);
        //esmTranslatef(model_matrix, -1.0f, -1.0f, 0.0f);
        vrms_client_render_buffer_set(client, memory_id, matrix_size, render_size);
        nanosleep(&ts, NULL);
    }

    vrms_destroy_scene(client);
    return 0;
}
