#include <stdio.h>
#include <string.h>
#include "safemalloc.h"
#include "vrms_client.h"
#include "render_vm.h"
#include "vrms_geometry.h"
#include "gl-matrix.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void std_cube_generate_verticies(float* verts, uint32_t x, uint32_t y, uint32_t z) {
    uint32_t off = 0;

    // front
    verts[off + 0] = 0.0f; // 0
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 1
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = 0.0f; // 2
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 3
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;

    // left
    verts[off + 0] = 0.0f; // 4
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = 0.0f; // 5
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = 0.0f; // 6
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = 0.0f; // 7
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;

    // right
    verts[off + 0] = x; // 8
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 9
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 10
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 11
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;

    // top
    verts[off + 0] = x; // 12
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = 0.0f; // 13
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 14
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = 0.0f; // 15
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;

    // bottom
    verts[off + 0] = 0.0f; // 16
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 17
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = 0.0f; // 18
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 19
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;

    // back
    verts[off + 0] = 0.0f; // 20
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 21
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = 0.0f; // 22
    verts[off + 1] = y;
    verts[off + 2] = z;
    off += 3;
    verts[off + 0] = x; // 23
    verts[off + 1] = 0.0f;
    verts[off + 2] = z;
    off += 3;
}

void std_cube_generate_normals(float* norms) {
    uint32_t off = 0;

    // [+-1, 0, 0],   [0,+-1,0],    [0,0,+-1]
    // (Left/Right), (Top/Bottom), (Front/Back)

    // front
    norms[off + 0] = 0.0f; // 0
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 1
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 2
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 3
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;

    // left
    norms[off + 0] = -1.0f; // 4
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = -1.0f; // 5
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = -1.0f; // 6
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = -1.0f; // 7
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;

    // right
    norms[off + 0] = 1.0f; // 8
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 1.0f; // 9
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 1.0f; // 10
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 1.0f; // 11
    norms[off + 1] = 0.0f;
    norms[off + 2] = 0.0f;
    off += 3;

    // top
    norms[off + 0] = 0.0f; // 12
    norms[off + 1] = 1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 13
    norms[off + 1] = 1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 14
    norms[off + 1] = 1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 15
    norms[off + 1] = 1.0f;
    norms[off + 2] = 0.0f;
    off += 3;

    // bottom
    norms[off + 0] = 0.0f; // 16
    norms[off + 1] = -1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 17
    norms[off + 1] = -1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 18
    norms[off + 1] = -1.0f;
    norms[off + 2] = 0.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 19
    norms[off + 1] = -1.0f;
    norms[off + 2] = 0.0f;
    off += 3;

    // back
    norms[off + 0] = 0.0f; // 20
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 21
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 22
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 23
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
}

void std_cube_generate_indicies(uint16_t* indicies) {
    uint32_t off = 0;

    // front
    indicies[off + 0] = 0;
    indicies[off + 1] = 1;
    indicies[off + 2] = 2;
    indicies[off + 3] = 1;
    indicies[off + 4] = 2;
    indicies[off + 5] = 3;
    off += 6;

    // left
    indicies[off + 0] = 4;
    indicies[off + 1] = 5;
    indicies[off + 2] = 6;
    indicies[off + 3] = 4;
    indicies[off + 4] = 5;
    indicies[off + 5] = 7;
    off += 6;

    // right
    indicies[off + 0] = 8;
    indicies[off + 1] = 9;
    indicies[off + 2] = 10;
    indicies[off + 3] = 8;
    indicies[off + 4] = 9;
    indicies[off + 5] = 11;
    off += 6;

    // top
    indicies[off + 0] = 12;
    indicies[off + 1] = 13;
    indicies[off + 2] = 14;
    indicies[off + 3] = 12;
    indicies[off + 4] = 13;
    indicies[off + 5] = 15;
    off += 6;

    // bottom
    indicies[off + 0] = 16;
    indicies[off + 1] = 17;
    indicies[off + 2] = 18;
    indicies[off + 3] = 16;
    indicies[off + 4] = 17;
    indicies[off + 5] = 19;
    off += 6;

    // back
    indicies[off + 0] = 20;
    indicies[off + 1] = 21;
    indicies[off + 2] = 22;
    indicies[off + 3] = 20;
    indicies[off + 4] = 21;
    indicies[off + 5] = 23;
    off += 6;
}

