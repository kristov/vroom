#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <syscall.h>
#include <sys/un.h>
#include <linux/memfd.h>
#include "memfd.h"
#include <string.h>

#include "geometry.h"
#include "gl-matrix.h"

void geometry_plane_generate_verticies(float* verts, float x_min, float y_min, float x_max, float y_max) {
    uint32_t off = 0;

    verts[off + 0] = x_min; // 0
    verts[off + 1] = y_min;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x_max; // 1
    verts[off + 1] = y_min;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x_min; // 2
    verts[off + 1] = y_max;
    verts[off + 2] = 0.0f;
    off += 3;
    verts[off + 0] = x_max; // 3
    verts[off + 1] = y_max;
    verts[off + 2] = 0.0f;
}

void geometry_plane_generate_normals(float* norms) {
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

void geometry_plane_generate_indicies(uint16_t* indicies) {
    uint32_t off = 0;

    indicies[off + 0] = 0;
    indicies[off + 1] = 1;
    indicies[off + 2] = 2;
    indicies[off + 3] = 1;
    indicies[off + 4] = 2;
    indicies[off + 5] = 3;
    off += 6;
}

void geometry_any_generate_color(float* colors, uint32_t nr_colors, float r, float g, float b, float a) {
    uint32_t nr_floats = nr_colors * 4;
    uint32_t off = 0;
    for (off = 0; off < nr_floats; off += 4) {
        colors[off + 0] = r;
        colors[off + 1] = g;
        colors[off + 2] = b;
        colors[off + 3] = a;
    }
}

void geometry_plane_generate_uvs(float* uvs) {
    uint32_t off = 0;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 1.0f;
    off += 2;

    uvs[off + 0] = 0.0f;
    uvs[off + 1] = 0.0f;
    off += 2;

    uvs[off + 0] = 1.0f;
    uvs[off + 1] = 0.0f;
    off += 2;
}

void geometry_cube_generate_verticies(float* verts, uint32_t x, uint32_t y, uint32_t z) {
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

void geometry_cube_generate_normals(float* norms) {
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

void geometry_cube_generate_indicies(uint16_t* indicies) {
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

void geometry_cube_generate_uvs(float* uvs) {
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

void geometry_plane_color(memory_layout_t* layout, float xmin, float ymin, float xmax, float ymax, float r, float g, float b, float a) {
    memory_layout_add_vec3(layout, LAYOUT_DEFAULT_VERTEX, 4);
    memory_layout_add_vec3(layout, LAYOUT_DEFAULT_NORMAL, 4);
    memory_layout_add_uint16(layout, LAYOUT_DEFAULT_INDEX, 6);
    memory_layout_add_vec4(layout, LAYOUT_DEFAULT_COLOR, 4);
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

    geometry_plane_generate_verticies(verts, xmin, ymin, xmax, ymax);
    geometry_plane_generate_normals(norms);
    geometry_plane_generate_indicies(indicies);
    geometry_any_generate_color(colors, 4, r, g, b, a);

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

void geometry_cube_color(memory_layout_t* layout, float x, float y, float z, float r, float g, float b, float a) {
    memory_layout_add_vec3(layout, LAYOUT_DEFAULT_VERTEX, 24);
    memory_layout_add_vec3(layout, LAYOUT_DEFAULT_NORMAL, 24);
    memory_layout_add_uint16(layout, LAYOUT_DEFAULT_INDEX, 36);
    memory_layout_add_vec4(layout, LAYOUT_DEFAULT_COLOR, 24);
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
