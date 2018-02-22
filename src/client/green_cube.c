#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vrms_client.h"
#include "vrms_geometry.h"
#include "esm.h"

int main(void) {
    uint32_t scene_id;
    uint32_t cube_id;
    uint32_t memory_id;
    uint32_t render_ret;
    uint32_t matrix_size;
    uint32_t render_size;
    vrms_client_t* client;
    uint8_t* shared_mem;
    float* model_matrix;
    uint32_t* render_buffer;
    uint8_t nr_loops;

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

    //esmRotatef(model_matrix, 0.1f, 0, 1, 0);
    esmLoadIdentity(model_matrix);
    esmTranslatef(model_matrix, -1.0f, -1.0f, -5.0f);
    //esmRotatef(model_matrix, 0.5f, 1, 0, 0);

    render_buffer[0] = memory_id;
    render_buffer[1] = 0;
    render_buffer[2] = cube_id;

    render_ret = vrms_client_render_buffer_set(client, memory_id, matrix_size, sizeof(uint32_t) * 3);
    fprintf(stderr, "render_ret: %d\n", render_ret);

    nr_loops = 0;
    while (nr_loops < 30) {
        sleep(1);
        nr_loops++;
        esmRotatef(model_matrix, 0.5f, 1, 0, 0);
        render_ret = vrms_client_render_buffer_set(client, memory_id, matrix_size, sizeof(uint32_t) * 3);
        fprintf(stderr, "render_ret: %d\n", render_ret);
    }

    vrms_destroy_scene(client);
    return 0;
}