void std_cube_generate_uvs(float* uvs) {
    uint32_t off = 0;

    // front
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    // left
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    // right
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    // top
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    // bottom
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    // back
    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;
}

uint32_t vrms_geometry_cube(vrms_client_t* client, uint32_t x, uint32_t y, uint32_t z, float r, float g, float b, float a) {
    uint32_t nr_verticies;
    uint32_t nr_indicies;
    uint32_t size_of_vert;
    uint32_t size_of_verts;
    uint32_t size_of_index;
    uint32_t size_of_indicies;
    uint32_t memory_id;
    uint8_t* address = NULL;
    float* verts;
    float* norms;
    uint16_t* indicies;
    uint32_t buff_off;

    nr_verticies = 4 * 6;
    nr_indicies = 6 * 6;

    size_of_vert = sizeof(float) * 3;
    size_of_index = sizeof(uint16_t);

    size_of_verts = size_of_vert * nr_verticies;
    size_of_indicies = size_of_index * nr_indicies;

    verts = SAFEMALLOC(size_of_verts);
    norms = SAFEMALLOC(size_of_verts);
    indicies = SAFEMALLOC(size_of_indicies);

    std_cube_generate_verticies(verts, x, y, z);
    std_cube_generate_normals(norms);
    std_cube_generate_indicies(indicies);

    memory_id = vrms_client_create_memory(client, &address, (size_of_verts * 2) + size_of_indicies);
    if (0 == memory_id) {
        return 0;
    }

    buff_off = 0;
    memcpy(&address[buff_off], verts, size_of_verts);
    uint32_t vertex_id = vrms_client_create_data_object(client, memory_id, buff_off, size_of_verts, size_of_vert, sizeof(float), VRMS_VERTEX);

    buff_off += size_of_verts;
    memcpy(&address[buff_off], norms, size_of_verts);
    uint32_t normal_id = vrms_client_create_data_object(client, memory_id, buff_off, size_of_verts, size_of_vert, sizeof(float), VRMS_NORMAL);

    buff_off += size_of_verts;
    memcpy(&address[buff_off], indicies, size_of_indicies);
    uint32_t index_id = vrms_client_create_data_object(client, memory_id, buff_off, size_of_indicies, size_of_index, size_of_index, VRMS_INDEX);

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);

    uint32_t mesh_id = vrms_client_create_mesh_color(client, geometry_id, r, g, b, a);

    free(verts);
    free(norms);
    free(indicies);

    return mesh_id;
}

// destination == where the square should go
// source    == source data
// square_px == nr. pixels per side of the square
// bytes_pl  == bytes per line original image
// bytes_pp  == bytes per pixel
// x_idx     == index of the square in x
// y_idx     == index of the square in y
// dest_off  == location to put the data
void vrms_geometry_image_copy_square(uint8_t* destination, uint8_t* source, uint32_t square_px, uint32_t bytes_pl, uint32_t bytes_pp, uint32_t x_idx, uint32_t y_idx, uint32_t dest_off) {
    uint32_t i, start, length;

    start = ((square_px * y_idx) * bytes_pl) + ((square_px * x_idx) * bytes_pp);
    length = (square_px * bytes_pp);

    for (i = 0; i < square_px; i++) {
        memcpy(&destination[dest_off], &source[start], length);
        start += bytes_pl;
        dest_off += length;
    }
}

