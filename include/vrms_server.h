#include "vrms.h"

#define NR_RENDER_AVG 10

typedef struct vrms_scene vrms_scene_t;

typedef struct vrms_queue_item vrms_queue_item_t;

typedef void (*system_matrix_callback_t)(vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix);

typedef struct vrms_server {
    uint32_t next_scene_id;
    vrms_scene_t** scenes;
    vrms_queue_item_t** inbound_queue;
    uint32_t inbound_queue_index;
    pthread_mutex_t* inbound_queue_lock;
    uint32_t color_shader_id;
    uint32_t texture_shader_id;
    uint32_t cubemap_shader_id;
    float* head_matrix;
    float* body_matrix;
    system_matrix_callback_t system_matrix_update;
    uint32_t render_usecs[NR_RENDER_AVG];
} vrms_server_t;

vrms_server_t* vrms_server_create();
vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id);
uint32_t vrms_server_create_scene(vrms_server_t* vrms_server, char* name);
uint32_t vrms_server_destroy_scene(vrms_server_t* server, uint32_t scene_id);
void vrms_server_draw_scenes(vrms_server_t* vrms_server, float* projection_matrix, float* view_matrix, float* model_matrix, float* skybox_projection_matrix);
void vrms_queue_item_process(vrms_queue_item_t* queue_item);
void vrms_server_process_queue(vrms_server_t* server);
uint32_t vrms_server_queue_add_data_load(vrms_server_t* server, uint32_t size, uint32_t* gl_id_ref, vrms_data_type_t type, uint8_t* buffer);
uint32_t vrms_server_queue_add_texture_load(vrms_server_t* server, uint32_t size, uint32_t* gl_id_ref, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type, uint8_t* buffer);
void vrms_server_queue_update_system_matrix(vrms_server_t* server, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, uint8_t* buffer);
void vrms_server_draw_mesh_color(vrms_server_t* server, vrms_object_mesh_color_t* mesh, float* projection_matrix, float* view_matrix, float* model_matrix);
void vrms_server_draw_mesh_texture(vrms_server_t* server, vrms_object_mesh_texture_t* mesh, float* projection_matrix, float* view_matrix, float* model_matrix);
void vrms_server_draw_skybox(vrms_server_t* server, vrms_object_skybox_t* skybox, float* projection_matrix, float* view_matrix, float* model_matrix);
