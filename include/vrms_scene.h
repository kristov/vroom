#include "vrms.h"

typedef struct vrms_queue_item vrms_queue_item_t;
typedef struct vrms_server vrms_server_t;
typedef struct vrms_object vrms_object_t;

typedef struct vrms_scene {
    char* name;
    uint32_t id;
    vrms_server_t* server;
    uint32_t next_object_id;
    vrms_object_t** objects;
    vrms_queue_item_t** outbound_queue;
    uint32_t outbound_queue_index;
    pthread_mutex_t* outbound_queue_lock;
    uint32_t render_buffer_nr_objects;
    uint32_t* render_buffer;
    pthread_mutex_t* render_buffer_lock;
    GLuint onecolor_shader_id;
    GLuint texture_shader_id;
    GLuint cubemap_shader_id;
} vrms_scene_t;

vrms_scene_t* vrms_scene_create(char* name);
void vrms_scene_destroy(vrms_scene_t* scene);
uint32_t vrms_scene_create_memory(vrms_scene_t* scene, uint32_t fd, uint32_t size);
uint32_t vrms_scene_create_object_data(vrms_scene_t* scene, vrms_data_type_t type, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, uint32_t value_length);
uint32_t vrms_scene_create_object_texture(vrms_scene_t* scene, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);
uint32_t vrms_scene_create_object_geometry(vrms_scene_t* scene, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);
uint32_t vrms_scene_create_object_mesh_color(vrms_scene_t* scene, uint32_t geometry_id, float r, float g, float b, float a);
uint32_t vrms_scene_create_object_mesh_texture(vrms_scene_t* scene, uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id);
uint32_t vrms_scene_set_render_buffer(vrms_scene_t* scene, uint32_t memory_id, uint32_t nr_objects);

vrms_object_t* vrms_scene_get_object_by_id(vrms_scene_t* scene, uint32_t id);
uint32_t vrms_scene_update_system_matrix(vrms_scene_t* scene, uint32_t memory_id, uint32_t offset, uint32_t size, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type);
vrms_object_t* vrms_scene_get_mesh_by_id(vrms_scene_t* scene, uint32_t mesh_id);
