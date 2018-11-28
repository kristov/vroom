#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vrms_client.h"
#include "vrms_geometry.h"
#include "esm.h"

int main(void) {
    uint32_t scene_id, cube_id1, cube_id2, matrix_id1, matrix_id2;
    uint32_t render_ret;
    vrms_client_t* client;
    float* model_matrix1;
    float* model_matrix2;
    uint32_t render_buffer[6];

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

    cube_id1 = vrms_geometry_cube_textured(client, 2, 2, 2, "temp/wolf3d.png");
    if (cube_id1 == 0) {
        fprintf(stderr, "Unable to create cube\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    cube_id2 = vrms_geometry_cube(client, 1, 1, 2, 0.5, 1.0, 0.5, 1.0);
    if (cube_id2 == 0) {
        fprintf(stderr, "Unable to create cube\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    model_matrix1 = esmCreate();
    esmRotatef(model_matrix1, 2.0f, 1, 0, 0);
    esmTranslatef(model_matrix1, -1.0f, -1.0f, -1.0f);

    matrix_id1 = vrms_geometry_load_matrix_data(client, 1, model_matrix1);

    model_matrix2 = esmCreate();
    esmRotatef(model_matrix2, 2.0f, 1, 0, 0);
    esmTranslatef(model_matrix2, -2.0f, -1.0f, -1.0f);

    matrix_id2 = vrms_geometry_load_matrix_data(client, 1, model_matrix2);

    render_buffer[0] = matrix_id1;
    render_buffer[1] = 0;
    render_buffer[2] = cube_id1;
    render_buffer[3] = matrix_id2;
    render_buffer[4] = 0;
    render_buffer[5] = cube_id2;

    render_ret = vrms_geometry_render_buffer_set(client, 2, render_buffer);
    fprintf(stderr, "render_ret: %d\n", render_ret);

    sleep(10);

    vrms_destroy_scene(client);
    return 0;
}