uint32_t vrms_load_skybox_texture(vrms_client_t* client, const char* filename) {
    uint32_t memory_id;
    uint32_t data_id;
    uint32_t texture_id;
    int32_t width;
    int32_t height;
    int32_t bytes_pp;
    uint8_t* data = NULL;
    uint8_t* address = NULL;
    uint32_t total_byte_size;
    uint32_t square_px;
    uint32_t bytes_sq;
    uint32_t bytes_pl;
    uint32_t dest_off;

    width = 0;
    height = 0;
    bytes_pp = 0;
    data = stbi_load(filename, &width, &height, &bytes_pp, 3);
    if (!data) {
        return 0;
    }

    square_px = width / 4;
    bytes_pl = width * bytes_pp;
    bytes_sq = square_px * square_px * bytes_pp;
    total_byte_size = bytes_sq * 6;

    memory_id = vrms_client_create_memory(client, &address, total_byte_size);
    if (0 == memory_id) {
        stbi_image_free(data);
        return 0;
    }

    dest_off = 0;

    /*
          [YPOS]
    [XNEG][ZPOS][XPOS][ZNEG]
          [YNEG]
    */

    // XPOS
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 2, 1, dest_off);
    dest_off += bytes_sq;

    // XNEG
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 0, 1, dest_off);
    dest_off += bytes_sq;

    // YPOS
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 1, 2, dest_off);
    dest_off += bytes_sq;

    // YNEG
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 1, 0, dest_off);
    dest_off += bytes_sq;

    // ZPOS
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 1, 1, dest_off);
    dest_off += bytes_sq;

    // ZNEG
    vrms_geometry_image_copy_square(address, data, square_px, bytes_pl, bytes_pp, 3, 1, dest_off);
    dest_off += bytes_sq;

    data_id = vrms_client_create_data_object(client, memory_id, 0, total_byte_size, bytes_pp, 1, VRMS_TEXTURE);

    texture_id = vrms_client_create_texture_object(client, data_id, square_px, square_px, VRMS_FORMAT_BGR888, VRMS_TEXTURE_CUBE_MAP);

    return texture_id;
}

uint32_t vrms_load_texture(vrms_client_t* client, const char* filename, vrms_texture_type_t type) {
    int32_t memory_id;
    int32_t data_id;
    int32_t texture_id;
    int32_t width, height, bytes_pp;
    uint8_t* data = NULL;
    uint8_t* address = NULL;
    uint32_t total_byte_size;

    width = 0;
    height = 0;
    bytes_pp = 0;
    data = stbi_load(filename, &width, &height, &bytes_pp, 3);
    if (!data) {
        stbi_image_free(data);
        return 0;
    }

    total_byte_size = width * height * bytes_pp;

    memory_id = vrms_client_create_memory(client, &address, total_byte_size);
    if (0 == memory_id) {
        return 0;
    }

    memcpy(address, data, total_byte_size);

    data_id = vrms_client_create_data_object(client, memory_id, 0, total_byte_size, bytes_pp, 1, VRMS_TEXTURE);

    texture_id = vrms_client_create_texture_object(client, data_id, width, height, VRMS_FORMAT_BGR888, type);

    return texture_id;
}

