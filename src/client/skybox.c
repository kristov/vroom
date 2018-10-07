#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vrms_client.h"
#include "vrms_geometry.h"
#include "esm.h"
#include <time.h>

int main(void) {
    uint32_t scene_id;
    uint32_t skybox_id;
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

    skybox_id = vrms_geometry_skybox(client, "temp/miramar_large_o.png");
    if (skybox_id == 0) {
        fprintf(stderr, "Unable to create skybox\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    vrms_geometry_render_buffer_basic(client, skybox_id, 0.0f, 0.0f, 0.0f);

    while (1) {
        sleep(60);
    }

    vrms_destroy_scene(client);
    return 0;
}
