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
#include "vrms.h"
#include "vrms_object.h"
#include "vrms_scene.h"
#include "vrms_server.h"
#include "esm.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

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

vrms_object_t* vrms_scene_get_object_by_id(vrms_scene_t* scene, uint32_t id) {
    vrms_object_t* vrms_object;
    if (scene->next_object_id <= id) {
        debug_print("id out of range\n");
        return NULL;
    }
    vrms_object = scene->objects[id];
    if (NULL == vrms_object) {
        return NULL;
    }
    return vrms_object;
}

vrms_object_memory_t* vrms_scene_get_memory_object_by_id(vrms_scene_t* scene, uint32_t memory_id) {
    vrms_object_t* object;

    if (0 == memory_id) {
        debug_print("vrms_scene_get_memory_object_by_id: no memory id passed\n");
        return NULL;
    }

    object = vrms_scene_get_object_by_id(scene, memory_id);
    if (NULL == object) {
        return NULL;
    }

    if (VRMS_OBJECT_MEMORY != object->type) {
        debug_print("vrms_scene_get_memory_object_by_id: asked for an id that is not a memory object\n");
        return NULL;
    }

    return object->object.object_memory;
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

uint32_t vrms_scene_create_memory(vrms_scene_t* scene, uint32_t fd, uint32_t size) {
    void* address;
    int32_t seals;

    seals = fcntl(fd, F_GET_SEALS);
    if (!(seals & F_SEAL_SHRINK)) {
        debug_print("got non-sealed memfd\n");
        return 0;
    }

    address = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (MAP_FAILED == address) {
        debug_print("memory map failed\n");
        return 0;
    }

    vrms_object_t* object = vrms_object_memory_create(address, size);
    vrms_scene_add_object(scene, object);

    return object->id;
}

uint32_t vrms_scene_create_object_data(vrms_scene_t* scene, vrms_data_type_t type, uint32_t memory_id, uint32_t offset, uint32_t size, uint32_t nr_strides, uint32_t stride) {
    void* buffer;
    vrms_object_memory_t* memory;

    memory = vrms_scene_get_memory_object_by_id(scene, memory_id);
    if (NULL == memory) {
        return 0;
    }

    if ((offset + size) > memory->size) {
        debug_print("create_data_object: read beyond memory size!\n");
    }

    vrms_object_t* object = vrms_object_data_create(type, size, nr_strides, stride);
    vrms_scene_add_object(scene, object);

    if (VRMS_MATRIX == type) {
        buffer = SAFEMALLOC(size);
        memcpy(buffer, &((unsigned char*)memory->address)[offset], size);
        object->object.object_data->local_storage = buffer;
    }
    else {
        buffer = &((unsigned char*)memory->address)[offset];
        vrms_server_queue_add_data_load(scene->server, size, &object->object.object_data->gl_id, type, buffer);
    }

    return object->id;
}

uint32_t vrms_scene_create_object_texture(vrms_scene_t* scene, uint32_t memory_id, uint32_t offset, uint32_t size, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type) {
    void* buffer;
    vrms_object_memory_t* memory;

    memory = vrms_scene_get_memory_object_by_id(scene, memory_id);
    if (NULL == memory) {
        debug_print("unable to find memory object\n");
        return 0;
    }

    if ((offset + size) > memory->size) {
        debug_print("create_data_object: read beyond memory size!\n");
    }

    buffer = &((unsigned char*)memory->address)[offset];

    vrms_object_t* object = vrms_object_texture_create(size, width, height, format, type);
    vrms_scene_add_object(scene, object);

    vrms_server_queue_add_texture_load(scene->server, size, &object->object.object_texture->gl_id, width, height, format, type, buffer);

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

uint32_t vrms_scene_update_system_matrix(vrms_scene_t* scene, uint32_t memory_id, uint32_t offset, uint32_t size, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type) {
    void* buffer;
    vrms_object_memory_t* memory;

    memory = vrms_scene_get_memory_object_by_id(scene, memory_id);
    if (NULL == memory) {
        return 0;
    }

    if ((offset + size) > memory->size) {
        debug_print("vrms_scene_update_system_matrix: read beyond memory size!\n");
    }

    buffer = &((unsigned char*)memory->address)[offset];
    vrms_server_queue_update_system_matrix(scene->server, matrix_type, update_type, buffer);

    return 1;
}

uint32_t vrms_scene_set_render_buffer(vrms_scene_t* scene, uint32_t memory_id, uint32_t nr_objects) {
    size_t size;
    vrms_object_memory_t* memory;

    size = (sizeof(uint32_t) * 3) * nr_objects;

    memory = vrms_scene_get_memory_object_by_id(scene, memory_id);
    if (NULL == memory) {
        return 0;
    }

    if (size > memory->size) {
        debug_print("vrms_scene_set_render_buffer: read beyond memory size!\n");
    }

    pthread_mutex_lock(scene->render_buffer_lock);
    if (NULL != scene->render_buffer) {
        free(scene->render_buffer);
    }
    scene->render_buffer = SAFEMALLOC(size);
    scene->render_buffer_nr_objects = nr_objects;
    memcpy(scene->render_buffer, (unsigned char*)memory->address, size);
    pthread_mutex_unlock(scene->render_buffer_lock);

    return 1;
}

uint8_t vrms_scene_mesh_color_realize(vrms_scene_t* scene, vrms_object_mesh_color_t* mesh) {
    vrms_object_t* object;
    vrms_object_geometry_t* geometry;
    vrms_object_data_t* vertex;
    vrms_object_data_t* normal;
    vrms_object_data_t* index;

    if ((0 != mesh->vertex_gl_id) && (0 != mesh->normal_gl_id) && (0 != mesh->index_gl_id)) {
        return 1;
    }

    object = vrms_scene_get_object_by_id(scene, mesh->geometry_id);
    if (NULL == object) {
        return 0;
    }
    geometry = object->object.object_geometry;

    object = vrms_scene_get_object_by_id(scene, geometry->vertex_id);
    if (NULL == object) {
        return 0;
    }
    vertex = object->object.object_data;
    if (0 != vertex->gl_id) {
        mesh->vertex_gl_id = vertex->gl_id;
    }

    object = vrms_scene_get_object_by_id(scene, geometry->normal_id);
    if (NULL == object) {
        return 0;
    }
    normal = object->object.object_data;
    if (0 != normal->gl_id) {
        mesh->normal_gl_id = normal->gl_id;
    }

    object = vrms_scene_get_object_by_id(scene, geometry->index_id);
    if (NULL == object) {
        return 0;
    }
    index = object->object.object_data;
    if (0 != index->gl_id) {
        mesh->index_gl_id = index->gl_id;
        mesh->nr_indicies = index->nr_strides;
    }

    if ((0 != mesh->vertex_gl_id) && (0 != mesh->normal_gl_id) && (0 != mesh->index_gl_id)) {
        return 1;
    }

    return 0;
}

uint8_t vrms_scene_mesh_texture_realize(vrms_scene_t* scene, vrms_object_mesh_texture_t* mesh) {
    vrms_object_t* object;
    vrms_object_geometry_t* geometry;
    vrms_object_data_t* vertex;
    vrms_object_data_t* normal;
    vrms_object_data_t* index;
    vrms_object_data_t* uv;
    vrms_object_texture_t* texture;
    vrms_texture_type_t texture_type;

    if ((0 != mesh->vertex_gl_id) && (0 != mesh->normal_gl_id) && (0 != mesh->index_gl_id) && (0 != mesh->uv_gl_id) && (0 != mesh->texture_gl_id)) {
        return 1;
    }

    object = vrms_scene_get_object_by_id(scene, mesh->geometry_id);
    if (NULL == object) {
        debug_print("no geometry object\n");
        return 0;
    }
    geometry = object->object.object_geometry;

    object = vrms_scene_get_object_by_id(scene, geometry->vertex_id);
    if (NULL == object) {
        debug_print("no vertex object\n");
        return 0;
    }
    vertex = object->object.object_data;
    if (0 != vertex->gl_id) {
        mesh->vertex_gl_id = vertex->gl_id;
    }

    object = vrms_scene_get_object_by_id(scene, geometry->normal_id);
    if (NULL == object) {
        debug_print("no normal object\n");
        return 0;
    }
    normal = object->object.object_data;
    if (0 != normal->gl_id) {
        mesh->normal_gl_id = normal->gl_id;
    }

    object = vrms_scene_get_object_by_id(scene, mesh->texture_id);
    if (NULL == object) {
        return 0;
    }
    texture = object->object.object_texture;
    if (0 != texture->gl_id) {
        mesh->texture_gl_id = texture->gl_id;
        texture_type = texture->type;
    }

    object = vrms_scene_get_object_by_id(scene, geometry->index_id);
    if (NULL == object) {
        debug_print("no index object\n");
        return 0;
    }
    index = object->object.object_data;
    if (0 != index->gl_id) {
        mesh->index_gl_id = index->gl_id;
        mesh->nr_indicies = index->nr_strides;
    }

    switch (texture_type) {
        case VRMS_TEXTURE_2D:
            debug_print("realizing texture mesh\n");
            object = vrms_scene_get_object_by_id(scene, mesh->uv_id);
            if (NULL == object) {
                return 0;
            }
            uv = object->object.object_data;
            if (0 != uv->gl_id) {
                mesh->uv_gl_id = uv->gl_id;
            }
            if ((0 != mesh->vertex_gl_id) && (0 != mesh->normal_gl_id) && (0 != mesh->index_gl_id) && (0 != mesh->uv_gl_id) && (0 != mesh->texture_gl_id)) {
                return 1;
            }
            break;
        case VRMS_TEXTURE_CUBE_MAP:
            debug_print("realizing cubemap mesh\n");
            mesh->uv_gl_id = 0;
            if ((0 != mesh->vertex_gl_id) && (0 != mesh->normal_gl_id) && (0 != mesh->index_gl_id) && (0 != mesh->texture_gl_id)) {
                return 1;
            }
            break;
        default:
            break;
    }

    return 0;
}

vrms_object_t* vrms_scene_get_mesh_by_id(vrms_scene_t* scene, uint32_t mesh_id) {
    vrms_object_t* object;
    uint8_t realized;

    if (0 == mesh_id) {
        debug_print("vrms_scene_get_mesh_by_id: no mesh id passed\n");
        return NULL;
    }

    object = vrms_scene_get_object_by_id(scene, mesh_id);
    if (NULL == object) {
        return NULL;
    }

    realized = 0;
    switch (object->type) {
        case VRMS_OBJECT_MESH_COLOR:
            realized = vrms_scene_mesh_color_realize(scene, object->object.object_mesh_color);
            break;
        case VRMS_OBJECT_MESH_TEXTURE:
            realized = vrms_scene_mesh_texture_realize(scene, object->object.object_mesh_texture);
            break;
        default:
            break;
    }

    if (!realized) {
        return NULL;
    }

    return object;
}
