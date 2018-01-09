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
} vrms_scene_t;

vrms_scene_t* vrms_scene_create(char* name);
void vrms_scene_destroy(vrms_scene_t* scene);
uint32_t vrms_scene_create_object_data(vrms_scene_t* scene, vrms_data_type_t type, uint32_t fd, uint32_t offset, uint32_t size, uint32_t nr_values, uint32_t stride);
uint32_t vrms_scene_create_object_geometry(vrms_scene_t* scene, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);
uint32_t vrms_scene_create_object_mesh_color(vrms_scene_t* scene, uint32_t geometry_id, float r, float g, float b, float a);
uint32_t vrms_scene_create_object_mesh_texture(vrms_scene_t* scene, uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id);
uint32_t vrms_scene_set_render_buffer(vrms_scene_t* scene, uint32_t fd, uint32_t nr_objects);

void vrms_server_queue_add_data_load(vrms_server_t* server, uint32_t size, GLuint* gl_id_ref, vrms_data_type_t type, void* buffer);
vrms_object_t* vrms_scene_get_object_by_id(vrms_scene_t* vrms_scene, uint32_t id);
