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

#include "geometry2.h"
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

void vrms_geometry_layout_add_item(memory_layout_t* layout, uint32_t idx, vrms_data_type_t type, uint32_t size, uint32_t item_length, uint32_t data_length) {
    memory_layout_item_t* item = &layout->items[idx];
    item->memory_size = size;
    item->item_length = item_length;
    item->data_length = data_length;
    item->type = type;
}

void vrms_geometry_layout_add_vertex(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    vrms_geometry_layout_add_item(layout, idx, VRMS_VERTEX, (SIZEOF_VEC3 * count), SIZEOF_VEC3, SIZEOF_FL);
}

void vrms_geometry_layout_add_normal(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    vrms_geometry_layout_add_item(layout, idx, VRMS_VERTEX, (SIZEOF_VEC3 * count), SIZEOF_VEC3, SIZEOF_FL);
}

void vrms_geometry_layout_add_index(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    vrms_geometry_layout_add_item(layout, idx, VRMS_INDEX, (sizeof(uint16_t) * count), sizeof(uint16_t), sizeof(uint16_t));
}

void vrms_geometry_layout_add_color(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    vrms_geometry_layout_add_item(layout, idx, VRMS_COLOR, (SIZEOF_VEC4 * count), SIZEOF_VEC4, SIZEOF_FL);
}

void vrms_geometry_layout_add_uv(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    vrms_geometry_layout_add_item(layout, idx, VRMS_UV, (SIZEOF_VEC2 * count), SIZEOF_VEC2, SIZEOF_FL);
}

void vrms_geometry_layout_add_register(memory_layout_t* layout, uint32_t idx) {
    vrms_geometry_layout_add_item(layout, idx, VRMS_REGISTER, (sizeof(uint32_t) * 10), sizeof(uint32_t), sizeof(uint32_t));
}

void vrms_geometry_layout_add_program(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    vrms_geometry_layout_add_item(layout, idx, VRMS_PROGRAM, (sizeof(uint8_t) * count), sizeof(uint8_t), sizeof(uint8_t));
}

void vrms_geometry_layout_add_matrix(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    vrms_geometry_layout_add_item(layout, idx, VRMS_MATRIX, (SIZEOF_MAT4 * count), SIZEOF_MAT4, SIZEOF_FL);
}

uint8_t* vrms_geometry_get_uint8_pointer(memory_layout_t* layout, uint32_t idx) {
    memory_layout_item_t* item = &layout->items[idx];
    return (uint8_t*)item->mem;
}

uint16_t* vrms_geometry_get_uint16_pointer(memory_layout_t* layout, uint32_t idx) {
    memory_layout_item_t* item = &layout->items[idx];
    return (uint16_t*)item->mem;
}

uint32_t* vrms_geometry_get_uint32_pointer(memory_layout_t* layout, uint32_t idx) {
    memory_layout_item_t* item = &layout->items[idx];
    return (uint32_t*)item->mem;
}

float* vrms_geometry_get_float_pointer(memory_layout_t* layout, uint32_t idx) {
    memory_layout_item_t* item = &layout->items[idx];
    return (float*)item->mem;
}

uint32_t vrms_geometry_layout_get_id(memory_layout_t* layout, uint32_t idx) {
    memory_layout_item_t* item = &layout->items[idx];
    return item->id;
}

void vrms_geometry_layout_calculate_offsets(memory_layout_t* layout) {
    uint32_t offset = 0;
    uint32_t idx = 0;
    memory_layout_item_t* item;
    for (idx = 0; idx < layout->nr_items; idx++) {
        item = &layout->items[idx];
        item->memory_offset = offset;
        offset += item->memory_size;
    }
}

void vrms_geometry_layout_link_mem(memory_layout_t* layout) {
    uint32_t idx = 0;
    memory_layout_item_t* item;
    for (idx = 0; idx < layout->nr_items; idx++) {
        item = &layout->items[idx];
        item->mem = &layout->mem[item->memory_offset];
    }
}

void vrms_geometry_layout_init_memory(memory_layout_t* layout) {
    uint32_t index;
    memory_layout_item_t* item;

    for (index = 0; index < layout->nr_items; index++) {
        item = &layout->items[index];
        layout->total_size += item->memory_size;
    }

    layout->fd = memfd_create("WAYVROOM surface", MFD_ALLOW_SEALING);
    if (layout->fd <= 0) {
        fprintf(stderr, "unable to create shared memory: %d\n", errno);
        return;
    }

    int32_t ret = ftruncate(layout->fd, layout->total_size);
    if (-1 == ret) {
        fprintf(stderr, "unable to truncate memfd to size %zd\n", layout->total_size);
        return;
    }

    ret = fcntl(layout->fd, F_ADD_SEALS, F_SEAL_SHRINK);
    if (-1 == ret) {
        fprintf(stderr, "failed to add seals to memfd\n");
        return;
    }

    layout->mem = mmap(NULL, layout->total_size, PROT_READ|PROT_WRITE, MAP_SHARED, layout->fd, 0);
    if (MAP_FAILED == layout->mem) {
        fprintf(stderr, "unable to attach address\n");
        return;
    }
}

