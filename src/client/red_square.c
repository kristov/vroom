#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vrms_client.h"
#include "vrms_geometry.h"
#include "esm.h"

int main(void) {
    uint32_t scene_id;
    uint32_t plane_id;
    vrms_client_t* client;

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

    plane_id = vrms_geometry_plane(client, 2, 2, 1.0, 0.0, 0.0, 1.0);
    if (plane_id == 0) {
        fprintf(stderr, "Unable to create plane\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    vrms_geometry_render_buffer_basic(client, plane_id, 0.0f, 0.0f, -10.0f);

    fprintf(stderr, "sleeping 60 sec\n");
    sleep(10);

    vrms_destroy_scene(client);
    return 0;
}
