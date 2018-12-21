#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include "vroom.h"

#define SIZEOF_FLOAT sizeof(float)
#define SIZEOF_VEC2 (SIZEOF_FLOAT * 2)
#define SIZEOF_VEC3 (SIZEOF_FLOAT * 3)
#define SIZEOF_VEC4 (SIZEOF_FLOAT * 4)
#define SIZEOF_MAT2 (SIZEOF_FLOAT * 4)
#define SIZEOF_MAT3 (SIZEOF_FLOAT * 9)
#define SIZEOF_MAT4 (SIZEOF_FLOAT * 16)

#define LAYOUT_DEFAULT_VERTEX    0
#define LAYOUT_DEFAULT_NORMAL    1
#define LAYOUT_DEFAULT_INDEX     2
#define LAYOUT_DEFAULT_COLOR     3
#define LAYOUT_DEFAULT_REGISTER  4
#define LAYOUT_DEFAULT_PROGRAM   5
#define LAYOUT_DEFAULT_MATRIX    6

typedef struct memory_layout_item memory_layout_item_t;
typedef struct memory_layout memory_layout_t;

typedef void (*realize_layout_item_t)(memory_layout_t* layout, memory_layout_item_t* item, void* user_data);
typedef void (*realize_layout_t)(memory_layout_t* layout, void* user_data);

typedef struct memory_layout_item {
    uint32_t id;
    uint8_t* mem;
    uint32_t memory_offset;
    uint32_t memory_size;
    vrms_data_type_t type;
} memory_layout_item_t;

typedef struct memory_layout {
    uint32_t id;
    int32_t fd;
    uint8_t* mem;
    size_t total_size;
    uint32_t nr_items;
    memory_layout_item_t* items;
    realize_layout_t realizer;
    void* realizer_data;
    realize_layout_item_t item_realizer;
    void* item_realizer_data;
} memory_layout_t;

memory_layout_t* memory_layout_create(uint32_t nr_items);

void memory_layout_item_realizer(memory_layout_t* layout, realize_layout_item_t callback, void* user_data);

void memory_layout_realizer(memory_layout_t* layout, realize_layout_t callback, void* user_data);

uint8_t* memory_layout_get_uint8_pointer(memory_layout_t* layout, uint32_t idx);

uint16_t* memory_layout_get_uint16_pointer(memory_layout_t* layout, uint32_t idx);

uint32_t* memory_layout_get_uint32_pointer(memory_layout_t* layout, uint32_t idx);

float* memory_layout_get_float_pointer(memory_layout_t* layout, uint32_t idx);

uint32_t memory_layout_get_id(memory_layout_t* layout, uint32_t idx);

void memory_layout_add_uint8(memory_layout_t* layout, uint32_t idx, uint32_t count);

void memory_layout_add_uint16(memory_layout_t* layout, uint32_t idx, uint32_t count);

void memory_layout_add_uint32(memory_layout_t* layout, uint32_t idx, uint32_t count);

void memory_layout_add_float(memory_layout_t* layout, uint32_t idx, uint32_t count);

void memory_layout_add_vec2(memory_layout_t* layout, uint32_t idx, uint32_t count);

void memory_layout_add_vec3(memory_layout_t* layout, uint32_t idx, uint32_t count);

void memory_layout_add_vec4(memory_layout_t* layout, uint32_t idx, uint32_t count);

void memory_layout_add_mat2(memory_layout_t* layout, uint32_t idx, uint32_t count);

void memory_layout_add_mat3(memory_layout_t* layout, uint32_t idx, uint32_t count);

void memory_layout_add_mat4(memory_layout_t* layout, uint32_t idx, uint32_t count);

void memory_layout_realize(memory_layout_t* layout);

void memory_layout_item_realize(memory_layout_t* layout, uint32_t idx);

void memory_layout_add_item(memory_layout_t* layout, uint32_t idx, vrms_data_type_t type, uint32_t size);

#endif
