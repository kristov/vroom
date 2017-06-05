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
        vrms_object_matrix_t* object_matrix;
    } object;
} vrms_object_t;

typedef enum vrms_queue_item_type {
    VRMS_QUEUE_DATA_LOAD,
    VRMS_QUEUE_EVENT
} vrms_queue_item_type_t;

typedef struct vrms_queue_item_data_load {
    vrms_data_type_t type;
    GLuint* destination;
    void* buffer;
    uint32_t size;
} vrms_queue_item_data_load_t;

typedef struct vrms_queue_item_event {
    char* data;
} vrms_queue_item_event_t;

typedef struct vrms_queue_item {
    vrms_queue_item_type_t type;
    union {
        vrms_queue_item_data_load_t* data_load;
        vrms_queue_item_event_t* event;
    } item;
} vrms_queue_item_t;

typedef struct vrms_scene {
    char* name;
    uint32_t id;
    uint32_t next_object_id;
    vrms_object_t** objects;
    vrms_queue_item_t** inbound_queue;
    uint32_t inbound_queue_index;
    pthread_mutex_t* inbound_queue_lock;
    vrms_queue_item_t** outbound_queue;
    uint32_t outbound_queue_index;
    pthread_mutex_t* outbound_queue_lock;
    uint32_t render_buffer_nr_objects;
    uint32_t* render_buffer;
    pthread_mutex_t* render_buffer_lock;
} vrms_scene_t;

typedef struct vrms_server {
    uint32_t next_scene_id;
    vrms_scene_t** scenes;
} vrms_server_t;

vrms_server_t* vrms_server_create();
vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id);
uint32_t vrms_create_scene(vrms_server_t* vrms_server, char* name);
uint32_t vrms_create_data_object(vrms_scene_t* vrms_scene, vrms_data_type_t type, uint32_t fd, uint32_t offset, uint32_t size, uint32_t nr_values, uint32_t stride);
uint32_t vrms_create_geometry_object(vrms_scene_t* scene, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);
uint32_t vrms_create_mesh_color(vrms_scene_t* vrms_scene, uint32_t geometry_id, float r, float g, float b, float a);
uint32_t vrms_create_mesh_texture(vrms_scene_t* vrms_scene, uint32_t geometry_id, uint32_t uv_id, uint32_t texture_id);
uint32_t vrms_set_render_buffer(vrms_scene_t* vrms_scene, uint32_t fd, uint32_t nr_objects);
void vrms_server_draw_scenes(vrms_server_t* vrms_server, GLuint shader_id, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix);
void vrms_queue_item_process(vrms_queue_item_t* queue_item);
void vrms_server_process_queues(vrms_server_t* server);