uint32_t vrms_geometry_cube_textured(vrms_client_t* client, uint32_t x, uint32_t y, uint32_t z, const char* filename) {
    uint32_t nr_verticies;
    uint32_t nr_indicies;
    uint32_t size_of_vert;
    uint32_t size_of_verts;
    uint32_t size_of_index;
    uint32_t size_of_indicies;
    uint32_t size_of_uv;
    uint32_t size_of_uvs;
    uint32_t size_total;
    uint32_t buff_off;
    int32_t memory_id;
    uint8_t* address = NULL;
    float* verts;
    float* norms;
    uint16_t* indicies;
    float* uvs;

    nr_verticies = 4 * 6;
    nr_indicies = 6 * 6;

    size_of_vert = sizeof(float) * 3;
    size_of_uv = sizeof(float) * 2;
    size_of_index = sizeof(uint16_t);

    size_of_verts = size_of_vert * nr_verticies;
    size_of_uvs = size_of_uv * nr_verticies;
    size_of_indicies = size_of_index * nr_indicies;

    verts = SAFEMALLOC(size_of_verts);
    norms = SAFEMALLOC(size_of_verts);
    indicies = SAFEMALLOC(size_of_indicies);
    uvs = SAFEMALLOC(size_of_uvs);

    std_cube_generate_verticies(verts, x, y, z);
    std_cube_generate_normals(norms);
    std_cube_generate_indicies(indicies);
    std_cube_generate_uvs(uvs);

    size_total = size_of_verts + size_of_verts + size_of_indicies + size_of_uvs;

    memory_id = vrms_client_create_memory(client, &address, size_total);
    if (0 == memory_id) {
        return 0;
    }

    buff_off = 0;
    memcpy(&address[buff_off], verts, size_of_verts);
    uint32_t vertex_id = vrms_client_create_data_object(client, memory_id, buff_off, size_of_verts, size_of_vert, sizeof(float), VRMS_VERTEX);

    buff_off += size_of_verts;
    memcpy(&address[buff_off], norms, size_of_verts);
    uint32_t normal_id = vrms_client_create_data_object(client, memory_id, buff_off, size_of_verts, size_of_vert, sizeof(float), VRMS_NORMAL);

    buff_off += size_of_verts;
    memcpy(&address[buff_off], indicies, size_of_indicies);
    uint32_t index_id = vrms_client_create_data_object(client, memory_id, buff_off, size_of_indicies, size_of_index, sizeof(uint16_t), VRMS_INDEX);

    buff_off += size_of_indicies;
    memcpy(&address[buff_off], uvs, size_of_uvs);
    uint32_t uv_id = vrms_client_create_data_object(client, memory_id, buff_off, size_of_uvs, size_of_uv, sizeof(float), VRMS_UV);

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);
    uint32_t texture_id = vrms_load_texture(client, filename, VRMS_TEXTURE_2D);
    if (0 == texture_id) {
        fprintf(stderr, "unable to create texture\n");
    }

    uint32_t mesh_id = vrms_client_create_mesh_texture(client, geometry_id, texture_id, uv_id);

    free(verts);
    free(norms);
    free(indicies);
    free(uvs);

    return mesh_id;
}

uint32_t vrms_geometry_skybox(vrms_client_t* client, const char* filename) {
    uint32_t texture_id, skybox_id;

    texture_id = vrms_load_skybox_texture(client, filename);
    skybox_id = vrms_client_create_skybox(client, texture_id);

    return skybox_id;
}

void std_plane_generate_verticies(float* verts, uint32_t x, uint32_t y) {
    uint32_t off = 0;

    verts[off + 0] = 0.0f; // 0
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 1
    verts[off + 1] = 0.0f;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = 0.0f; // 2
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x; // 3
    verts[off + 1] = y;
    verts[off + 2] = 0.0f;
    off += 3;
}

void std_plane_generate_normals(float* norms) {
    uint32_t off = 0;

    norms[off + 0] = 0.0f; // 0
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 1
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 2
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 3
    norms[off + 1] = 0.0f;
    norms[off + 2] = 1.0f;
    off += 3;
}

void std_plane_generate_indicies(uint16_t* indicies) {
    uint32_t off = 0;

    indicies[off + 0] = 0;
    indicies[off + 1] = 1;
    indicies[off + 2] = 2;
    indicies[off + 3] = 1;
    indicies[off + 4] = 2;
    indicies[off + 5] = 3;
    off += 6;
}

void std_plane_generate_uvs(float* uvs) {
    uint32_t off = 0;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;
}

