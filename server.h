#ifndef VRMS_SERVER_H
#define VRMS_SERVER_H

#include "vroom.h"

#define NR_RENDER_AVG 10

typedef struct vrms_scene vrms_scene_t;

typedef enum vrms_queue_item_type {
    VRMS_QUEUE_DATA_LOAD,
    VRMS_QUEUE_TEXTURE_LOAD,
    VRMS_QUEUE_UPDATE_SYSTEM_MATRIX,
    VRMS_QUEUE_EVENT
} vrms_queue_item_type_t;

typedef struct vrms_queue_item_data_load {
    uint32_t scene_id;
    uint32_t object_id;
    uint8_t* buffer;
    uint32_t size;
    vrms_data_type_t type;
} vrms_queue_item_data_load_t;

typedef struct vrms_queue_item_texture_load {
    uint32_t scene_id;
    uint32_t object_id;
    uint8_t* buffer;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    vrms_texture_format_t format;
    vrms_texture_type_t type;
} vrms_queue_item_texture_load_t;

typedef struct vrms_queue_item_update_system_matrix {
    vrms_matrix_type_t matrix_type;
    vrms_update_type_t update_type;
    uint8_t* buffer;
} vrms_queue_item_update_system_matrix_t;

typedef struct vrms_queue_item_event {
    char* data;
} vrms_queue_item_event_t;

typedef struct vrms_queue_item {
    vrms_queue_item_type_t type;
    union {
        vrms_queue_item_data_load_t* data_load;
        vrms_queue_item_texture_load_t* texture_load;
        vrms_queue_item_update_system_matrix_t* update_system_matrix;
        vrms_queue_item_event_t* event;
    } item;
} vrms_queue_item_t;

typedef void (*system_matrix_callback_t)(vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix);

typedef struct vrms_skybox {
    uint32_t texture_id;
    float vertex_data[24];
    uint16_t index_data[36];
    uint32_t vertex_gl_id;
    uint32_t index_gl_id;
    uint32_t texture_gl_id;
    uint32_t shader_id;
    uint8_t realized;
} vrms_skybox_t;

typedef struct vrms_server {
    uint32_t next_scene_id;
    vrms_scene_t** scenes;
    vrms_queue_item_t inbound_queue[256];
    uint8_t inbound_queue_index;
    pthread_mutex_t inbound_queue_lock;
    uint32_t color_shader_id;
    uint32_t texture_shader_id;
    uint32_t cubemap_shader_id;
    float head_matrix[16];
    float body_matrix[16];
    system_matrix_callback_t system_matrix_update;
    uint32_t render_usecs[NR_RENDER_AVG];
    vrms_skybox_t skybox;
} vrms_server_t;

vrms_server_t* vrms_server_create();

vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id);

uint32_t vrms_server_create_scene(vrms_server_t* vrms_server, char* name);

uint32_t vrms_server_destroy_scene(vrms_server_t* server, uint32_t scene_id);

void vrms_server_draw_scenes(vrms_server_t* vrms_server, float projection_matrix[16], float view_matrix[16], float model_matrix[16], float skybox_projection_matrix[16]);

void vrms_queue_item_process(vrms_queue_item_t* queue_item);

void vrms_server_process_queue(vrms_server_t* server);

uint32_t vrms_server_queue_add_data_load(vrms_server_t* server, uint32_t size, uint32_t scene_id, uint32_t object_id, vrms_data_type_t type, uint8_t* buffer);

uint32_t vrms_server_queue_add_texture_load(vrms_server_t* server, uint32_t size, uint32_t scene_id, uint32_t object_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type, uint8_t* buffer);

void vrms_server_queue_update_system_matrix(vrms_server_t* server, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, uint8_t* buffer);

#endif
