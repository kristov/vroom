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
#include "safemalloc.h"
#include "vroom.h"
#include "object.h"
#include "scene.h"
#include "server.h"
#include "gl-matrix.h"
#include "gl.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define DEBUG_RENDER 1
#define debug_render_print(fmt, ...) do { if (DEBUG_RENDER) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define ALLOCATION_US_30FPS 33000
#define ALLOCATION_US_60FPS 16666

const char *VRMS_DATA_TYPE_NAMES[] = {
    "VRMS_VERTEX",
    "VRMS_NORMAL",
    "VRMS_INDEX",
    "VRMS_COLOR",
    "VRMS_UV",
    "VRMS_TEXTURE",
    "VRMS_MATRIX",
    "VRMS_PROGRAM",
    "VRMS_REGISTER"
};

void mat4_dump(float* M, const char* name) {
    if (!M) {
        fprintf(stderr, "undefined matrix\n");
        return;
    }

    fprintf(stderr, "+----------+----------+----------+----------+ %s\n", name);
    fprintf(stderr, "| %8.4f | %8.4f | %8.4f | %8.4f |\n", M[0], M[1], M[2], M[3]);
    fprintf(stderr, "| %8.4f | %8.4f | %8.4f | %8.4f |\n", M[4], M[5], M[6], M[7]);
    fprintf(stderr, "| %8.4f | %8.4f | %8.4f | %8.4f |\n", M[8], M[9], M[10], M[11]);
    fprintf(stderr, "| %8.4f | %8.4f | %8.4f | %8.4f |\n", M[12], M[13], M[14], M[15]);
    fprintf(stderr, "+----------+----------+----------+----------+\n");
    fprintf(stderr, "   trans1     trans2     trans3\n");
    fprintf(stderr, "\n");
}

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
        vrms_gl_delete_buffer(&data->gl_id);
    }
    vrms_object_data_destroy(data);
}

void vrms_scene_destroy_object_texture(vrms_object_texture_t* texture) {
    if (texture->gl_id > 0) {
        vrms_gl_delete_buffer(&texture->gl_id);
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
        vrms_gl_delete_buffer(&skybox->vertex_gl_id);
    }
    if (skybox->index_gl_id > 0) {
        vrms_gl_delete_buffer(&skybox->index_gl_id);
    }
    vrms_object_skybox_destroy(skybox);
}

void vrms_scene_destroy_object(vrms_scene_t* scene, uint32_t object_id) {
    vrms_object_t* object = vrms_scene_get_object_by_id(scene, object_id);
    if (!object) {
        return;
    }

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
        case VRMS_OBJECT_SCENE:
            // Not done here
            break;
        case VRMS_OBJECT_INVALID:
            // N/A
            break;
    }
}

