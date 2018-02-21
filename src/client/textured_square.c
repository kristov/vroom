#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vrms_client.h"
#include "vrms_geometry.h"
#include "esm.h"

int main(void) {
    uint32_t scene_id, plane_id, matrix_id;
    uint32_t render_ret;
    vrms_client_t* client;
    float* model_matrix;
    uint32_t render_buffer[3];

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

    plane_id = vrms_geometry_plane_textured(client, 2, 2, "temp/wolf3d.png");
    if (plane_id == 0) {
        fprintf(stderr, "Unable to create cube\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    model_matrix = esmCreate();
    esmTranslatef(model_matrix, -1.0f, -1.0f, -1.0f);
    esmRotatef(model_matrix, 0.2f, 1, 0, 0);

    matrix_id = vrms_geometry_load_matrix_data(client, 1, model_matrix);

    render_buffer[0] = matrix_id;
    render_buffer[1] = 0;
    render_buffer[2] = plane_id;

    render_ret = vrms_geometry_render_buffer_set(client, 1, render_buffer);
    fprintf(stderr, "render_ret: %d\n", render_ret);

    sleep(10);

    vrms_destroy_scene(client);
    return 0;
}
