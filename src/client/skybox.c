#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vrms_client.h"
#include "vrms_geometry.h"
#include "esm.h"

int main(void) {
    uint32_t scene_id, skybox_id;
    uint32_t render_ret;
    vrms_client_t* client;
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

    skybox_id = vrms_geometry_skybox(client, "temp/miramar_large_o.png");
    if (skybox_id == 0) {
        fprintf(stderr, "Unable to create skybox\n");
        vrms_destroy_scene(client);
        exit(1);
    }

    render_buffer[0] = 0;
    render_buffer[1] = 0;
    render_buffer[2] = skybox_id;

    render_ret = vrms_geometry_render_buffer_set(client, 1, render_buffer);
    fprintf(stderr, "render_ret: %d\n", render_ret);

    sleep(10);

    vrms_destroy_scene(client);
    return 0;
}
