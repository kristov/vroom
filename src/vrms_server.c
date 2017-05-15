#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include "vrms_server.h"

uint32_t vrms_create_scene(char* name) {
    return 1;
}

uint32_t vrms_create_data_object(uint32_t scene_id, vrms_data_type_t type, uint32_t shm_fd, uint32_t offset, uint32_t size_of, uint32_t stride) {
    void* address;
    float* buffer;

    buffer = malloc(size_of);
    fprintf(stderr, "size_of / stride = %d\n", size_of / stride);

    address = mmap(NULL, size_of, PROT_READ, MAP_SHARED, shm_fd, 0);
    memcpy(buffer, address, size_of);

    fprintf(stderr, "address: %d\n", (int)address);
    fprintf(stderr, "vrms_create_data_object(): type[%d], shm_id[%d], offset[%d], sizeof[%d], stride[%d]\n", type, shm_fd, offset, size_of, stride);
    return 1;
}

uint32_t vrms_create_geometry_object(uint32_t scene_id, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id) {
    return 1;
}

uint32_t vrms_create_color_mesh(uint32_t scene_id, uint32_t geometry_id, float r, float g, float b, float a) {
    return 1;
}

uint32_t vrms_create_texture_mesh(uint32_t scene_id, uint32_t geometry_id, uint32_t uv_id, uint32_t texture_id) {
    return 1;
}
