#ifndef VRMS_SCENE_H
#define VRMS_SCENE_H

#include "vroom.h"
#include "gl.h"
#include "rendervm.h"

typedef struct vrms_server vrms_server_t;
typedef struct vrms_object vrms_object_t;

typedef enum vrms_scene_queue_item_type {
    VRMS_SCENE_QUEUE_GL_LOAD
} vrms_scene_queue_item_type_t;

typedef struct vrms_scene_queue_item_gl_load {
    vrms_object_type_t type;
    uint32_t object_id;
    uint32_t gl_id;
} vrms_scene_queue_item_gl_load_t;

typedef struct vrms_scene_queue_item {
    vrms_scene_queue_item_type_t type;
    union {
        vrms_scene_queue_item_gl_load_t* gl_load;
    } item;
} vrms_scene_queue_item_t;

typedef struct vrms_scene {
    char* name;
    uint32_t id;
    vrms_server_t* server;
    uint32_t next_object_id;
    vrms_object_t** objects;
    vrms_scene_queue_item_t outbound_queue[256];
    uint8_t outbound_queue_index;
    pthread_mutex_t outbound_queue_lock;
    uint32_t render_buffer_size;
    uint8_t* render_buffer;
    rendervm_t* vm;
    pthread_mutex_t scene_lock;
    uint32_t render_allocation_usec;
    uint32_t skybox_texture_id;
    vrms_gl_render_t render;
    vrms_gl_matrix_t matrix;
} vrms_scene_t;

vrms_scene_t* vrms_scene_create(char* name);

void vrms_scene_destroy(vrms_scene_t* scene);

void vrms_scene_destroy_object(vrms_scene_t* scene, uint32_t object_id);

uint32_t vrms_scene_create_memory(vrms_scene_t* scene, uint32_t fd, uint32_t size);

uint32_t vrms_scene_create_object_data(vrms_scene_t* scene, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, vrms_data_type_t type);

uint32_t vrms_scene_create_object_texture(vrms_scene_t* scene, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);

uint32_t vrms_scene_create_program(vrms_scene_t* scene, uint32_t data_id);

uint32_t vrms_scene_run_program(vrms_scene_t* scene, uint32_t program_id, uint32_t register_id);
vrms_object_t* vrms_scene_get_object_by_id(vrms_scene_t* scene, uint32_t id);

uint32_t vrms_scene_update_system_matrix(vrms_scene_t* scene, uint32_t data_id, uint32_t data_index, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type);

uint32_t vrms_scene_set_skybox(vrms_scene_t* scene, uint32_t texture_id);
vrms_object_t* vrms_scene_get_mesh_by_id(vrms_scene_t* scene, uint32_t mesh_id);

uint32_t vrms_scene_draw(vrms_scene_t* scene, float* projection_matrix, float* view_matrix, float* model_matrix, float* skybox_projection_matrix);

uint32_t vrms_scene_queue_add_gl_loaded(vrms_scene_t* scene, vrms_object_type_t type, uint32_t object_id, uint32_t gl_id);

#endif
