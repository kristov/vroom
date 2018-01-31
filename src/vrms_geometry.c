#include <stdio.h>
#include <string.h>
#include "safe_malloc.h"
#include "vrms_client.h"
#include "vrms_geometry.h"

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

void std_cube_generate_indicies(unsigned short* indicies) {
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
    uvs[off + 0] = 0.0000f;
    uvs[off + 1] = 0.6666f;
    uvs[off + 2] = 0.3333f;
    uvs[off + 3] = 0.6666f;
    uvs[off + 4] = 0.0000f;
    uvs[off + 5] = 1.0000f;
    off += 6;
    uvs[off + 0] = 0.3333f;
    uvs[off + 1] = 0.6666f;
    uvs[off + 2] = 0.0000f;
    uvs[off + 3] = 1.0000f;
    uvs[off + 4] = 0.3333f;
    uvs[off + 5] = 1.0000f;
    off += 6;

    // left
    uvs[off + 0] = 0.3333f;
    uvs[off + 1] = 0.6666f;
    uvs[off + 2] = 0.0000f;
    uvs[off + 3] = 1.0000f;
    uvs[off + 4] = 0.0000f;
    uvs[off + 5] = 0.6666f;
    off += 6;
    uvs[off + 0] = 0.3333f;
    uvs[off + 1] = 0.6666f;
    uvs[off + 2] = 0.0000f;
    uvs[off + 3] = 1.0000f;
    uvs[off + 4] = 0.3333f;
    uvs[off + 5] = 1.0000f;
    off += 6;

    // right
    uvs[off + 0] = 0.0000f;
    uvs[off + 1] = 1.0000f;
    uvs[off + 2] = 0.3333f;
    uvs[off + 3] = 0.6666f;
    uvs[off + 4] = 0.3333f;
    uvs[off + 5] = 1.0000f;
    off += 6;
    uvs[off + 0] = 0.0000f;
    uvs[off + 1] = 1.0000f;
    uvs[off + 2] = 0.3333f;
    uvs[off + 3] = 0.6666f;
    uvs[off + 4] = 0.0000f;
    uvs[off + 5] = 0.6666f;
    off += 6;

    // top
    uvs[off + 0] = 0.0000f;
    uvs[off + 1] = 1.0000f;
    uvs[off + 2] = 0.3333f;
    uvs[off + 3] = 0.6666f;
    uvs[off + 4] = 0.3333f;
    uvs[off + 5] = 1.0000f;
    off += 6;
    uvs[off + 0] = 0.0000f;
    uvs[off + 1] = 1.0000f;
    uvs[off + 2] = 0.3333f;
    uvs[off + 3] = 0.6666f;
    uvs[off + 4] = 0.0000f;
    uvs[off + 5] = 0.6666f;
    off += 6;

    // bottom
    uvs[off + 0] = 0.0000f;
    uvs[off + 1] = 0.6666f;
    uvs[off + 2] = 0.3333f;
    uvs[off + 3] = 1.0000f;
    uvs[off + 4] = 0.0000f;
    uvs[off + 5] = 1.0000f;
    off += 6;
    uvs[off + 0] = 0.0000f;
    uvs[off + 1] = 0.6666f;
    uvs[off + 2] = 0.3333f;
    uvs[off + 3] = 1.0000f;
    uvs[off + 4] = 0.3333f;
    uvs[off + 5] = 0.6666f;
    off += 6;

    // back
    uvs[off + 0] = 0.0000f;
    uvs[off + 1] = 0.6666f;
    uvs[off + 2] = 0.3333f;
    uvs[off + 3] = 1.0000f;
    uvs[off + 4] = 0.0000f;
    uvs[off + 5] = 1.0000f;
    off += 6;
    uvs[off + 0] = 0.0000f;
    uvs[off + 1] = 0.6666f;
    uvs[off + 2] = 0.3333f;
    uvs[off + 3] = 1.0000f;
    uvs[off + 4] = 0.3333f;
    uvs[off + 5] = 0.6666f;
    off += 6;
}

