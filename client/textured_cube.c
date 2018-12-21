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

    texture_t texture;
    texture_init(&texture, "assets/wolf3d.png");

    memory_layout_add_vec3(layout, LAYOUT_DEFAULT_VERTEX, 24);
    memory_layout_add_vec3(layout, LAYOUT_DEFAULT_NORMAL, 24);
    memory_layout_add_uint16(layout, LAYOUT_DEFAULT_INDEX, 36);
    memory_layout_add_vec2(layout, LAYOUT_DEFAULT_UV, 24);
    memory_layout_add_uint32(layout, LAYOUT_DEFAULT_REGISTER, 10);
    memory_layout_add_uint8(layout, LAYOUT_DEFAULT_PROGRAM, 1);
    memory_layout_add_mat4(layout, LAYOUT_DEFAULT_MATRIX, 1);
    memory_layout_realize(layout);

    float* verts = memory_layout_get_float_pointer(layout, LAYOUT_DEFAULT_VERTEX);
    float* norms = memory_layout_get_float_pointer(layout, LAYOUT_DEFAULT_NORMAL);
    uint16_t* indicies = memory_layout_get_uint16_pointer(layout, LAYOUT_DEFAULT_INDEX);
    float* colors = memory_layout_get_float_pointer(layout, LAYOUT_DEFAULT_COLOR);
    uint32_t* registers = memory_layout_get_uint32_pointer(layout, LAYOUT_DEFAULT_REGISTER);
    uint8_t* program = memory_layout_get_uint8_pointer(layout, LAYOUT_DEFAULT_PROGRAM);
    float* matrix = memory_layout_get_float_pointer(layout, LAYOUT_DEFAULT_MATRIX);

    geometry_cube_generate_verticies(verts, x, y, z);
    geometry_cube_generate_normals(norms);
    geometry_cube_generate_indicies(indicies);
    geometry_any_generate_color(colors, 24, r, g, b, a);

    memory_layout_item_realize(layout, LAYOUT_DEFAULT_VERTEX);
    memory_layout_item_realize(layout, LAYOUT_DEFAULT_NORMAL);
    memory_layout_item_realize(layout, LAYOUT_DEFAULT_INDEX);
    memory_layout_item_realize(layout, LAYOUT_DEFAULT_COLOR);
    
    registers[0] = memory_layout_get_id(layout, LAYOUT_DEFAULT_VERTEX);
    registers[1] = memory_layout_get_id(layout, LAYOUT_DEFAULT_NORMAL);
    registers[2] = memory_layout_get_id(layout, LAYOUT_DEFAULT_INDEX);
    registers[3] = memory_layout_get_id(layout, LAYOUT_DEFAULT_COLOR);

    mat4_identity(matrix);
    mat4_translatef(matrix, 0.0f, 0.0f, -10.0f);
    memory_layout_item_realize(layout, LAYOUT_DEFAULT_MATRIX);
    registers[4] = memory_layout_get_id(layout, LAYOUT_DEFAULT_MATRIX);
    registers[5] = 0;

    memory_layout_item_realize(layout, LAYOUT_DEFAULT_REGISTER);

    program[0] = 0xc8;
    memory_layout_item_realize(layout, LAYOUT_DEFAULT_PROGRAM);
}
