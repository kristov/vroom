#include <GL/gl.h>
#include "vrms.h"

typedef struct vrms_object_data {
    vrms_data_type_t type;
    uint32_t nr_values;
    GLuint gl_id;
    GLuint gl_offset;
    GLuint gl_size;
} vrms_object_data_t;

typedef struct vrms_object_geometry {
    uint32_t id;
    uint32_t vertex_id;
    uint32_t normal_id;
    uint32_t index_id;
} vrms_object_geometry_t;

typedef struct vrms_object_mesh_texture {
    uint32_t id;
    uint32_t geometry_id;
    uint32_t uv_id;
    uint32_t texture_id;
} vrms_object_mesh_texture_t;

typedef struct vrms_object_mesh_color {
    uint32_t id;
    uint32_t geometry_id;
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
} vrms_object_mesh_color_t;

typedef struct vrms_object_matrix {
    uint32_t id;
    GLfloat* data;
} vrms_object_matrix_t;

typedef struct vrms_object {
    uint32_t id;
    vrms_object_type_t type;
    union {
        vrms_object_data_t* object_data;
        vrms_object_geometry_t* object_geometry;
        vrms_object_mesh_color_t* object_mesh_color;
        vrms_object_mesh_texture_t* object_mesh_texture;
        vrms_object_matrix_t* object_matrix;
    } object;
} vrms_object_t;

typedef struct vrms_scene {
    char* name;
    uint32_t id;
    uint32_t next_object_id;
    vrms_object_t** objects;
} vrms_scene_t;

typedef struct vrms_server {
    uint32_t next_scene_id;
    vrms_scene_t** scenes;
} vrms_server_t;

vrms_server_t* vrms_server_create();
vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id);
uint32_t vrms_create_scene(vrms_server_t* vrms_server, char* name);
uint32_t vrms_create_data_object(vrms_scene_t* scene, vrms_data_type_t type, uint32_t shm_fd, uint32_t offset, uint32_t size_of, uint32_t size, uint32_t stride);
uint32_t vrms_create_geometry_object(vrms_scene_t* scene, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);
uint32_t vrms_create_mesh_color(vrms_scene_t* vrms_scene, uint32_t geometry_id, float r, float g, float b, float a);
uint32_t vrms_create_mesh_texture(vrms_scene_t* vrms_scene, uint32_t geometry_id, uint32_t uv_id, uint32_t texture_id);
void vrms_server_draw_scene(vrms_server_t* vrms_server, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix);
//void vrms_server_draw_mesh_color(vrms_scene_t* vrms_scene, vrms_object_mesh_color_t* vrms_object_mesh_color, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix);
//void vrms_server_draw_mesh_texture(vrms_scene_t* vrms_scene, vrms_object_mesh_texture_t* vrms_object_mesh_texture, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix);
