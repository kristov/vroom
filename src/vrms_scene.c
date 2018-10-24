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
#include "vrms_render_vm.h"
#include "vrms_scene.h"
#include "vrms_server.h"
#include "esm.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define DEBUG_RENDER 0
#define debug_render_print(fmt, ...) do { if (DEBUG_RENDER) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define ALLOCATION_US_30FPS 33000
#define ALLOCATION_US_60FPS 16666

vrms_object_t* vrms_scene_get_object_by_id(vrms_scene_t* scene, uint32_t id) {
    vrms_object_t* vrms_object;
    if (scene->next_object_id <= id) {
        debug_print("id: %d out of range\n", id);
        return NULL;
    }
    vrms_object = scene->objects[id];
    if (!vrms_object) {
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
    if (!object) {
        return NULL;
    }

    if (VRMS_OBJECT_MEMORY != object->type) {
        debug_print("vrms_scene_get_memory_object_by_id: asked for an id that is not a memory object\n");
        return NULL;
    }

    return object->object.object_memory;
}

vrms_object_data_t* vrms_scene_get_data_object_by_id(vrms_scene_t* scene, uint32_t data_id) {
    vrms_object_t* object;

    if (0 == data_id) {
        debug_print("vrms_scene_get_data_object_by_id: no data id passed\n");
        return NULL;
    }

    object = vrms_scene_get_object_by_id(scene, data_id);
    if (!object) {
        return NULL;
    }

    if (VRMS_OBJECT_DATA != object->type) {
        debug_print("vrms_scene_get_data_object_by_id: asked for an id that is not a data object\n");
        return NULL;
    }

    return object->object.object_data;
}

vrms_object_program_t* vrms_scene_get_program_object_by_id(vrms_scene_t* scene, uint32_t program_id) {
    vrms_object_t* object;

    if (0 == program_id) {
        debug_print("vrms_scene_get_program_object_by_id: no program id passed\n");
        return NULL;
    }

    object = vrms_scene_get_object_by_id(scene, program_id);
    if (!object) {
        return NULL;
    }

    if (VRMS_OBJECT_PROGRAM != object->type) {
        debug_print("vrms_scene_get_program_object_by_id: asked for an id that is not a program object\n");
        return NULL;
    }

    return object->object.object_program;
}

void vrms_scene_destroy_object_memory(vrms_object_memory_t* memory) {
    if (NULL != memory->address) {
        munmap(memory->address, memory->size);
    }
    vrms_object_memory_destroy(memory);
}

void vrms_scene_destroy_object_data(vrms_object_data_t* data) {
    if (NULL != data->local_storage) {
        free(data->local_storage);
    }
    if (data->gl_id > 0) {
        glDeleteBuffers(1, &data->gl_id);
    }
    vrms_object_data_destroy(data);
}

void vrms_scene_destroy_object_texture(vrms_object_texture_t* texture) {
    if (texture->gl_id > 0) {
        glDeleteBuffers(1, &texture->gl_id);
    }
    vrms_object_texture_destroy(texture);
}

void vrms_scene_destroy_object_skybox(vrms_object_skybox_t* skybox) {
    if (NULL != skybox->vertex_data) {
        free(skybox->vertex_data);
    }
    if (NULL != skybox->index_data) {
        free(skybox->index_data);
    }
    if (skybox->vertex_gl_id > 0) {
        glDeleteBuffers(1, &skybox->vertex_gl_id);
    }
    if (skybox->index_gl_id > 0) {
        glDeleteBuffers(1, &skybox->index_gl_id);
    }
    vrms_object_skybox_destroy(skybox);
}

void vrms_scene_destroy_object(vrms_object_t* object) {
    switch (object->type) {
        case VRMS_OBJECT_MEMORY:
            vrms_scene_destroy_object_memory(object->object.object_memory);
            break;
        case VRMS_OBJECT_DATA:
            vrms_scene_destroy_object_data(object->object.object_data);
            break;
        case VRMS_OBJECT_TEXTURE:
            vrms_scene_destroy_object_texture(object->object.object_texture);
            break;
        case VRMS_OBJECT_SKYBOX:
            vrms_scene_destroy_object_skybox(object->object.object_skybox);
            break;
        case VRMS_OBJECT_GEOMETRY:
            vrms_object_geometry_destroy(object->object.object_geometry);
            break;
        case VRMS_OBJECT_MESH_COLOR:
            vrms_object_mesh_color_destroy(object->object.object_mesh_color);
            break;
        case VRMS_OBJECT_MESH_TEXTURE:
            vrms_object_mesh_texture_destroy(object->object.object_mesh_texture);
            break;
        case VRMS_OBJECT_PROGRAM:
            vrms_object_program_destroy(object->object.object_program);
            break;
        case VRMS_OBJECT_SCENE:
            // Not done here
            break;
        case VRMS_OBJECT_INVALID:
            // N/A
            break;
    }
}

void vrms_scene_destroy_objects(vrms_scene_t* scene) {
    uint32_t idx;
    vrms_object_t* object;

    for (idx = 1; idx < scene->next_object_id; idx++) {
        object = vrms_scene_get_object_by_id(scene, idx);
        vrms_scene_destroy_object(object);
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
    // TODO: Need locking here. The server might choose to destroy a scene
    // while an object is being added.
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

    vrms_render_vm_destroy(scene->vm);

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

    vrms_object_t* object = vrms_object_memory_create(fd, address, size);
    vrms_scene_add_object(scene, object);

    return object->id;
}

uint32_t vrms_scene_create_object_data(vrms_scene_t* scene, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, uint32_t item_length, uint32_t data_length, vrms_data_type_t type) {
    void* buffer;
    vrms_object_memory_t* memory;

    memory = vrms_scene_get_memory_object_by_id(scene, memory_id);
    if (!memory) {
        return 0;
    }

    if ((memory_offset + memory_length) > memory->size) {
        debug_print("create_object_data: read beyond memory size!\n");
    }

    vrms_object_t* object = vrms_object_data_create(memory_id, memory_offset, memory_length, item_length, data_length, type);
    vrms_scene_add_object(scene, object);

    debug_print("created data object[%d]:\n", object->id);
    debug_print("    memory_id[%d]\n", memory_id);
    debug_print("    memory_offset[%d]\n", memory_offset);
    debug_print("    memory_length[%d]\n", memory_length);
    debug_print("    item_length[%d]\n", item_length);
    debug_print("    data_length[%d]\n", data_length);
    debug_print("    type[%d]\n", type);
    debug_print("\n");

    if (!object->realized) {
        debug_print("not realized, copying to GPU\n");
        buffer = &((uint8_t*)memory->address)[memory_offset];
        vrms_server_queue_add_data_load(scene->server, memory_length, &object->object.object_data->gl_id, type, buffer);
    }

    return object->id;
}

uint32_t vrms_scene_create_object_texture(vrms_scene_t* scene, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type) {
    void* buffer;
    vrms_object_data_t* data;
    vrms_object_memory_t* memory;

    data = vrms_scene_get_data_object_by_id(scene, data_id);
    if (!data) {
        debug_print("unable to find data object\n");
        return 0;
    }

    memory = vrms_scene_get_memory_object_by_id(scene, data->memory_id);
    if (!memory) {
        debug_print("unable to find memory object\n");
        return 0;
    }

    vrms_object_t* object = vrms_object_texture_create(data_id, width, height, format, type);
    vrms_scene_add_object(scene, object);

    debug_print("created texture object[%d]:\n", object->id);
    debug_print("    data_id[%d]\n", data_id);
    debug_print("    width[%d]\n", width);
    debug_print("    height[%d]\n", height);
    debug_print("    format[%d]\n", format);
    debug_print("    type[%d]\n", type);
    debug_print("\n");

    if (!object->realized) {
        buffer = &((uint8_t*)memory->address)[data->memory_offset];
        vrms_server_queue_add_texture_load(scene->server, data->memory_length, &object->object.object_texture->gl_id, width, height, format, type, buffer);
    }

    return object->id;
}

uint32_t vrms_scene_create_object_geometry(vrms_scene_t* scene, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id) {
    vrms_object_t* object = vrms_object_geometry_create(vertex_id, normal_id, index_id);
    vrms_scene_add_object(scene, object);

    debug_print("created geometry object[%d]:\n", object->id);
    debug_print("    vertex_id[%d]\n", vertex_id);
    debug_print("    normal_id[%d]\n", normal_id);
    debug_print("    index_id[%d]\n", index_id);
    debug_print("\n");

    return object->id;
}

uint32_t vrms_scene_create_object_mesh_color(vrms_scene_t* scene, uint32_t geometry_id, float r, float g, float b, float a) {
    vrms_object_t* object = vrms_object_mesh_color_create(geometry_id, r, g, b, a);
    vrms_scene_add_object(scene, object);

    debug_print("created colored mesh object[%d]:\n", object->id);
    debug_print("    geometry_id[%d]\n", geometry_id);
    debug_print("    r[%0.2f]\n", r);
    debug_print("    g[%0.2f]\n", g);
    debug_print("    b[%0.2f]\n", b);
    debug_print("    a[%0.2f]\n", a);
    debug_print("\n");

    return object->id;
}

uint32_t vrms_scene_create_object_mesh_texture(vrms_scene_t* scene, uint32_t geometry_id, uint32_t texture_id, uint32_t uv_id) {
    vrms_object_t* object = vrms_object_mesh_texture_create(geometry_id, texture_id, uv_id);
    vrms_scene_add_object(scene, object);

    debug_print("created textured mesh object[%d]:\n", object->id);
    debug_print("    geometry_id[%d]\n", geometry_id);
    debug_print("    texture_id[%d]\n", texture_id);
    debug_print("    uv_id[%d]\n", uv_id);
    debug_print("\n");

    return object->id;
}

uint32_t vrms_scene_update_system_matrix(vrms_scene_t* scene, uint32_t data_id, uint32_t data_index, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type) {
    float* matrix_array;
    float* matrix;
    vrms_object_data_t* data;
    vrms_object_memory_t* memory;

    data = vrms_scene_get_data_object_by_id(scene, data_id);
    if (!data) {
        debug_print("unable to find data object\n");
        return 0;
    }

    memory = vrms_scene_get_memory_object_by_id(scene, data->memory_id);
    if (!memory) {
        return 0;
    }

    matrix_array = &((float*)memory->address)[data->memory_offset];
    matrix = &matrix_array[data_index * data->item_length];

    vrms_server_queue_update_system_matrix(scene->server, matrix_type, update_type, (uint8_t*)matrix);

    return 1;
}

uint32_t vrms_scene_create_object_skybox(vrms_scene_t* scene, uint32_t texture_id) {
    vrms_object_skybox_t* skybox;

    float vertex_data[] = {
         100.0f, -100.0f,  100.0f,
         100.0f, -100.0f, -100.0f,
         100.0f,  100.0f,  100.0f,
         100.0f,  100.0f, -100.0f,
        -100.0f, -100.0f, -100.0f,
        -100.0f, -100.0f,  100.0f,
        -100.0f,  100.0f, -100.0f,
        -100.0f,  100.0f,  100.0f
    };

    uint16_t index_data[] = {
        0, 1, 2, 2, 1, 3,
        4, 5, 6, 6, 5, 7,
        7, 2, 6, 6, 2, 3,
        4, 1, 5, 5, 1, 0,
        5, 0, 7, 7, 0, 2,
        1, 4, 3, 3, 4, 6
    };

    vrms_object_t* object = vrms_object_skybox_create(texture_id);
    vrms_scene_add_object(scene, object);
    skybox = object->object.object_skybox;

    debug_print("created skybox object[%d]:\n", object->id);
    debug_print("    texture_id[%d]\n", texture_id);
    debug_print("\n");

    uint32_t vertex_length = 8 * 3 * sizeof(float);
    skybox->vertex_data = SAFEMALLOC(vertex_length);
    memcpy(skybox->vertex_data, (uint8_t*)vertex_data, vertex_length);

    vrms_server_queue_add_data_load(scene->server, vertex_length, &skybox->vertex_gl_id, VRMS_VERTEX, skybox->vertex_data);

    uint32_t index_length = 36 * sizeof(uint16_t);
    skybox->index_data = SAFEMALLOC(index_length);
    memcpy(skybox->index_data, (uint8_t*)index_data, index_length);

    vrms_server_queue_add_data_load(scene->server, index_length, &skybox->index_gl_id, VRMS_INDEX, skybox->index_data);

    return object->id;
}

uint32_t vrms_scene_create_program(vrms_scene_t* scene, uint32_t data_id) {
    vrms_object_data_t* data;
    vrms_object_memory_t* memory;
    vrms_object_program_t* program;

    data = vrms_scene_get_data_object_by_id(scene, data_id);
    if (!data) {
        debug_print("unable to find data object\n");
        return 0;
    }

    memory = vrms_scene_get_memory_object_by_id(scene, data->memory_id);
    if (!memory) {
        debug_print("unable to find memory object\n");
        return 0;
    }

    vrms_object_t* object = vrms_object_program_create(data->memory_length);
    vrms_scene_add_object(scene, object);
    program = object->object.object_program;

    program->data = SAFEMALLOC(data->memory_length);
    memcpy(program->data, &((uint8_t*)memory->address)[data->memory_offset], data->memory_length);

    return object->id;
}

uint32_t vrms_scene_run_program(vrms_scene_t* scene, uint32_t program_id, uint32_t register_id) {
    vrms_object_data_t* data;
    vrms_object_memory_t* memory;
    vrms_object_program_t* program;
    uint32_t nr_regs;
    uint32_t* registers;
    uint8_t i;
    uint32_t program_memory_length;

    data = vrms_scene_get_data_object_by_id(scene, register_id);
    if (!data) {
        debug_print("unable to find register data object\n");
        return 0;
    }

    memory = vrms_scene_get_memory_object_by_id(scene, data->memory_id);
    if (!memory) {
        debug_print("unable to find memory object\n");
        return 0;
    }

    program = vrms_scene_get_program_object_by_id(scene, program_id);
    if (!program) {
        debug_print("no program found for id %d\n", program_id);
        return 0;
    }

    vrms_render_vm_reset(scene->vm);
    nr_regs = data->memory_length / data->data_length;
    registers = &((uint32_t*)memory->address)[data->memory_offset];

    for (i = 0; i < nr_regs; i++) {
        debug_print("setting register %d to %d\n", i, registers[i]);
        vrms_render_vm_iregister_set(scene->vm, i, registers[i]);
    }

    program_memory_length = program->length * sizeof(uint8_t);
    debug_print("copying program of length[%d] to render buffer\n", program_memory_length);

    pthread_mutex_lock(scene->render_buffer_lock);
    if (NULL != scene->render_buffer) {
        free(scene->render_buffer);
    }
    scene->render_buffer = SAFEMALLOC(program_memory_length);
    scene->render_buffer_size = program_memory_length;
    memcpy(scene->render_buffer, program->data, program_memory_length);
    pthread_mutex_unlock(scene->render_buffer_lock);

    debug_print("program: ");
    for (i = 0; i < program_memory_length; i++) {
        debug_print("0x%02x ", scene->render_buffer[i]);
    }
    debug_print("\n");

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
    if (!object) {
        return 0;
    }
    geometry = object->object.object_geometry;

    object = vrms_scene_get_object_by_id(scene, geometry->vertex_id);
    if (!object) {
        return 0;
    }
    vertex = object->object.object_data;
    if (0 != vertex->gl_id) {
        mesh->vertex_gl_id = vertex->gl_id;
    }

    object = vrms_scene_get_object_by_id(scene, geometry->normal_id);
    if (!object) {
        return 0;
    }
    normal = object->object.object_data;
    if (0 != normal->gl_id) {
        mesh->normal_gl_id = normal->gl_id;
    }

    object = vrms_scene_get_object_by_id(scene, geometry->index_id);
    if (!object) {
        return 0;
    }
    index = object->object.object_data;
    if (0 != index->gl_id) {
        mesh->index_gl_id = index->gl_id;
        mesh->nr_indicies = index->memory_length / index->data_length;
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

    if ((0 != mesh->vertex_gl_id) && (0 != mesh->normal_gl_id) && (0 != mesh->index_gl_id) && (0 != mesh->uv_gl_id) && (0 != mesh->texture_gl_id)) {
        return 1;
    }

    object = vrms_scene_get_object_by_id(scene, mesh->geometry_id);
    if (!object) {
        debug_print("no geometry object\n");
        return 0;
    }
    geometry = object->object.object_geometry;

    object = vrms_scene_get_object_by_id(scene, geometry->vertex_id);
    if (!object) {
        debug_print("no vertex object\n");
        return 0;
    }
    vertex = object->object.object_data;
    if (0 != vertex->gl_id) {
        mesh->vertex_gl_id = vertex->gl_id;
    }

    object = vrms_scene_get_object_by_id(scene, geometry->normal_id);
    if (!object) {
        debug_print("no normal object\n");
        return 0;
    }
    normal = object->object.object_data;
    if (0 != normal->gl_id) {
        mesh->normal_gl_id = normal->gl_id;
    }

    object = vrms_scene_get_object_by_id(scene, mesh->texture_id);
    if (!object) {
        return 0;
    }
    texture = object->object.object_texture;
    if (0 != texture->gl_id) {
        mesh->texture_gl_id = texture->gl_id;
    }

    object = vrms_scene_get_object_by_id(scene, geometry->index_id);
    if (!object) {
        return 0;
    }
    index = object->object.object_data;
    if (0 != index->gl_id) {
        mesh->index_gl_id = index->gl_id;
        mesh->nr_indicies = index->memory_length / index->data_length;
    }

    object = vrms_scene_get_object_by_id(scene, mesh->uv_id);
    if (!object) {
        return 0;
    }
    uv = object->object.object_data;
    if (0 != uv->gl_id) {
        mesh->uv_gl_id = uv->gl_id;
    }

    if ((0 != mesh->vertex_gl_id) && (0 != mesh->normal_gl_id) && (0 != mesh->index_gl_id) && (0 != mesh->uv_gl_id) && (0 != mesh->texture_gl_id)) {
        return 1;
    }

    return 0;
}

uint8_t vrms_scene_skybox_realize(vrms_scene_t* scene, vrms_object_skybox_t* skybox) {
    vrms_object_t* object;
    vrms_object_texture_t* texture;

    if ((0 != skybox->vertex_gl_id) && (0 != skybox->texture_gl_id)) {
        return 1;
    }

    object = vrms_scene_get_object_by_id(scene, skybox->texture_id);
    if (!object) {
        return 0;
    }
    texture = object->object.object_texture;
    if (0 != texture->gl_id) {
        skybox->texture_gl_id = texture->gl_id;
    }

    if ((0 != skybox->vertex_gl_id) && (0 != skybox->texture_gl_id)) {
        return 1;
    }

    return 1;
}

void vrms_server_draw_scene_object(vrms_scene_t* scene, uint32_t object_id, float* projection_matrix, float* view_matrix, float* model_matrix) {
    vrms_object_t* object;
    vrms_object_mesh_color_t* mesh_color;
    vrms_object_mesh_texture_t* mesh_texture;
    vrms_object_skybox_t* skybox;
    uint8_t realized;

    if (object_id >= scene->next_object_id) {
        debug_render_print("vrms_server_draw_scene_object(): object: %d is invalid\n", object_id);
        return;
    }

    object = vrms_scene_get_object_by_id(scene, object_id);
    if (!object) {
        debug_render_print("vrms_server_draw_scene_object(): object: %d NULL\n", object_id);
        return;
    }

    debug_render_print("vrms_server_draw_scene_object(): object: %d drawing...\n", object_id);
    switch (object->type) {
        case VRMS_OBJECT_MESH_COLOR:
            mesh_color = object->object.object_mesh_color;
            realized = vrms_scene_mesh_color_realize(scene, mesh_color);
            if (!realized) {
                debug_render_print("vrms_server_draw_scene_object(): object: %d not realized\n", object_id);
                return;
            }
            vrms_server_draw_mesh_color(scene->server, mesh_color, projection_matrix, view_matrix, model_matrix);
            break;
        case VRMS_OBJECT_MESH_TEXTURE:
            mesh_texture = object->object.object_mesh_texture;
            realized = vrms_scene_mesh_texture_realize(scene, mesh_texture);
            if (!realized) {
                debug_render_print("vrms_server_draw_scene_object(): object: %d not realized\n", object_id);
                return;
            }
            vrms_server_draw_mesh_texture(scene->server, mesh_texture, projection_matrix, view_matrix, model_matrix);
            break;
        case VRMS_OBJECT_SKYBOX:
            skybox = object->object.object_skybox;
            realized = vrms_scene_skybox_realize(scene, skybox);
            if (!realized) {
                debug_render_print("vrms_server_draw_scene_object(): object: %d not realized\n", object_id);
                return;
            }
            vrms_server_draw_skybox(scene->server, skybox, projection_matrix, view_matrix, model_matrix);
            break;
        default:
            break;
    }
}

uint32_t vrms_scene_draw(vrms_scene_t* scene, float* projection_matrix, float* view_matrix, float* model_matrix, float* skybox_projection_matrix) {
    struct timespec start;
    struct timespec end;
    uint32_t render_allocation_usec;
    uint32_t usec_elapsed;
    uint64_t nsec_elapsed;
    vrms_render_vm_t* vm;

    if ((!scene->render_buffer) || (0 == scene->render_buffer_size)) {
        debug_render_print("vrms_scene_draw(): no render_buffer\n");
        return 0;
    }

    render_allocation_usec = scene->render_allocation_usec;
    render_allocation_usec = ALLOCATION_US_60FPS;
    vm = scene->vm;

    debug_render_print("vrms_scene_draw(): allocation for render is %d usec\n", render_allocation_usec);
    if (!pthread_mutex_trylock(scene->render_buffer_lock)) {

        vrms_render_vm_sysmregister_set(vm, VM_SYSMREG_PROJECTION, projection_matrix);
        vrms_render_vm_sysmregister_set(vm, VM_SYSMREG_VIEW, view_matrix);
        vrms_render_vm_sysiregister_set(vm, VM_SYSIREG_USEC_ALLOC, render_allocation_usec);

        start.tv_sec = 0;
        start.tv_nsec = 0;
        end.tv_sec = 0;
        end.tv_nsec = 0;
        usec_elapsed = 0;

        vrms_render_vm_sysiregister_set(vm, VM_SYSIREG_USEC_ELAPSED, usec_elapsed);
        clock_gettime(CLOCK_MONOTONIC, &start);

        while (vrms_render_vm_exec(vm, scene->render_buffer, scene->render_buffer_size)) {

            clock_gettime(CLOCK_MONOTONIC, &end);
            nsec_elapsed = ((1.0e+9 * end.tv_sec) + end.tv_nsec) - ((1.0e+9 * start.tv_sec) + start.tv_nsec);
            usec_elapsed += nsec_elapsed / 1000;

            debug_render_print("vrms_scene_draw(): executed 1 VM cycle in %d usec\n", usec_elapsed);

            if (usec_elapsed > render_allocation_usec) {
                vrms_render_vm_alloc_ex_interrupt(vm);
                debug_render_print("vrms_scene_draw(): allocation exceeded after %d usec\n", usec_elapsed);
            }

            vrms_render_vm_sysiregister_set(vm, VM_SYSIREG_USEC_ELAPSED, usec_elapsed);
        }
        if (vrms_render_vm_has_exception(vm)) {
            // TODO: queue message to client that exception occurred
            debug_render_print("vrms_scene_draw(): VM has exception\n");
            vrms_render_vm_reset(vm);
        }

        pthread_mutex_unlock(scene->render_buffer_lock);
        vrms_render_vm_resume(scene->vm);
    }
    else {
        debug_render_print("vrms_scene_draw(): lock on render buffer\n");
    }

    return usec_elapsed;
}

float* vrms_scene_vm_load_matrix(vrms_render_vm_t* vm, uint32_t data_id, uint32_t matrix_idx, void* user_data) {
    vrms_scene_t* scene;
    vrms_object_t* data_object;
    vrms_object_t* memory_object;
    vrms_object_data_t* data;
    vrms_object_memory_t* memory;
    float* matrix;

    if (!user_data) {
        debug_render_print("vrms_scene_vm_load_matrix(): user_data NULL\n");
        return NULL;
    }

    scene = (vrms_scene_t*)user_data;

    data_object = vrms_scene_get_object_by_id(scene, data_id);
    if (!data_object) {
        debug_render_print("vrms_scene_vm_load_matrix(): data_object[%d] NULL\n", data_id);
        return NULL;
    }

    data = data_object->object.object_data;

    memory_object = vrms_scene_get_object_by_id(scene, data->memory_id);
    if (!memory_object) {
        debug_render_print("vrms_scene_vm_load_matrix(): memory_object NULL\n");
        return NULL;
    }

    memory = memory_object->object.object_memory;
    if (!memory->address) {
        debug_render_print("vrms_scene_vm_load_matrix(): memory->address NULL\n");
        return NULL;
    }
    matrix = &((float*)memory->address)[data->memory_offset];

    return matrix;
}

void vrms_scene_vm_draw(vrms_render_vm_t* vm, float* model_matrix, uint32_t object_id, void* user_data) {
    vrms_scene_t* scene;
    float* projection_matrix;
    float* view_matrix;

    if (!user_data) {
        debug_render_print("vrms_scene_vm_draw(): user_data NULL\n");
        return;
    }

    scene = (vrms_scene_t*)user_data;

    projection_matrix = vrms_render_vm_sysmregister_get(vm, VM_REG0);
    view_matrix = vrms_render_vm_sysmregister_get(vm, VM_REG1);

    vrms_server_draw_scene_object(scene, object_id, projection_matrix, view_matrix, model_matrix);
}

vrms_scene_t* vrms_scene_create(char* name) {
    vrms_scene_t* scene = SAFEMALLOC(sizeof(vrms_scene_t));
    memset(scene, 0, sizeof(vrms_scene_t));

    scene->objects = SAFEMALLOC(sizeof(vrms_object_t) * 10);
    memset(scene->objects, 0, sizeof(vrms_object_t) * 10);
    scene->next_object_id = 1;

    scene->render_buffer_size = 0;
    scene->render_buffer_lock = SAFEMALLOC(sizeof(pthread_mutex_t));
    memset(scene->render_buffer_lock, 0, sizeof(pthread_mutex_t));

    scene->vm = vrms_render_vm_create();
    scene->vm->load_matrix = &vrms_scene_vm_load_matrix;
    scene->vm->draw = &vrms_scene_vm_draw;
    scene->vm->user_data = (void*)scene;

    return scene;
}
