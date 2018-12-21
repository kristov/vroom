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

#include "memory_layout.h"

void memory_layout_add_item(memory_layout_t* layout, uint32_t idx, vrms_data_type_t type, uint32_t size) {
    memory_layout_item_t* item = &layout->items[idx];
    item->memory_size = size;
    item->type = type;
}

void memory_layout_add_uint8(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    memory_layout_add_item(layout, idx, VRMS_UINT8, (sizeof(uint8_t) * count));
}

void memory_layout_add_uint16(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    memory_layout_add_item(layout, idx, VRMS_UINT16, (sizeof(uint16_t) * count));
}

void memory_layout_add_uint32(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    memory_layout_add_item(layout, idx, VRMS_UINT32, (sizeof(uint32_t) * count));
}

void memory_layout_add_float(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    memory_layout_add_item(layout, idx, VRMS_FLOAT, (SIZEOF_FLOAT * count));
}

void memory_layout_add_vec2(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    memory_layout_add_item(layout, idx, VRMS_VEC2, (SIZEOF_VEC2 * count));
}

void memory_layout_add_vec3(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    memory_layout_add_item(layout, idx, VRMS_VEC3, (SIZEOF_VEC3 * count));
}

void memory_layout_add_vec4(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    memory_layout_add_item(layout, idx, VRMS_VEC4, (SIZEOF_VEC4 * count));
}

void memory_layout_add_mat2(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    memory_layout_add_item(layout, idx, VRMS_MAT2, (SIZEOF_MAT2 * count));
}

void memory_layout_add_mat3(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    memory_layout_add_item(layout, idx, VRMS_MAT3, (SIZEOF_MAT3 * count));
}

void memory_layout_add_mat4(memory_layout_t* layout, uint32_t idx, uint32_t count) {
    memory_layout_add_item(layout, idx, VRMS_MAT4, (SIZEOF_MAT4 * count));
}

uint8_t* memory_layout_get_uint8_pointer(memory_layout_t* layout, uint32_t idx) {
    memory_layout_item_t* item = &layout->items[idx];
    return (uint8_t*)item->mem;
}

uint16_t* memory_layout_get_uint16_pointer(memory_layout_t* layout, uint32_t idx) {
    memory_layout_item_t* item = &layout->items[idx];
    return (uint16_t*)item->mem;
}

uint32_t* memory_layout_get_uint32_pointer(memory_layout_t* layout, uint32_t idx) {
    memory_layout_item_t* item = &layout->items[idx];
    return (uint32_t*)item->mem;
}

float* memory_layout_get_float_pointer(memory_layout_t* layout, uint32_t idx) {
    memory_layout_item_t* item = &layout->items[idx];
    return (float*)item->mem;
}

uint32_t memory_layout_get_id(memory_layout_t* layout, uint32_t idx) {
    memory_layout_item_t* item = &layout->items[idx];
    return item->id;
}

void memory_layout_calculate_offsets(memory_layout_t* layout) {
    uint32_t offset = 0;
    uint32_t idx = 0;
    memory_layout_item_t* item;
    for (idx = 0; idx < layout->nr_items; idx++) {
        item = &layout->items[idx];
        item->memory_offset = offset;
        offset += item->memory_size;
    }
}

void memory_layout_link_mem(memory_layout_t* layout) {
    uint32_t idx = 0;
    memory_layout_item_t* item;
    for (idx = 0; idx < layout->nr_items; idx++) {
        item = &layout->items[idx];
        item->mem = &layout->mem[item->memory_offset];
    }
}

void memory_layout_init_memory(memory_layout_t* layout) {
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

void memory_layout_item_realize(memory_layout_t* layout, uint32_t idx) {
    layout->item_realizer(layout, &layout->items[idx], layout->item_realizer_data);
}

void memory_layout_realize(memory_layout_t* layout) {
    memory_layout_calculate_offsets(layout);
    memory_layout_init_memory(layout);
    layout->realizer(layout, layout->realizer_data);
    memory_layout_link_mem(layout);
}

void memory_layout_item_realizer(memory_layout_t* layout, realize_layout_item_t callback, void* user_data) {
    layout->item_realizer = callback;
    layout->item_realizer_data = user_data;
}

void memory_layout_realizer(memory_layout_t* layout, realize_layout_t callback, void* user_data) {
    layout->realizer = callback;
    layout->realizer_data = user_data;
}

memory_layout_t* memory_layout_create(uint32_t nr_items) {
    memory_layout_t* layout;
    layout = malloc(sizeof(memory_layout_t));
    memset(layout, 0, sizeof(memory_layout_t));
    layout->items = malloc(sizeof(memory_layout_item_t) * nr_items);
    memset(layout->items, 0, sizeof(memory_layout_item_t) * nr_items);
    layout->nr_items = nr_items;
    return layout;
}
