#include <GL/gl.h>
#include "vrms.h"

/*
typedef struct vrms_server {
    array scenes;
} vrms_server_t;

typedef struct vrms_scene {
    char* name;
    uint32_t next_id;
    array dataobjects;
    array geometryobjects;
    array colormeshes;
    array texturemeshes;
    array matricies;
} vrms_scene_t;

typedef struct vrms_data_uv {
    uint32_t id;
    GLuint gl_id;
    GLuint gl_offset;
    GLuint gl_size;
} vrms_data_uv_t;

typedef struct vrms_data_color {
    uint32_t id;
    GLuint gl_id;
    GLuint gl_offset;
    GLuint gl_size;
} vrms_data_color_t;

typedef struct vrms_data_texture {
    uint32_t id;
    GLuint gl_id;
    GLuint gl_offset;
    GLuint gl_size;
} vrms_data_texture_t;

typedef struct vrms_data_vertex {
    uint32_t id;
    GLuint gl_id;
    GLuint gl_offset;
    GLuint gl_size;
} vrms_data_vertex_t;

typedef struct vrms_data_normal {
    uint32_t id;
    GLuint gl_id;
    GLuint gl_offset;
    GLuint gl_size;
} vrms_data_normal_t;

typedef struct vrms_data_index {
    uint32_t id;
    GLuint gl_id;
    GLuint gl_offset;
    GLuint gl_size;
} vrms_data_index_t;

typedef struct vrms_geometry {
    uint32_t id;
    uint32_t vertex_id;
    uint32_t normal_id;
    uint32_t index_id;
} vrms_data_geometry_t;

typedef struct vrms_colormesh {
    uint32_t id;
    uint32_t geometry_id;
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
} vrms_data_colormesh_t;

typedef struct vrms_texturemesh {
    uint32_t id;
    uint32_t geometry_id;
    uint32_t uv_id;
    uint32_t texture_id;
} vrms_data_texturemesh_t;

typedef struct vrms_matrix {
    uint32_t id;
    GLfloat* data;
} vrms_data_matrix_t;

*/

uint32_t vrms_create_scene(char* name);
uint32_t vrms_create_data_object(uint32_t scene_id, vrms_data_type_t type, uint32_t shm_fd, uint32_t offset, uint32_t size_of, uint32_t stride);
uint32_t vrms_create_geometry_object(uint32_t scene_id, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);
uint32_t vrms_create_color_mesh(uint32_t scene_id, uint32_t geometry_id, float r, float g, float b, float a);
uint32_t vrms_create_texture_mesh(uint32_t scene_id, uint32_t geometry_id, uint32_t uv_id, uint32_t texture_id);