void vrms_geometry_layout_plane(memory_layout_t* layout) {
    vrms_geometry_layout_add_vertex(layout, LAYOUT_DEFAULT_VERTEX, 4);
    vrms_geometry_layout_add_normal(layout, LAYOUT_DEFAULT_NORMAL, 4);
    vrms_geometry_layout_add_index(layout, LAYOUT_DEFAULT_INDEX, 6);
}

void vrms_geometry_layout_plane_color(memory_layout_t* layout, float xmin, float ymin, float xmax, float ymax) {
    vrms_geometry_layout_plane(layout);
    vrms_geometry_layout_add_color(layout, LAYOUT_DEFAULT_COLOR, 4);
    vrms_geometry_layout_add_register(layout, LAYOUT_DEFAULT_REGISTER);
    vrms_geometry_layout_add_program(layout, LAYOUT_DEFAULT_PROGRAM, 1);
    vrms_geometry_layout_add_matrix(layout, LAYOUT_DEFAULT_MATRIX, 1);

    vrms_geometry_layout_calculate_offsets(layout);
    vrms_geometry_layout_init_memory(layout);
    layout->realizer(layout, layout->realizer_data);
    vrms_geometry_layout_link_mem(layout);

    float* verts = vrms_geometry_get_float_pointer(layout, LAYOUT_DEFAULT_VERTEX);
    float* norms = vrms_geometry_get_float_pointer(layout, LAYOUT_DEFAULT_NORMAL);
    uint16_t* indicies = vrms_geometry_get_uint16_pointer(layout, LAYOUT_DEFAULT_INDEX);
    float* colors = vrms_geometry_get_float_pointer(layout, LAYOUT_DEFAULT_COLOR);
    uint32_t* registers = vrms_geometry_get_uint32_pointer(layout, LAYOUT_DEFAULT_REGISTER);
    uint8_t* program = vrms_geometry_get_uint8_pointer(layout, LAYOUT_DEFAULT_PROGRAM);
    float* matrix = vrms_geometry_get_float_pointer(layout, LAYOUT_DEFAULT_MATRIX);

    std_plane_generate_verticies(verts, xmin, ymin, xmax, ymax);
    std_plane_generate_normals(norms);
    std_plane_generate_indicies(indicies);
    std_plane_generate_color(colors);
    layout->item_realizer(layout, &layout->items[LAYOUT_DEFAULT_VERTEX], layout->item_realizer_data);
    layout->item_realizer(layout, &layout->items[LAYOUT_DEFAULT_NORMAL], layout->item_realizer_data);
    layout->item_realizer(layout, &layout->items[LAYOUT_DEFAULT_INDEX], layout->item_realizer_data);
    layout->item_realizer(layout, &layout->items[LAYOUT_DEFAULT_COLOR], layout->item_realizer_data);
    
    registers[0] = vrms_geometry_layout_get_id(layout, LAYOUT_DEFAULT_VERTEX);
    registers[1] = vrms_geometry_layout_get_id(layout, LAYOUT_DEFAULT_NORMAL);
    registers[2] = vrms_geometry_layout_get_id(layout, LAYOUT_DEFAULT_INDEX);
    registers[3] = vrms_geometry_layout_get_id(layout, LAYOUT_DEFAULT_COLOR);

    mat4_identity(matrix);
    mat4_translatef(matrix, 0.0f, 0.0f, -10.0f);
    layout->item_realizer(layout, &layout->items[LAYOUT_DEFAULT_MATRIX], layout->item_realizer_data);
    registers[4] = vrms_geometry_layout_get_id(layout, LAYOUT_DEFAULT_MATRIX);
    registers[5] = 0;

    layout->item_realizer(layout, &layout->items[LAYOUT_DEFAULT_REGISTER], layout->item_realizer_data);

    program[0] = 0xc8;
    layout->item_realizer(layout, &layout->items[LAYOUT_DEFAULT_PROGRAM], layout->item_realizer_data);
}

void vrms_geometry_layout_item_realizer(memory_layout_t* layout, realize_layout_item_t callback, void* user_data) {
    layout->item_realizer = callback;
    layout->item_realizer_data = user_data;
}

void vrms_geometry_layout_realizer(memory_layout_t* layout, realize_layout_t callback, void* user_data) {
    layout->realizer = callback;
    layout->realizer_data = user_data;
}

memory_layout_t* vrms_geometry_layout_create(uint32_t nr_items) {
    memory_layout_t* layout;
    layout = malloc(sizeof(memory_layout_t));
    memset(layout, 0, sizeof(memory_layout_t));
    layout->items = malloc(sizeof(memory_layout_item_t) * nr_items);
    memset(layout->items, 0, sizeof(memory_layout_item_t) * nr_items);
    layout->nr_items = nr_items;
    return layout;
}