void vrms_scene_destroy_objects(vrms_scene_t* scene) {
    uint32_t id;
    for (id = 1; id < scene->next_object_id; id++) {
        vrms_scene_destroy_object(scene, id);
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
    if (!pthread_mutex_lock(scene->scene_lock)) {
        debug_print("vrms_scene_destroy(): locked scene\n");
        vrms_scene_destroy_objects(scene);
        /*
        vrms_scene_empty_outbound_queue(scene);
        free(scene->outbound_queue);
        free(scene->outbound_queue_lock);
        */
        rendervm_destroy(scene->vm);

        pthread_mutex_t* scene_lock = scene->scene_lock;
        free(scene);

        debug_print("vrms_scene_destroy(): unlocked scene\n");
        pthread_mutex_unlock(scene_lock);
        free(scene_lock);
    }
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
    debug_print("    realized[%d]\n", object->realized);
    debug_print("    type[%s]\n", VRMS_DATA_TYPE_NAMES[type]);

    if (!object->realized) {
        uint8_t* buffer_ref = (uint8_t*)memory->address;
        void* buffer = &buffer_ref[memory_offset];
        uint32_t idx = vrms_server_queue_add_data_load(scene->server, memory_length, &object->object.object_data->gl_id, type, buffer);
        debug_print("    queue_idx[%d]\n", idx);
    }

    debug_print("\n");

    return object->id;
}

uint32_t vrms_scene_create_object_texture(vrms_scene_t* scene, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type) {

    vrms_object_data_t* data = vrms_scene_get_data_object_by_id(scene, data_id);
    if (!data) {
        debug_print("unable to find data object\n");
        return 0;
    }

    vrms_object_memory_t* memory = vrms_scene_get_memory_object_by_id(scene, data->memory_id);
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
    debug_print("    realized[%d]\n", object->realized);
    debug_print("    type[%d]\n", type);

    if (!object->realized) {
        uint8_t* buffer_ref = (uint8_t*)memory->address;
        void* buffer = &buffer_ref[data->memory_offset];
        uint32_t idx = vrms_server_queue_add_texture_load(scene->server, data->memory_length, &object->object.object_texture->gl_id, width, height, format, type, buffer);
        debug_print("    queue_idx[%d]\n", idx);
    }
    debug_print("\n");

    return object->id;
}

uint32_t vrms_scene_update_system_matrix(vrms_scene_t* scene, uint32_t data_id, uint32_t data_index, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type) {
    vrms_object_data_t* data = vrms_scene_get_data_object_by_id(scene, data_id);
    if (!data) {
        debug_print("unable to find data object\n");
        return 0;
    }

    vrms_object_memory_t* memory = vrms_scene_get_memory_object_by_id(scene, data->memory_id);
    if (!memory) {
        return 0;
    }

    uint8_t* buffer_ref = (uint8_t*)memory->address;
    float* matrix_array = (float*)&buffer_ref[data->memory_offset];
    float* matrix = &matrix_array[data_index * data->item_length];

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

    vrms_server_queue_add_data_load(scene->server, vertex_length, &skybox->vertex_gl_id, VRMS_VERTEX, (uint8_t*)skybox->vertex_data);

    uint32_t index_length = 36 * sizeof(uint16_t);
    skybox->index_data = SAFEMALLOC(index_length);
    memcpy(skybox->index_data, (uint8_t*)index_data, index_length);

    vrms_server_queue_add_data_load(scene->server, index_length, &skybox->index_gl_id, VRMS_INDEX, (uint8_t*)skybox->index_data);

    return object->id;
}

uint32_t vrms_scene_run_program(vrms_scene_t* scene, uint32_t program_id, uint32_t register_id) {
    uint8_t i = 0;

    rendervm_reset(scene->vm);

    vrms_object_data_t* reg_data = vrms_scene_get_data_object_by_id(scene, register_id);
    if (!reg_data) {
        debug_print("unable to find register data object\n");
        return 0;
    }

    vrms_object_memory_t* reg_memory = vrms_scene_get_memory_object_by_id(scene, reg_data->memory_id);
    if (!reg_memory) {
        debug_print("unable to find register memory object\n");
        return 0;
    }

    uint32_t reg_count = reg_data->memory_length / reg_data->data_length;
    uint8_t* reg_buffer = (uint8_t*)reg_memory->address;
    uint32_t* registers = (uint32_t*)&reg_buffer[reg_data->memory_offset];

    for (i = 0; i < reg_count; i++) {
        debug_print("setting register %d to %d\n", i, registers[i]);
        scene->vm->draw_reg[i] = registers[i];
    }

    vrms_object_data_t* prg_data = vrms_scene_get_data_object_by_id(scene, program_id);
    if (!prg_data) {
        debug_print("unable to find program data object\n");
        return 0;
    }

    vrms_object_memory_t* prg_memory = vrms_scene_get_memory_object_by_id(scene, prg_data->memory_id);
    if (!reg_memory) {
        debug_print("unable to find program memory object\n");
        return 0;
    }

    uint32_t prg_count = prg_data->memory_length / prg_data->data_length;
    uint8_t* prg_buffer = (uint8_t*)prg_memory->address;
    uint8_t* program = (uint8_t*)&prg_buffer[prg_data->memory_offset];

    debug_print("copying program of length[%d] to render buffer\n", prg_count);

    pthread_mutex_lock(scene->scene_lock);
    scene->render_buffer = program;
    scene->render_buffer_size = prg_count;
    pthread_mutex_unlock(scene->scene_lock);

    debug_print("program: ");
    for (i = 0; i < prg_count; i++) {
        debug_print("0x%02x ", scene->render_buffer[i]);
    }
    debug_print("\n");

    return 1;
}

uint32_t vrms_scene_draw(vrms_scene_t* scene, float* projection_matrix, float* view_matrix, float* model_matrix, float* skybox_projection_matrix) {
    uint32_t usec_elapsed;

    scene->matrix.p = projection_matrix;
    scene->matrix.v = view_matrix;

    if (!pthread_mutex_trylock(scene->scene_lock)) {
        debug_render_print("vrms_scene_draw(): locked scene\n");
        if ((!scene->render_buffer) || (0 == scene->render_buffer_size)) {
            pthread_mutex_unlock(scene->scene_lock);
            return 0;
        }

        uint32_t render_allocation_usec = scene->render_allocation_usec;
        render_allocation_usec = ALLOCATION_US_60FPS;
        rendervm_t* vm = scene->vm;

        debug_render_print("vrms_scene_draw(): allocation for render is %d usec\n", render_allocation_usec);

        struct timespec start;
        struct timespec end;
        start.tv_sec = 0;
        start.tv_nsec = 0;
        end.tv_sec = 0;
        end.tv_nsec = 0;
        usec_elapsed = 0;

        clock_gettime(CLOCK_MONOTONIC, &start);
        while (rendervm_exec(vm, scene->render_buffer, scene->render_buffer_size)) {

            clock_gettime(CLOCK_MONOTONIC, &end);
            uint64_t nsec_elapsed = ((1.0e+9 * end.tv_sec) + end.tv_nsec) - ((1.0e+9 * start.tv_sec) + start.tv_nsec);
            usec_elapsed += nsec_elapsed / 1000;

            debug_render_print("vrms_scene_draw(): executed 1 VM cycle in %d usec\n", usec_elapsed);

            if (usec_elapsed > render_allocation_usec) {
                //rendervm_interrupt(vm);
                debug_render_print("vrms_scene_draw(): allocation exceeded after %d usec\n", usec_elapsed);
            }
        }
        if (rendervm_has_exception(vm)) {
            // TODO: queue message to client that exception occurred
            debug_print("vrms_scene_draw(): VM has exception: 0x%02x\n", vm->exception);
        }

        pthread_mutex_unlock(scene->scene_lock);
        debug_render_print("vrms_scene_draw(): unlocked scene\n");
    }
    else {
        debug_render_print("vrms_scene_draw(): lock on render buffer\n");
    }

    return usec_elapsed;
}

// TODO this is called for every render call. It is likely that the matrix will
// be different each time, but unlikely that the matrix will come from a
// different memory object (ie: just an increment of the matrix_idx). This code
// should detect that and not do all this work every time.
void vrms_scene_attach_matrix(vrms_scene_t* scene) {
    rendervm_t* vm = scene->vm;
    uint32_t matrix_id = vm->draw_reg[4];
    uint32_t matrix_idx = vm->draw_reg[5];

    vrms_object_t* mat_object = vrms_scene_get_object_by_id(scene, matrix_id);
    if (!mat_object) {
        debug_render_print("vrms_scene_attach_matrix(): no matrix object for id: %d\n", matrix_id);
        return;
    }
    vrms_object_data_t* mat_data = mat_object->object.object_data;

    vrms_object_t* mem_object = vrms_scene_get_object_by_id(scene, mat_data->memory_id);
    if (!mem_object) {
        debug_render_print("vrms_scene_attach_matrix(): no memory object for id: %d\n", mat_data->memory_id);
        return;
    }

    vrms_object_memory_t* memory = mem_object->object.object_memory;
    if (!memory->address) {
        debug_render_print("vrms_scene_attach_matrix(): memory->address NULL\n");
        return;
    }
    uint8_t* buffer_ref = (uint8_t*)memory->address;
    float* matrix_array = (float*)&buffer_ref[mat_data->memory_offset];
    float* model_matrix = &matrix_array[matrix_idx * mat_data->item_length];

    scene->matrix.realized = 1;
    scene->matrix.m = model_matrix;

    mat4_copy(scene->matrix.mv, scene->matrix.v);
    mat4_multiply(scene->matrix.mv, scene->matrix.m);

    mat4_copy(scene->matrix.mvp, scene->matrix.p);
    mat4_multiply(scene->matrix.mvp, scene->matrix.v);
    mat4_multiply(scene->matrix.mvp, scene->matrix.m);
}

// TODO: add gpu_attempted flag to data object, pass vertex_id as reference and
// return code of vrms_scene_data_get_gl_id indicates if we should try again
// later or give up due to error.
uint32_t vrms_scene_data_get_gl_id(vrms_scene_t* scene, uint32_t object_id, uint8_t* found) {
    vrms_object_t* object = vrms_scene_get_object_by_id(scene, object_id);
    if (!object) {
        return 0;
    }
    vrms_object_data_t* object_data = object->object.object_data;
    if (0 != object_data->gl_id) {
        (*found)++;
        return object_data->gl_id;
    }
    return 0;
}

uint32_t vrms_scene_data_get_nr_indicies(vrms_scene_t* scene, uint32_t object_id) {
    vrms_object_t* object = vrms_scene_get_object_by_id(scene, object_id);
    if (!object) {
        return 0;
    }
    vrms_object_data_t* object_data = object->object.object_data;
    return object_data->memory_length / object_data->data_length;
}

void vrms_scene_render_realize_color(vrms_scene_t* scene) {
    rendervm_t* vm = scene->vm;
    uint8_t found = 0;
    if (vm->draw_reg[0]) {
        scene->render.vertex_id = vrms_scene_data_get_gl_id(scene, vm->draw_reg[0], &found);
    }
    if (vm->draw_reg[1]) {
        scene->render.normal_id = vrms_scene_data_get_gl_id(scene, vm->draw_reg[1], &found);
    }
    if (vm->draw_reg[2]) {
        scene->render.index_id = vrms_scene_data_get_gl_id(scene, vm->draw_reg[2], &found);
        scene->render.nr_indicies = vrms_scene_data_get_nr_indicies(scene, vm->draw_reg[2]);
    }
    if (vm->draw_reg[3]) {
        scene->render.color_id = vrms_scene_data_get_gl_id(scene, vm->draw_reg[3], &found);
    }
    if (found == 4) {
        scene->render.realized = 1;
    }
}

void vrms_scene_render_realize_texture(vrms_scene_t* scene) {
    rendervm_t* vm = scene->vm;
    uint8_t found = 0;
    if (vm->draw_reg[0]) {
        scene->render.vertex_id = vrms_scene_data_get_gl_id(scene, vm->draw_reg[1], &found);
    }
    if (vm->draw_reg[1]) {
        scene->render.normal_id = vrms_scene_data_get_gl_id(scene, vm->draw_reg[1], &found);
    }
    if (vm->draw_reg[2]) {
        scene->render.index_id = vrms_scene_data_get_gl_id(scene, vm->draw_reg[2], &found);
        scene->render.nr_indicies = vrms_scene_data_get_nr_indicies(scene, vm->draw_reg[2]);
    }
    if (vm->draw_reg[6]) {
        scene->render.uv_id = vrms_scene_data_get_gl_id(scene, vm->draw_reg[6], &found);
    }
    if (vm->draw_reg[7]) {
        scene->render.texture_id = vrms_scene_data_get_gl_id(scene, vm->draw_reg[7], &found);
    }
    if (found == 5) {
        scene->render.realized = 1;
    }
}

void vrms_scene_dump_render(vrms_scene_t* scene) {
    fprintf(stderr, "scene->render:\n");
    fprintf(stderr, "    vertex_id: %d\n", scene->render.vertex_id);
    fprintf(stderr, "    normal_id: %d\n", scene->render.normal_id);
    fprintf(stderr, "    index_id: %d\n", scene->render.index_id);
    fprintf(stderr, "    color_id: %d\n", scene->render.color_id);
    fprintf(stderr, "    uv_id: %d\n", scene->render.uv_id);
    fprintf(stderr, "    texture_id: %d\n", scene->render.texture_id);
    fprintf(stderr, "    realized: %d\n", scene->render.realized);
}

void vrms_scene_vm_callback(rendervm_t* vm, rendervm_opcode_t opcode, void* user_data) {
    vrms_scene_t* scene = (vrms_scene_t*)user_data;
    switch ((uint8_t)opcode) {
        case 0xc8:
            vrms_scene_dump_render(scene);
            vrms_scene_render_realize_color(scene);
            vrms_scene_attach_matrix(scene);
            scene->render.shader_id = scene->server->color_shader_id;
            vrms_gl_draw_mesh_color(scene->render, scene->matrix, 0.0f, 1.0f, 0.0f, 1.0f);
            break;
        case 0xc9:
            debug_render_print("vrms_scene_vm_callback(): vrms_gl_draw_mesh_texture\n");
            vrms_scene_render_realize_texture(scene);
            vrms_scene_attach_matrix(scene);
            scene->render.shader_id = scene->server->texture_shader_id;
            vrms_gl_draw_mesh_texture(scene->render, scene->matrix);
            break;
        default:
            break;
    }
}

vrms_scene_t* vrms_scene_create(char* name) {
    vrms_scene_t* scene = SAFEMALLOC(sizeof(vrms_scene_t));
    memset(scene, 0, sizeof(vrms_scene_t));

    scene->objects = SAFEMALLOC(sizeof(vrms_object_t) * 10);
    memset(scene->objects, 0, sizeof(vrms_object_t) * 10);
    scene->next_object_id = 1;

    scene->render_buffer_size = 0;
    scene->scene_lock = SAFEMALLOC(sizeof(pthread_mutex_t));
    memset(scene->scene_lock, 0, sizeof(pthread_mutex_t));

    scene->vm = rendervm_create();
    rendervm_attach_callback(scene->vm, &vrms_scene_vm_callback, (void*)scene);

    return scene;
}
