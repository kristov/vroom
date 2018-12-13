#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "vroom.h"

#define SIZEOF_FL sizeof(float)
#define SIZEOF_VEC2 (SIZEOF_FL * 2)
#define SIZEOF_VEC3 (SIZEOF_FL * 3)
#define SIZEOF_VEC4 (SIZEOF_FL * 4)
#define SIZEOF_MAT4 (SIZEOF_FL * 16)

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
    uint32_t item_length;
    uint32_t data_length;
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

memory_layout_t* vrms_geometry_layout_create(uint32_t nr_items);

void vrms_geometry_layout_item_realizer(memory_layout_t* layout, realize_layout_item_t callback, void* user_data);

void vrms_geometry_layout_realizer(memory_layout_t* layout, realize_layout_t callback, void* user_data);

void vrms_geometry_layout_plane_color(memory_layout_t* layout, float xmin, float ymin, float xmax, float ymax);

uint32_t vrms_geometry_layout_get_id(memory_layout_t* layout, uint32_t idx);
#endif