uint32_t vrms_geometry_cube(vrms_client_t* client, uint32_t x, uint32_t y, uint32_t z, float r, float g, float b, float a) {
    uint32_t nr_verticies, nr_indicies, nr_vert_floats;
    size_t size_of_verts, size_of_indicies;

    int32_t shm_fd;
    void* address = NULL;
    char* buffer = NULL;

    float* verts;
    float* norms;
    unsigned short* indicies;

    nr_verticies = 4 * 6;
    nr_vert_floats = 3 * nr_verticies;

    verts = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    norms = SAFEMALLOC(sizeof(float) * nr_vert_floats);

    nr_indicies = 6 * 6;
    indicies = SAFEMALLOC(sizeof(unsigned short) * nr_indicies);

    std_cube_generate_verticies(verts, x, y, z);
    std_cube_generate_normals(norms);
    std_cube_generate_indicies(indicies);

    size_of_verts = sizeof(float) * nr_vert_floats;
    size_of_indicies = sizeof(unsigned short) * nr_indicies;

    shm_fd = vrms_create_memory((size_of_verts * 2) + size_of_indicies, &address);
    if (-1 == shm_fd) {
        return 0;
    }

    //msync(result, size, MS_SYNC);
    //munmap(result, size);

    buffer = (char*)address;

    memcpy(buffer, verts, size_of_verts);
    memcpy(&buffer[size_of_verts], norms, size_of_verts);
    memcpy(&buffer[size_of_verts * 2], indicies, size_of_indicies);

    uint32_t vertex_id = vrms_client_create_data_object(client, VRMS_VERTEX, shm_fd, 0, size_of_verts, nr_verticies, 3);
    uint32_t normal_id = vrms_client_create_data_object(client, VRMS_NORMAL, shm_fd, size_of_verts, size_of_verts, nr_verticies, 3);
    uint32_t index_id = vrms_client_create_data_object(client, VRMS_INDEX, shm_fd, size_of_verts * 2, size_of_indicies, nr_indicies, 1);

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);

    uint32_t mesh_id = vrms_client_create_mesh_color(client, geometry_id, r, g, b, a);

    return mesh_id;
}

uint32_t vrms_load_texture(vrms_client_t* client, const char* filename) {
    uint8_t header[54];
    uint8_t data_pos;
    uint32_t width, height;
    uint32_t image_size;
    int32_t shm_fd;
    void* address = NULL;
    uint8_t* buffer = NULL;

    FILE* file = fopen(filename, "rb");
    if (NULL == file) {
        printf("Image could not be opened\n");
        return 0;
    }

    if (fread(header, 1, 54, file) != 54) {
        printf("Not a correct BMP file\n");
        return 0;
    }

    if ( header[0] != 'B' || header[1] != 'M') {
        printf("Not a correct BMP file\n");
        return 0;
    }

    width  = *(int*)&(header[0x12]);
    height = *(int*)&(header[0x16]);
    image_size = width * height * 3;
    data_pos = 54;

    shm_fd = vrms_create_memory(image_size, &address);
    if (-1 == shm_fd) {
        fclose(file);
        return 0;
    }

    buffer = (uint8_t*)address;
    fread(buffer, data_pos, image_size, file);
    fclose(file);

/*
    int y = 0;
    int x = 0;
    uint8_t i = 0;
    for (y = 0; y < width; y++) {
        for (x = 0; x < height; x++) {
            printf("%x %x %x\n", ((uint8_t*)address)[i], ((uint8_t*)address)[i+1], ((uint8_t*)address)[i+2]);
            i += 3;
        }
    }
*/

    uint32_t texture_id = vrms_client_create_texture_object(client, shm_fd, 0, image_size, width, height, VRMS_RGBA_8);

    return texture_id;
}

