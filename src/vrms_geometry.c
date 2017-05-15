#include <stdio.h>
#include <string.h>
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
    norms[off + 2] = -1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 21
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 22
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;
    norms[off + 0] = 0.0f; // 23
    norms[off + 1] = 0.0f;
    norms[off + 2] = -1.0f;
    off += 3;
}

/*
void std_cube_generate_colors(float* colors, float r, float g, float b, float a) {
    uint32_t off = 0;
    // front
    colors[off + 0] = r; // 0
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 1
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 2
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 3
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;

    // left
    colors[off + 0] = r; // 4
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 5
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 6
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 7
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;

    // right
    colors[off + 0] = r; // 8
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 9
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 10
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 11
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;

    // top
    colors[off + 0] = r; // 12
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 13
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 14
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 15
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;

    // bottom
    colors[off + 0] = r; // 16
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 17
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 18
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 19
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;

    // back
    colors[off + 0] = r; // 20
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 21
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 22
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
    colors[off + 0] = r; // 23
    colors[off + 1] = g;
    colors[off + 2] = b;
    colors[off + 3] = a;
    off += 4;
}
*/

void std_cube_generate_indicies(uint32_t* indicies) {
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

uint32_t vrms_geometry_cube(vrms_client_t* client, uint32_t x, uint32_t y, uint32_t z, float r, float g, float b, float a) {
    uint32_t nr_verticies, nr_indicies, nr_vert_floats;
    //uint32_t nr_color_floats;
    size_t size_of_verts, size_of_indicies;

    int32_t shm_fd;
    void* address = NULL;
    char* buffer = NULL;

    float* verts;
    float* norms;
    uint32_t* indicies;
    //float* colors;

    nr_verticies = 4 * 6;
    nr_vert_floats = 3 * nr_verticies;

    verts = malloc(sizeof(float) * nr_vert_floats);
    norms = malloc(sizeof(float) * nr_vert_floats);

    nr_indicies = 6 * 6;
    indicies = malloc(sizeof(uint32_t) * nr_indicies);

    //nr_color_floats = 4 * nr_verticies;
    //colors = malloc(sizeof(float) * nr_color_floats);

    std_cube_generate_verticies(verts, x, y, z);
    std_cube_generate_normals(norms);
    //std_cube_generate_colors(colors, r, g, b, a);
    std_cube_generate_indicies(indicies);

    size_of_verts = sizeof(float) * nr_vert_floats;
    size_of_indicies = sizeof(uint32_t) * nr_indicies;

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

    uint32_t vertex_id = vrms_client_create_data_object(client, VRMS_VERTEX, shm_fd, 0, size_of_verts, 3);
    uint32_t normal_id = vrms_client_create_data_object(client, VRMS_NORMAL, shm_fd, size_of_verts, size_of_verts, 3);
    uint32_t index_id = vrms_client_create_data_object(client, VRMS_INDEX, shm_fd, size_of_verts * 2, size_of_indicies, 1);

    uint32_t geometry_id = vrms_client_create_geometry_object(client, vertex_id, normal_id, index_id);

    return geometry_id;
}
