#ifdef RASPBERRYPI
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else /* not RASPBERRYPI */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif /* RASPBERRYPI */
#include "vrms.h"

typedef struct vrms_object_memory {
    uint32_t fd;
    void* address;
    uint32_t size;
} vrms_object_memory_t;

typedef struct vrms_object_data {
    vrms_data_type_t type;
    uint32_t memory_length;
    uint32_t value_length;
    GLuint gl_id;
    void* local_storage;
} vrms_object_data_t;

typedef struct vrms_object_texture {
    uint32_t memory_length;
    uint32_t width;
    uint32_t height;
    vrms_texture_format_t format;
    vrms_texture_type_t type;
    GLuint gl_id;
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
    GLuint vertex_gl_id;
    GLuint normal_gl_id;
    GLuint index_gl_id;
    GLuint texture_gl_id;
    GLuint uv_gl_id;
    uint32_t nr_indicies;
} vrms_object_mesh_texture_t;

typedef struct vrms_object_mesh_color {
    uint32_t id;
    uint32_t geometry_id;
    GLuint vertex_gl_id;
    GLuint normal_gl_id;
    GLuint index_gl_id;
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
    uint32_t nr_indicies;
} vrms_object_mesh_color_t;

typedef struct vrms_object_skybox {
    uint32_t id;
    uint32_t texture_id;
    uint32_t size;
    float* vertex_data;
    uint16_t* index_data;
    GLuint vertex_gl_id;
    GLuint index_gl_id;
    GLuint texture_gl_id;
} vrms_object_skybox_t;

typedef struct vrms_object_matrix {
    uint32_t id;
    GLfloat* data;
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
        vrms_object_skybox_t* object_skybox;
    } object;
} vrms_object_t;

vrms_object_t* vrms_object_create();
vrms_object_t* vrms_object_memory_create(uint32_t fd, void* address, uint32_t size);
vrms_object_t* vrms_object_data_create(vrms_data_type_t type, uint32_t memory_length, uint32_t value_length);
vrms_object_t* vrms_object_texture_create(uint32_t memory_length, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);
vrms_object_t* vrms_object_geometry_create(uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);
vrms_object_t* vrms_object_mesh_color_create(uint32_t geometry_id, float r, float g, float b, float a);
vrms_object_t* vrms_object_mesh_texture_create(uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id);
vrms_object_t* vrms_object_skybox_create(uint32_t texture_id, uint32_t size);
void vrms_object_memory_destroy(vrms_object_memory_t* memory);
void vrms_object_data_destroy(vrms_object_data_t* data);
void vrms_object_geometry_destroy(vrms_object_geometry_t* geometry);
void vrms_object_mesh_color_destroy(vrms_object_mesh_color_t* mesh_color);
void vrms_object_mesh_texture_destroy(vrms_object_mesh_texture_t* mesh_texture);
void vrms_object_texture_destroy(vrms_object_texture_t* texture);
void vrms_object_matrix_destroy(vrms_object_matrix_t* matrix);
void vrms_object_skybox_destroy(vrms_object_skybox_t* skybox);