uint32_t vrms_geometry_cube_textured(vrms_client_t* client, uint32_t x, uint32_t y, uint32_t z, const char* filename) {
    uint32_t nr_verticies, nr_indicies, nr_vert_floats, nr_uv_floats;
    size_t size_of_verts, size_of_norms, size_of_indicies, size_of_uvs, size_total;
    uint32_t buff_off;

    int32_t shm_fd;
    void* address = NULL;
    unsigned char* buffer = NULL;

    float* verts;
    float* norms;
    unsigned short* indicies;
    float* uvs;

    nr_verticies = 4 * 6;
    nr_vert_floats = 3 * nr_verticies;
    nr_indicies = 6 * 6;
    nr_uv_floats = nr_indicies * 2;

    verts = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    norms = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    indicies = SAFEMALLOC(sizeof(unsigned short) * nr_indicies);
    uvs = SAFEMALLOC(sizeof(float) * nr_uv_floats);

    std_cube_generate_verticies(verts, x, y, z);
    std_cube_generate_normals(norms);
    std_cube_generate_indicies(indicies);
    std_cube_generate_uvs(uvs);

    size_of_verts = sizeof(float) * nr_vert_floats;
    size_of_norms = size_of_verts;
    size_of_indicies = sizeof(unsigned short) * nr_indicies;
    size_of_uvs = sizeof(float) * nr_uv_floats;
    size_total = size_of_verts + size_of_norms + size_of_indicies + size_of_uvs;

    shm_fd = vrms_create_memory(size_total, &address);
    if (-1 == shm_fd) {
        return 0;
    }

    buffer = (unsigned char*)address;

    buff_off = 0;
    memcpy(&buffer[buff_off], verts, size_of_verts);
    uint32_t vertex_id = vrms_client_create_data_object(client, VRMS_VERTEX, shm_fd, buff_off, size_of_verts, nr_verticies, 3);

    buff_off += size_of_verts;
    memcpy(&buffer[buff_off], norms, size_of_verts);
    uint32_t normal_id = vrms_client_create_data_object(client, VRMS_NORMAL, shm_fd, buff_off, size_of_verts, nr_verticies, 3);

    buff_off += size_of_norms;
    memcpy(&buffer[buff_off], indicies, size_of_indicies);
    uint32_t index_id = vrms_client_create_data_object(client, VRMS_INDEX, shm_fd, buff_off, size_of_indicies, nr_indicies, 1);

    buff_off += size_of_indicies;
    memcpy(&buffer[buff_off], uvs, size_of_uvs);
    uint32_t uv_id = vrms_client_create_data_object(client, VRMS_UV, shm_fd, buff_off, size_of_uvs, nr_indicies, 2);

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);
    uint32_t texture_id = vrms_load_texture(client, filename);
    uint32_t mesh_id = vrms_client_create_mesh_texture(client, geometry_id, texture_id, uv_id);

    return mesh_id;

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
}

void std_plane_generate_indicies(unsigned short* indicies) {
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
    uvs[off + 2] = 1.0f;
    uvs[off + 3] = 0.0f;
    uvs[off + 4] = 0.0f;
    uvs[off + 5] = 1.0f;
    off += 6;
    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    uvs[off + 2] = 0.0f;
    uvs[off + 3] = 1.0f;
    uvs[off + 4] = 1.0f;
    uvs[off + 5] = 1.0f;
    off += 6;
}

uint32_t vrms_geometry_plane(vrms_client_t* client, uint32_t x, uint32_t y, float r, float g, float b, float a) {
    uint32_t nr_verticies, nr_indicies, nr_vert_floats;
    size_t size_of_verts, size_of_indicies;

    int32_t shm_fd;
    void* address = NULL;
    char* buffer = NULL;

    float* verts;
    float* norms;
    unsigned short* indicies;

    nr_verticies = 4;
    nr_vert_floats = 3 * nr_verticies;

    verts = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    norms = SAFEMALLOC(sizeof(float) * nr_vert_floats);

    nr_indicies = 6;
    indicies = SAFEMALLOC(sizeof(unsigned short) * nr_indicies);

    std_plane_generate_verticies(verts, x, y);
    std_plane_generate_normals(norms);
    std_plane_generate_indicies(indicies);

    size_of_verts = sizeof(float) * nr_vert_floats;
    size_of_indicies = sizeof(unsigned short) * nr_indicies;

    shm_fd = vrms_create_memory((size_of_verts * 2) + size_of_indicies, &address);
    if (-1 == shm_fd) {
        return 0;
    }

    buffer = (char*)address;
    memcpy(buffer, verts, size_of_verts);
    memcpy(&buffer[size_of_verts], norms, size_of_verts);
    memcpy(&buffer[size_of_verts * 2], indicies, size_of_indicies);

    uint32_t vertex_id = vrms_client_create_data_object(client, VRMS_VERTEX, shm_fd, 0, size_of_verts, nr_verticies, 3);
    uint32_t normal_id = vrms_client_create_data_object(client, VRMS_NORMAL, shm_fd, size_of_verts, size_of_verts, nr_verticies, 3);
    uint32_t index_id = vrms_client_create_data_object(client, VRMS_INDEX, shm_fd, size_of_verts * 2, size_of_indicies, nr_indicies, 1);

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);

    uint32_t mesh_id = vrms_client_create_mesh_color(client, geometry_id, r, g, b, a);

    return mesh_id;
}

