#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syscall.h>
#include <sys/un.h>
#include <linux/memfd.h>
#include <unistd.h>
#include "memfd.h"
#include "vrms_server.h"

vrms_server_t* vrms_server_create() {
    vrms_server_t* vrms_server = malloc(sizeof(vrms_server_t));
    vrms_server->scenes = malloc(sizeof(vrms_scene_t) * 10);
    vrms_server->next_scene_id = 1;
    return vrms_server;
}

vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id) {
    return vrms_server->scenes[scene_id];
}

uint32_t vrms_create_scene(vrms_server_t* vrms_server, char* name) {
    vrms_scene_t* vrms_scene = malloc(sizeof(vrms_scene_t));
    vrms_scene->objects = malloc(sizeof(vrms_object_t) * 10);
    vrms_scene->next_object_id = 1;
    vrms_server->scenes[vrms_server->next_scene_id] = vrms_scene;
    vrms_scene->id = vrms_server->next_scene_id;
    vrms_server->next_scene_id++;
    return vrms_scene->id;
}

vrms_object_t* vrms_object_create(vrms_scene_t* vrms_scene) {
    vrms_object_t* vrms_object = malloc(sizeof(vrms_object_t));
    vrms_object->type = VRMS_OBJECT_INVALID;
    vrms_scene->objects[vrms_scene->next_object_id] = vrms_object;
    vrms_object->id = vrms_scene->next_object_id;
    vrms_scene->next_object_id++;
    return vrms_object;
}

uint32_t vrms_create_data_object(vrms_scene_t* vrms_scene, vrms_data_type_t type, uint32_t fd, uint32_t dtype, uint32_t offset, uint32_t size, uint32_t stride) {
    void* address;
    void* buffer;
    uint32_t* int_buffer;
    float* float_buffer;
    int32_t seals;
    uint32_t nr_values;
    vrms_object_t* vrms_object;

    buffer = malloc(size);
    seals = fcntl(fd, F_GET_SEALS);
    if (!(seals & F_SEAL_SHRINK)) {
        fprintf(stderr, "got non-sealed memfd\n");
        return 0;
    }

    address = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (MAP_FAILED == address) {
        fprintf(stderr, "memory map failed\n");
        return 0;
    }

    memcpy(buffer, &((char*)address)[offset], size);

    vrms_object = vrms_object_create(vrms_scene);
    vrms_object->type = VRMS_OBJECT_DATA;

    vrms_object_data_t* object_data = malloc(sizeof(vrms_object_data_t));
    object_data->type = type;
    vrms_object->object.object_data = object_data;

    if (VRMS_FLOAT == dtype) {
        nr_values = size / sizeof(float);
        float_buffer = (float*)buffer;
        int i;
        for (i = 0; i < nr_values; i++) {
            fprintf(stderr, "%0.2f ", float_buffer[i]);
        }
        fprintf(stderr, "\n");
    }
    else if (VRMS_INT == dtype) {
        nr_values = size / sizeof(uint32_t);
        int_buffer = (uint32_t*)buffer;
        int i;
        for (i = 0; i < nr_values; i++) {
            fprintf(stderr, "%d ", int_buffer[i]);
        }
        fprintf(stderr, "\n");
    }

    return vrms_object->id;
}

uint32_t vrms_create_geometry_object(vrms_scene_t* vrms_scene, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id) {
    vrms_object_t* vrms_object;
    fprintf(stderr, "vertex_id[%d], normal_id[%d], index_id[%d]\n", vertex_id, normal_id, index_id);
    vrms_object = vrms_object_create(vrms_scene);
    vrms_object->type = VRMS_OBJECT_GEOMETRY;

    vrms_object_geometry_t* object_geometry = malloc(sizeof(vrms_object_geometry_t));
    object_geometry->vertex_id = vertex_id;
    object_geometry->normal_id = normal_id;
    object_geometry->index_id = index_id;

    vrms_object->object.object_geometry = object_geometry;

    return vrms_object->id;
}

uint32_t vrms_create_color_mesh(uint32_t scene_id, uint32_t geometry_id, float r, float g, float b, float a) {
    return 1;
}

uint32_t vrms_create_texture_mesh(uint32_t scene_id, uint32_t geometry_id, uint32_t uv_id, uint32_t texture_id) {
    return 1;
}
