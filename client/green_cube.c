#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vroom_client.h>
#include "geometry.h"
#include "memory_layout.h"

void realize_layout_item(memory_layout_t* layout, memory_layout_item_t* item, void* user_data) {
    vroom_client_t* client = (vroom_client_t*)user_data;
    item->id = client->interface->create_object_data(client, layout->id, item->memory_offset, item->memory_size, item->type);
}

void realize_layout(memory_layout_t* layout, void* user_data) {
    vroom_client_t* client = (vroom_client_t*)user_data;
    layout->id = client->interface->create_memory(client, layout->fd, layout->total_size);
}

int main(void) {
    vroom_client_t* client = vroom_connect();
    if (NULL == client) {
        fprintf(stderr, "Unable to connect\n");
        exit(1);
    }

    uint32_t scene_id = client->interface->create_scene(client, "Test scene");
    if (scene_id == 0) {
        fprintf(stderr, "Unable to create scene\n");
        client->interface->destroy_scene(client);
        exit(1);
    }

    memory_layout_t* layout = memory_layout_create(7);
    memory_layout_realizer(layout, realize_layout, (void*)client);
    memory_layout_item_realizer(layout, realize_layout_item, (void*)client);
    geometry_cube_color(layout, 2, 2, 2, 0.7, 1.0, 0.6, 1.0);

    uint32_t register_id = memory_layout_get_id(layout, LAYOUT_DEFAULT_REGISTER);
    uint32_t program_id = memory_layout_get_id(layout, LAYOUT_DEFAULT_PROGRAM);

    client->interface->run_program(client, program_id, register_id);

    fprintf(stderr, "sleeping 60 sec\n");
    sleep(10);

    client->interface->destroy_scene(client);
    return 0;
}
