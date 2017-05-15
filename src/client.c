#include <stdio.h>
#include <stdlib.h>
#include "vrms_client.h"
#include "vrms_geometry.h"

int main(void) {
    uint32_t scene_id, cube_id;
    vrms_client_t* client;

    client = vrms_connect();
    if (NULL == client) {
        fprintf(stderr, "Unable to connect\n");
        exit(1);
    }

    scene_id = vrms_create_scene(client, "Test scene");
    if (scene_id > 0) {
        fprintf(stderr, "Unable to create scene\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    cube_id = vrms_geometry_cube(client, 10, 10, 10, 0.5, 0.5, 0.5, 1.0);
    if (cube_id == 0) {
        fprintf(stderr, "Unable to create cube\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    vrms_destroy_scene(client);
    return 0;
}
