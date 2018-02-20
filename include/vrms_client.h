#include <stdint.h>
#include <stdlib.h>
#include "vrms.h"

typedef struct vrms_client {
    int32_t socket;
    uint32_t scene_id;
} vrms_client_t;

vrms_client_t* vrms_connect();
uint32_t vrms_create_scene(vrms_client_t* client, char* name);
uint32_t vrms_destroy_scene(vrms_client_t* client);
uint32_t vrms_client_create_memory(vrms_client_t* client, uint8_t** address, size_t size);
uint32_t vrms_client_create_data_object(vrms_client_t* client, vrms_data_type_t type, int32_t fd, uint32_t offset, uint32_t size, uint32_t nr_strides, uint32_t stride);
uint32_t vrms_client_create_texture_object(vrms_client_t* client, int32_t memory_id, uint32_t offset, uint32_t size, uint32_t width, uint32_t height, vrms_texture_format_t format);
uint32_t vrms_client_create_geometry_object(vrms_client_t* client, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);
uint32_t vrms_client_create_mesh_color(vrms_client_t* client, uint32_t geometry_id, float r, float g, float b, float a);
uint32_t vrms_client_create_mesh_texture(vrms_client_t* client, uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id);
uint32_t vrms_client_render_buffer_set(vrms_client_t* client, int32_t memory_id, uint32_t nr_items);
uint32_t vrms_client_update_system_matrix(vrms_client_t* client, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, int32_t memory_id, uint32_t offset, uint32_t size);
