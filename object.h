#include "gl.h"
#include "vroom.h"

typedef struct vrms_object_memory {
    uint32_t fd;
    void* address;
    uint32_t size;
} vrms_object_memory_t;

typedef struct vrms_object_data {
    uint32_t memory_id;
    uint32_t memory_offset;
    uint32_t memory_length;
    uint32_t item_length;
    uint32_t data_length;
    vrms_data_type_t type;
    uint32_t gl_id;
    void* local_storage;
} vrms_object_data_t;

typedef struct vrms_object_texture {
    uint32_t memory_length;
    uint32_t width;
    uint32_t height;
    vrms_texture_format_t format;
    vrms_texture_type_t type;
    uint32_t gl_id;
} vrms_object_texture_t;

typedef struct vrms_object_skybox {
    uint32_t id;
    uint32_t texture_id;
    float* vertex_data;
    uint16_t* index_data;
    uint32_t vertex_gl_id;
    uint32_t index_gl_id;
    uint32_t texture_gl_id;
} vrms_object_skybox_t;

typedef struct vrms_object_matrix {
    uint32_t id;
    float* data;
} vrms_object_matrix_t;

typedef struct vrms_object {
    uint32_t id;
    vrms_object_type_t type;
    uint8_t realized;
    union {
        vrms_object_memory_t* object_memory;
        vrms_object_data_t* object_data;
        vrms_object_texture_t* object_texture;
        vrms_object_skybox_t* object_skybox;
    } object;
} vrms_object_t;

vrms_object_t* vrms_object_create();
vrms_object_t* vrms_object_memory_create(uint32_t fd, void* address, uint32_t size);
vrms_object_t* vrms_object_data_create(uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, uint32_t item_length, uint32_t data_length, vrms_data_type_t type);
vrms_object_t* vrms_object_texture_create(uint32_t memory_length, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);
vrms_object_t* vrms_object_skybox_create(uint32_t texture_id);
void vrms_object_memory_destroy(vrms_object_memory_t* memory);
void vrms_object_data_destroy(vrms_object_data_t* data);
void vrms_object_texture_destroy(vrms_object_texture_t* texture);
void vrms_object_matrix_destroy(vrms_object_matrix_t* matrix);
void vrms_object_skybox_destroy(vrms_object_skybox_t* skybox);
