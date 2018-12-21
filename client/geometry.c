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

void std_plane_generate_verticies(float* verts, float x_min, float y_min, float x_max, float y_max) {
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

void std_plane_generate_color(float* colors) {
    uint32_t off = 0;

    colors[off + 0] = 1.0f;
    colors[off + 1] = 0.0f;
    colors[off + 2] = 0.0f;
    colors[off + 3] = 1.0f;
    off += 4;

    colors[off + 0] = 1.0f;
    colors[off + 1] = 0.0f;
    colors[off + 2] = 0.0f;
    colors[off + 3] = 1.0f;
    off += 4;

    colors[off + 0] = 1.0f;
    colors[off + 1] = 0.0f;
    colors[off + 2] = 0.0f;
    colors[off + 3] = 1.0f;
    off += 4;

    colors[off + 0] = 1.0f;
    colors[off + 1] = 0.0f;
    colors[off + 2] = 0.0f;
    colors[off + 3] = 1.0f;
    off += 4;
}

void std_plane_generate_uvs(float* uvs) {
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

void geometry_plane_color(memory_layout_t* layout, float xmin, float ymin, float xmax, float ymax) {
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

    std_plane_generate_verticies(verts, xmin, ymin, xmax, ymax);
    std_plane_generate_normals(norms);
    std_plane_generate_indicies(indicies);
    std_plane_generate_color(colors);

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
