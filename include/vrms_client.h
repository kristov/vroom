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
uint32_t vrms_client_create_data_object(vrms_client_t* client, vrms_data_type_t type, int32_t shm_fd, uint32_t offset, uint32_t size_of, uint32_t stride);
uint32_t vrms_client_create_geometry_object(vrms_client_t* client, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id);
int32_t vrms_create_memory(size_t size, char* address);
