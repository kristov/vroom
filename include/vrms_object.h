#include "vrms_gl.h"
#include "vrms.h"

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

typedef struct vrms_object_geometry {
    uint32_t id;
    uint32_t vertex_id;
    uint32_t normal_id;
    uint32_t index_id;
} vrms_object_geometry_t;

typedef struct vrms_object_mesh_texture {
    uint32_t id;
    uint32_t geometry_id;
    uint32_t texture_id;
    uint32_t uv_id;
    uint32_t vertex_gl_id;
    uint32_t normal_gl_id;
    uint32_t index_gl_id;
    uint32_t texture_gl_id;
    uint32_t uv_gl_id;
    uint32_t nr_indicies;
} vrms_object_mesh_texture_t;

typedef struct vrms_object_mesh_color {
    uint32_t id;
    uint32_t geometry_id;
    uint32_t vertex_gl_id;
    uint32_t normal_gl_id;
    uint32_t index_gl_id;
    float r;
    float g;
    float b;
    float a;
    uint32_t nr_indicies;
} vrms_object_mesh_color_t;

typedef struct vrms_object_program {
    uint32_t id;
    uint32_t length;
    uint8_t* data;
} vrms_object_program_t;

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
        vrms_object_geometry_t* object_geometry;
        vrms_object_mesh_color_t* object_mesh_color;
        vrms_object_mesh_texture_t* object_mesh_texture;
        vrms_object_program_t* object_program;
        vrms_object_skybox_t* object_skybox;
    } object;
} vrms_object_t;

vrms_object_t* vrms_object_create();
vrms_object_t* vrms_object_memory_create(uint32_t fd, void* address, uint32_t size);
vrms_object_t* vrms_object_data_create(uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, uint32_t item_length, uint32_t data_length, vrms_data_type_t type);
vrms_object_t* vrms_object_texture_create(uint32_t memory_length, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);
vrms_object_t* vrms_object_geometry_create(uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);
vrms_object_t* vrms_object_mesh_color_create(uint32_t geometry_id, float r, float g, float b, float a);
vrms_object_t* vrms_object_mesh_texture_create(uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id);
vrms_object_t* vrms_object_program_create(uint32_t program_length);
vrms_object_t* vrms_object_skybox_create(uint32_t texture_id);
void vrms_object_memory_destroy(vrms_object_memory_t* memory);
void vrms_object_data_destroy(vrms_object_data_t* data);
void vrms_object_geometry_destroy(vrms_object_geometry_t* geometry);
void vrms_object_mesh_color_destroy(vrms_object_mesh_color_t* mesh_color);
void vrms_object_mesh_texture_destroy(vrms_object_mesh_texture_t* mesh_texture);
void vrms_object_program_destroy(vrms_object_program_t* program);
void vrms_object_texture_destroy(vrms_object_texture_t* texture);
void vrms_object_matrix_destroy(vrms_object_matrix_t* matrix);
void vrms_object_skybox_destroy(vrms_object_skybox_t* skybox);