uint32_t vrms_geometry_plane(vrms_client_t* client, uint32_t x, uint32_t y, float r, float g, float b, float a) {
    uint32_t nr_verticies;
    uint32_t nr_indicies;
    uint32_t size_of_vert;
    uint32_t size_of_verts;
    uint32_t size_of_index;
    uint32_t size_of_indicies;
    int32_t memory_id;
    uint8_t* address = NULL;
    float* verts;
    float* norms;
    uint16_t* indicies;

    nr_verticies = 4;
    nr_indicies = 6;

    size_of_vert = sizeof(float) * 3;
    size_of_index = sizeof(uint16_t);

    size_of_verts = size_of_vert * nr_verticies;
    size_of_indicies = size_of_index * nr_indicies;

    verts = SAFEMALLOC(size_of_verts);
    norms = SAFEMALLOC(size_of_verts);
    indicies = SAFEMALLOC(size_of_indicies);

    std_plane_generate_verticies(verts, x, y);
    std_plane_generate_normals(norms);
    std_plane_generate_indicies(indicies);

    memory_id = vrms_client_create_memory(client, &address, (size_of_verts * 2) + size_of_indicies);
    if (0 == memory_id) {
        return 0;
    }

    memcpy(address, verts, size_of_verts);
    memcpy(&address[size_of_verts], norms, size_of_verts);
    memcpy(&address[size_of_verts * 2], indicies, size_of_indicies);

    uint32_t vertex_id = vrms_client_create_data_object(client, memory_id, 0, size_of_verts, size_of_vert, sizeof(float), VRMS_VERTEX);
    uint32_t normal_id = vrms_client_create_data_object(client, memory_id, size_of_verts, size_of_verts, size_of_vert, sizeof(float), VRMS_NORMAL);
    uint32_t index_id = vrms_client_create_data_object(client, memory_id, size_of_verts * 2, size_of_indicies, size_of_index, sizeof(uint16_t), VRMS_INDEX);

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);

    uint32_t mesh_id = vrms_client_create_mesh_color(client, geometry_id, r, g, b, a);

    free(verts);
    free(norms);
    free(indicies);

    return mesh_id;
}

uint32_t vrms_geometry_plane_textured(vrms_client_t* client, uint32_t x, uint32_t y, const char* filename) {
    uint32_t nr_verticies;
    uint32_t nr_indicies;
    uint32_t size_of_vert;
    uint32_t size_of_verts;
    uint32_t size_of_index;
    uint32_t size_of_indicies;
    uint32_t size_of_uv;
    uint32_t size_of_uvs;
    uint32_t size_total;
    uint32_t buff_off;

    int32_t memory_id;
    uint8_t* address = NULL;

    float* verts;
    float* norms;
    uint16_t* indicies;
    float* uvs;

    nr_verticies = 4;
    nr_indicies = 6;

    size_of_vert = sizeof(float) * 3;
    size_of_index = sizeof(uint16_t);
    size_of_uv = sizeof(float) * 2;

    size_of_verts = size_of_vert * nr_verticies;
    size_of_indicies = size_of_index * nr_indicies;
    size_of_uvs = size_of_uv * nr_verticies;

    verts = SAFEMALLOC(size_of_verts);
    norms = SAFEMALLOC(size_of_verts);
    indicies = SAFEMALLOC(size_of_indicies);
    uvs = SAFEMALLOC(size_of_uvs);

    std_plane_generate_verticies(verts, x, y);
    std_plane_generate_normals(norms);
    std_plane_generate_indicies(indicies);
    std_plane_generate_uvs(uvs);

    size_total = size_of_verts + size_of_verts + size_of_indicies + size_of_uvs;

    memory_id = vrms_client_create_memory(client, &address, size_total);
    if (0 == memory_id) {
        return 0;
    }

    buff_off = 0;
    memcpy(&address[buff_off], verts, size_of_verts);
    uint32_t vertex_id = vrms_client_create_data_object(client, memory_id, 0, size_of_verts, size_of_vert, sizeof(float), VRMS_VERTEX);

    buff_off += size_of_verts;
    memcpy(&address[buff_off], norms, size_of_verts);
    uint32_t normal_id = vrms_client_create_data_object(client, memory_id, buff_off, size_of_verts, size_of_vert, sizeof(float), VRMS_NORMAL);

    buff_off += size_of_verts;
    memcpy(&address[buff_off], indicies, size_of_indicies);
    uint32_t index_id = vrms_client_create_data_object(client, memory_id, buff_off, size_of_indicies, size_of_index, sizeof(uint16_t), VRMS_INDEX);

    buff_off += size_of_indicies;
    memcpy(&address[buff_off], uvs, size_of_uvs);
    uint32_t uv_id = vrms_client_create_data_object(client, memory_id, buff_off, size_of_uvs, size_of_uv, sizeof(float), VRMS_UV);

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);
    uint32_t texture_id = vrms_load_texture(client, filename, VRMS_TEXTURE_2D);
    if (0 == texture_id) {
        fprintf(stderr, "unable to create texture\n");
    }

    uint32_t mesh_id = vrms_client_create_mesh_texture(client, geometry_id, texture_id, uv_id);

    free(verts);
    free(norms);
    free(indicies);
    free(uvs);

    return mesh_id;
}

