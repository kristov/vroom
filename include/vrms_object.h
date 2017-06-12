#ifdef RASPBERRYPI
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else /* not RASPBERRYPI */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif /* RASPBERRYPI */

#include "vrms.h"

typedef struct vrms_object_data {
    vrms_data_type_t type;
    uint32_t size;
    uint32_t nr_strides;
    uint32_t stride;
    GLuint gl_id;
    void* local_storage;
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
    } object;
} vrms_object_t;

vrms_object_t* vrms_object_create();
vrms_object_t* vrms_object_data_create(vrms_data_type_t type, uint32_t size, uint32_t nr_strides, uint32_t stride);
vrms_object_t* vrms_object_geometry_create(uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);
vrms_object_t* vrms_object_mesh_color_create(uint32_t geometry_id, float r, float g, float b, float a);
vrms_object_t* vrms_object_mesh_texture_create(uint32_t geometry_id, uint32_t uv_id, uint32_t texture_id);
void vrms_object_destroy(vrms_object_t* object);
