#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syscall.h>
#include <sys/un.h>
#include <linux/memfd.h>
#include <unistd.h>
#include "memfd.h"
#include "safe_malloc.h"
#include "vrms_object.h"
#include "vrms_scene.h"
#include "esm.h"

vrms_scene_t* vrms_scene_create(char* name) {
    vrms_scene_t* scene = SAFEMALLOC(sizeof(vrms_scene_t));
    memset(scene, 0, sizeof(vrms_scene_t));

    scene->objects = SAFEMALLOC(sizeof(vrms_object_t) * 10);
    memset(scene->objects, 0, sizeof(vrms_object_t) * 10);
    scene->next_object_id = 1;

    scene->render_buffer_nr_objects = 0;
    scene->render_buffer_lock = SAFEMALLOC(sizeof(pthread_mutex_t));
    memset(scene->render_buffer_lock, 0, sizeof(pthread_mutex_t));

    return scene;
}

vrms_object_t* vrms_scene_get_object_by_id(vrms_scene_t* vrms_scene, uint32_t id) {
    vrms_object_t* vrms_object;
    if (vrms_scene->next_object_id <= id) {
        fprintf(stderr, "id out of range\n");
        return NULL;
    }
    vrms_object = vrms_scene->objects[id];
    if (NULL == vrms_object) {
        fprintf(stderr, "undefined object ofr id: %d\n", id);
        return NULL;
    }
    return vrms_object;
}

void vrms_scene_destroy_objects(vrms_scene_t* scene) {
    uint32_t idx;
    vrms_object_t* object;

    for (idx = 1; idx < scene->next_object_id; idx++) {
        object = vrms_scene_get_object_by_id(scene, idx);
        vrms_object_destroy(object);
    }

    free(scene->objects);
}

void vrms_scene_empty_outbound_queue(vrms_scene_t* scene) {
/*
    uint32_t idx;

    if (!pthread_mutex_lock(scene->outbound_queue_lock)) {
        for (idx = 0; idx < scene->outbound_queue_index; idx++) {
            queue_item = scene->outbound_queue[idx];
            free(queue_item);
        }
        pthread_mutex_unlock(scene->outbound_queue_lock);
    }
*/
}

void vrms_scene_add_object(vrms_scene_t* scene, vrms_object_t* object) {
    scene->objects[scene->next_object_id] = object;
    object->id = scene->next_object_id;
    scene->next_object_id++;
}

void vrms_scene_destroy(vrms_scene_t* scene) {

    if (!pthread_mutex_lock(scene->render_buffer_lock)) {
        free(scene->render_buffer);
        pthread_mutex_unlock(scene->render_buffer_lock);
        free(scene->render_buffer_lock);
    }

    vrms_scene_destroy_objects(scene);

/*
    vrms_scene_empty_outbound_queue(scene);
    free(scene->outbound_queue);
    free(scene->outbound_queue_lock);
*/

    free(scene);
}

uint32_t vrms_scene_create_object_data(vrms_scene_t* scene, vrms_data_type_t type, uint32_t fd, uint32_t offset, uint32_t size, uint32_t nr_strides, uint32_t stride) {
    void* address;
    void* buffer;
    int32_t seals;

    buffer = SAFEMALLOC(size);
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

    vrms_object_t* object = vrms_object_data_create(type, size, nr_strides, stride);
    vrms_scene_add_object(scene, object);

    if (VRMS_MATRIX == type) {
        fprintf(stderr, "allocating local space for matrix data\n");
        object->object.object_data->local_storage = buffer;
    }
    else {
        vrms_server_queue_add_data_load(scene->server, size, &object->object.object_data->gl_id, type, buffer);
    }

    return object->id;
}

uint32_t vrms_scene_create_object_geometry(vrms_scene_t* scene, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id) {
    vrms_object_t* object = vrms_object_geometry_create(vertex_id, normal_id, index_id);
    vrms_scene_add_object(scene, object);
    return object->id;
}

uint32_t vrms_scene_create_object_mesh_color(vrms_scene_t* scene, uint32_t geometry_id, float r, float g, float b, float a) {
    vrms_object_t* object = vrms_object_mesh_color_create(geometry_id, r, g, b, a);
    vrms_scene_add_object(scene, object);
    return object->id;
}

uint32_t vrms_scene_create_object_mesh_texture(vrms_scene_t* scene, uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id) {
    vrms_object_t* object = vrms_object_mesh_texture_create(geometry_id, texture_id, uv_id);
    vrms_scene_add_object(scene, object);
    return object->id;
}

uint32_t vrms_scene_set_render_buffer(vrms_scene_t* scene, uint32_t fd, uint32_t nr_objects) {
    void* address;
    int32_t seals;
    size_t size;

    size = (sizeof(uint32_t) * 3) * nr_objects;
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

    pthread_mutex_lock(scene->render_buffer_lock);
    if (NULL != scene->render_buffer) {
        free(scene->render_buffer);
    }
    scene->render_buffer = SAFEMALLOC(size);
    scene->render_buffer_nr_objects = nr_objects;
    memcpy(scene->render_buffer, (char*)address, size);
    pthread_mutex_unlock(scene->render_buffer_lock);

    return 1;
}
