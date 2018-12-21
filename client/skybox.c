#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "client.h"
#include "geometry.h"
#include "memory_layout.h"
#include "texture.h"

void realize_layout_item(memory_layout_t* layout, memory_layout_item_t* item, void* user_data) {
    vrms_client_t* client = (vrms_client_t*)user_data;
    item->id = client->interface->create_object_data(client, layout->id, item->memory_offset, item->memory_size, item->type);
}

void realize_layout(memory_layout_t* layout, void* user_data) {
    vrms_client_t* client = (vrms_client_t*)user_data;
    layout->id = client->interface->create_memory(client, layout->fd, layout->total_size);
}

int main(void) {
    vrms_client_t* client = vrms_connect();
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

    texture_cubemap_t cubemap;
    texture_cubemap_init(&cubemap, "assets/miramar_large_o.png");
    //texture_cubemap_init(&cubemap, "assets/skybox_desert.png");
    fprintf(stderr, "cubemap: square_width: %d total_size: %d\n", cubemap.square_width, cubemap.total_size);

    memory_layout_t* layout = memory_layout_create(1);
    memory_layout_realizer(layout, realize_layout, (void*)client);
    memory_layout_item_realizer(layout, realize_layout_item, (void*)client);

    memory_layout_add_item(layout, 0, VRMS_UINT8, cubemap.total_size);
    memory_layout_realize(layout);

    uint8_t* cubemap_data = memory_layout_get_uint8_pointer(layout, 0);
    texture_cubemap_build_data(&cubemap, cubemap_data);

    memory_layout_item_realize(layout, 0);

    uint32_t texture_id = client->interface->create_object_texture(client, layout->items[0].id, cubemap.square_width, cubemap.square_width, VRMS_FORMAT_BGR888, VRMS_TEXTURE_CUBE_MAP);

    fprintf(stderr, "created texture_id: %d\n", texture_id);

    uint32_t ok = client->interface->set_skybox(client, texture_id);
    if (!ok) {
        fprintf(stderr, "skybox creation failed\n");
    }

    while (1) {
        sleep(10);
    }

    client->interface->destroy_scene(client);
    return 0;
}