uint32_t vrms_geometry_plane_textured(vrms_client_t* client, uint32_t x, uint32_t y, const char* filename) {
    uint32_t nr_verticies, nr_indicies, nr_vert_floats, nr_uv_floats;
    size_t size_of_verts, size_of_norms, size_of_indicies, size_of_uvs, size_total;
    uint32_t buff_off;

    int32_t shm_fd;
    void* address = NULL;
    unsigned char* buffer = NULL;

    float* verts;
    float* norms;
    unsigned short* indicies;
    float* uvs;

    nr_verticies = 4;
    nr_vert_floats = 3 * nr_verticies;
    nr_indicies = 6;
    nr_uv_floats = nr_indicies * 2;

    verts = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    norms = SAFEMALLOC(sizeof(float) * nr_vert_floats);
    indicies = SAFEMALLOC(sizeof(unsigned short) * nr_indicies);
    uvs = SAFEMALLOC(sizeof(float) * nr_uv_floats);

    std_plane_generate_verticies(verts, x, y);
    std_plane_generate_normals(norms);
    std_plane_generate_indicies(indicies);
    std_plane_generate_uvs(uvs);

    size_of_verts = sizeof(float) * nr_vert_floats;
    size_of_norms = size_of_verts;
    size_of_indicies = sizeof(unsigned short) * nr_indicies;
    size_of_uvs = sizeof(float) * nr_uv_floats;
    size_total = size_of_verts + size_of_norms + size_of_indicies + size_of_uvs;

    shm_fd = vrms_create_memory(size_total, &address);
    if (-1 == shm_fd) {
        return 0;
    }

    buffer = (unsigned char*)address;

    buff_off = 0;
    memcpy(&buffer[buff_off], verts, size_of_verts);
    uint32_t vertex_id = vrms_client_create_data_object(client, VRMS_VERTEX, shm_fd, 0, size_of_verts, nr_verticies, 3);

    buff_off += size_of_verts;
    memcpy(&buffer[buff_off], norms, size_of_verts);
    uint32_t normal_id = vrms_client_create_data_object(client, VRMS_NORMAL, shm_fd, buff_off, size_of_verts, nr_verticies, 3);

    buff_off += size_of_verts;
    memcpy(&buffer[buff_off], indicies, size_of_indicies);
    uint32_t index_id = vrms_client_create_data_object(client, VRMS_INDEX, shm_fd, buff_off, size_of_indicies, nr_indicies, 1);

    buff_off += size_of_indicies;
    memcpy(&buffer[buff_off], uvs, size_of_uvs);
    uint32_t uv_id = vrms_client_create_data_object(client, VRMS_UV, shm_fd, buff_off, size_of_uvs, nr_indicies, 2);

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);
    uint32_t texture_id = vrms_load_texture(client, filename);
    uint32_t mesh_id = vrms_client_create_mesh_texture(client, geometry_id, texture_id, uv_id);

    return mesh_id;
}

uint32_t vrms_geometry_load_matrix_data(vrms_client_t* client, uint32_t nr_matricies, float* matrix_data) {
    size_t size_of_matricies;

    int32_t shm_fd;
    void* address = NULL;
    char* buffer = NULL;

    size_of_matricies = (sizeof(float) * 16) * nr_matricies;
    shm_fd = vrms_create_memory(size_of_matricies, &address);
    if (-1 == shm_fd) {
        return 0;
    }

    buffer = (char*)address;
    memcpy(buffer, matrix_data, size_of_matricies);

    uint32_t matrix_id = vrms_client_create_data_object(client, VRMS_MATRIX, shm_fd, 0, size_of_matricies, nr_matricies, 16);

    return matrix_id;
}

uint32_t vrms_geometry_render_buffer_set(vrms_client_t* client, uint32_t nr_items, uint32_t* render_buffer) {
    size_t size_of_buffer;

    int32_t shm_fd;
    void* address = NULL;
    char* buffer = NULL;

    size_of_buffer = (sizeof(uint32_t) * 3) * nr_items;
    shm_fd = vrms_create_memory(size_of_buffer, &address);
    if (-1 == shm_fd) {
        return 0;
    }

    buffer = (char*)address;
    memcpy(buffer, render_buffer, size_of_buffer);

    uint32_t render_ret = vrms_client_render_buffer_set(client, shm_fd, nr_items);

    return render_ret;
}