uint32_t vrms_geometry_load_matrix_data(vrms_client_t* client, uint32_t nr_matricies, float* matrix_data) {
    uint32_t size_of_matrix;
    uint32_t size_of_matricies;
    int32_t memory_id;
    uint8_t* address = NULL;

    size_of_matrix = sizeof(float) * 16;
    size_of_matricies = size_of_matrix * nr_matricies;
    memory_id = vrms_client_create_memory(client, &address, size_of_matricies);
    if (0 == memory_id) {
        return 0;
    }

    memcpy(address, matrix_data, size_of_matricies);

    uint32_t matrix_id = vrms_client_create_data_object(client, memory_id, 0, size_of_matricies, size_of_matrix, sizeof(float), VRMS_MATRIX);

    return matrix_id;
}

uint8_t vrms_geometry_render_buffer_basic(vrms_client_t* client, uint32_t object_id, float x, float y, float z) {
    uint32_t memory_id;
    uint32_t program_data_id;
    uint32_t register_data_id;
    uint32_t matrix_data_id;
    uint32_t program_id;
    uint32_t render_ret;
    uint32_t matrix_size;
    uint32_t prog_size;
    uint32_t reg_size;
    uint32_t total_size;
    uint8_t* shared_mem;
    float* model_matrix;
    uint8_t* program;
    uint32_t* registers;
    uint32_t prog_off;
    uint32_t reg_off;
    float vec3[3];

    matrix_size = sizeof(float) * 16;
    prog_size = sizeof(uint8_t) * 10;
    reg_size = sizeof(uint32_t) * 8;
    total_size = matrix_size + prog_size + reg_size;
    memory_id = vrms_client_create_memory(client, &shared_mem, total_size);

    if (!shared_mem) {
        fprintf(stderr, "Unable to initialize shared memory\n");
        return 0;
    }

    prog_off = matrix_size;
    reg_off = matrix_size + prog_size;

    model_matrix = (float*)shared_mem;
    program = ((uint8_t*)&shared_mem[prog_off]);
    registers = ((uint32_t*)&shared_mem[reg_off]);

    vec3[0] = x;
    vec3[1] = y;
    vec3[2] = z;

    mat4_identity(model_matrix);
    mat4_translate(model_matrix, (float*)&vec3);

    matrix_data_id = vrms_client_create_data_object(client, memory_id, 0, matrix_size, matrix_size, sizeof(float), VRMS_MATRIX);
    if (0 == matrix_data_id) {
        fprintf(stderr, "Unable to create matrix data\n");
        return 0;
    }

    registers[0] = object_id;
    registers[1] = matrix_data_id;
    registers[2] = 0; // index to matrix in memory
    registers[3] = 0;
    registers[4] = 0;
    registers[5] = 0;
    registers[6] = 0;
    registers[7] = 0;

    program[0] = VM_MATLM;
    program[1] = VM_REG0;
    program[2] = VM_REG1;
    program[3] = VM_REG2;

    program[4] = VM_DRAW;
    program[5] = VM_REG0;
    program[6] = VM_REG0;

    program[7] = VM_FRWAIT;

    program[8] = VM_JMP;
    program[9] = 0x04;

    program_data_id = vrms_client_create_data_object(client, memory_id, prog_off, prog_size, 1, 1, VRMS_PROGRAM);
    if (0 == program_data_id) {
        fprintf(stderr, "Unable to create program data\n");
        return 0;
    }

    program_id = vrms_client_create_program_object(client, program_data_id);
    if (0 == program_id) {
        fprintf(stderr, "Unable to create program\n");
        return 0;
    }

    register_data_id = vrms_client_create_data_object(client, memory_id, reg_off, reg_size, sizeof(uint32_t), sizeof(uint32_t), VRMS_REGISTER);
    if (0 == register_data_id) {
        fprintf(stderr, "Unable to create register data\n");
        return 0;
    }

    render_ret = vrms_client_run_program(client, program_id, register_data_id);
    if (0 == render_ret) {
        return 0;
    }
    return 1;
}
