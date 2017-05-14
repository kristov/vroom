#include <stdint.h>
#include "vrms_server.h"

int32_t vrms_create_scene(char* name) {
    return 1;
}

int32_t vrms_create_data_object(int32_t scene_id, vrms_data_type_t type, int32_t shm_fd, int32_t offset, int32_t size_of, int32_t stride) {
    return 1;
}

int32_t vrms_create_geometry_object(int32_t scene_id, int32_t vertex_id, int32_t normal_id, int32_t index_id) {
    return 1;
}

int32_t vrms_create_color_mesh(int32_t scene_id, int32_t geometry_id, float r, float g, float b, float a) {
    return 1;
}

int32_t vrms_create_texture_mesh(int32_t scene_id, int32_t geometry_id, int32_t uv_id, int32_t texture_id) {
    return 1;
}
