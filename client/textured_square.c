#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vroom_client.h>
#include "geometry.h"
#include "memory_layout.h"
#include "texture.h"
#include "gl-matrix.h"

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

    texture_t texture;
    texture_init(&texture, "assets/wolf3d.png");
    uint32_t tbytes = texture.width * texture.height * texture.bytes_per_pixel;
    fprintf(stderr, "texture.width[%d] * texture.height[%d] * texture.bytes_per_pixel[%d] == %d\n", texture.width, texture.height, texture.bytes_per_pixel, tbytes);

    memory_layout_t* layout = memory_layout_create(8);
    memory_layout_realizer(layout, realize_layout, (void*)client);
    memory_layout_item_realizer(layout, realize_layout_item, (void*)client);

    memory_layout_add_vec3(layout, 0, 4);       // vertex
    memory_layout_add_vec3(layout, 1, 4);       // normal
    memory_layout_add_uint16(layout, 2, 6);     // index
    memory_layout_add_vec2(layout, 3, 4);       // uv
    memory_layout_add_uint8(layout, 4, tbytes); // texture
    memory_layout_add_uint32(layout, 5, 10);    // register
    memory_layout_add_uint8(layout, 6, 1);      // program
    memory_layout_add_mat4(layout, 7, 1);       // matrix
    memory_layout_realize(layout);

    float* verts = memory_layout_get_float_pointer(layout, 0);
    float* norms = memory_layout_get_float_pointer(layout, 1);
    uint16_t* indicies = memory_layout_get_uint16_pointer(layout, 2);
    float* uvs = memory_layout_get_float_pointer(layout, 3);
    uint8_t* tex = memory_layout_get_uint8_pointer(layout, 4);
    uint32_t* registers = memory_layout_get_uint32_pointer(layout, 5);
    uint8_t* program = memory_layout_get_uint8_pointer(layout, 6);
    float* matrix = memory_layout_get_float_pointer(layout, 7);

    geometry_plane_generate_verticies(verts, 0, 0, 2, 2);
    geometry_plane_generate_normals(norms);
    geometry_plane_generate_indicies(indicies);
    geometry_plane_generate_uvs(uvs);
    memcpy(tex, texture.data, tbytes);

    memory_layout_item_realize(layout, 0);
    memory_layout_item_realize(layout, 1);
    memory_layout_item_realize(layout, 2);
    memory_layout_item_realize(layout, 3);
    memory_layout_item_realize(layout, 4);

    registers[0] = memory_layout_get_id(layout, 0);
    registers[1] = memory_layout_get_id(layout, 1);
    registers[2] = memory_layout_get_id(layout, 2);
    registers[3] = 0;

    mat4_identity(matrix);
    mat4_translatef(matrix, 0.0f, 0.0f, -10.0f);
    memory_layout_item_realize(layout, 7);

    registers[4] = memory_layout_get_id(layout, 7);
    registers[5] = 0;
    registers[6] = memory_layout_get_id(layout, 3);

    program[0] = 0xc9;
    memory_layout_item_realize(layout, 6);

    uint32_t data_id = memory_layout_get_id(layout, 4);
    registers[7] = client->interface->create_object_texture(client, data_id, texture.width, texture.height, VRMS_FORMAT_BGR888, VRMS_TEXTURE_2D);
    memory_layout_item_realize(layout, 5);

    uint32_t register_id = memory_layout_get_id(layout, 5);
    uint32_t program_id = memory_layout_get_id(layout, 6);

    sleep(2);

    client->interface->run_program(client, program_id, register_id);

    fprintf(stderr, "sleeping 60 sec\n");
    sleep(10);

    client->interface->destroy_scene(client);
    return 0;
}
