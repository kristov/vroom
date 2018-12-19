#ifndef VRMS_RUNTIME_H
#define VRMS_RUNTIME_H

#include "vroom.h"
#include <pthread.h>

typedef struct vrms_server vrms_server_t;
typedef struct vrms_runtime vrms_runtime_t;
typedef struct vrms_runtime_interface vrms_runtime_interface_t;

typedef struct vrms_runtime_module_thread {
    char* module_name;
    vrms_runtime_t* vrms_runtime;
    pthread_t pthread;
} vrms_runtime_module_thread_t;

typedef struct vrms_runtime {
    vrms_runtime_module_thread_t* module_threads[10];
    uint8_t nr_module_threads;
    vrms_server_t* vrms_server;
    vrms_runtime_interface_t* interface;
    char* module_load_path;
    uint32_t w;
    uint32_t h;
} vrms_runtime_t;

typedef struct vrms_runtime_interface {
    uint32_t (*create_scene)(vrms_runtime_t* vrms_runtime, char* name);
    uint32_t (*create_memory)(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t fd, uint32_t size);
    uint32_t (*create_object_data)(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, vrms_data_type_t type);
    uint32_t (*create_object_texture)(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);
    uint32_t (*create_program)(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t data_id);
    uint32_t (*run_program)(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t program_id, uint32_t register_id);
    uint32_t (*set_skybox)(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t texture_id);
    uint32_t (*destroy_scene)(vrms_runtime_t* vrms_runtime, uint32_t scene_id);
    uint32_t (*destroy_object)(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t object_id);
    uint32_t (*update_system_matrix)(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t data_id, uint32_t data_index, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type);
    uint32_t (*update_system_matrix_module)(vrms_runtime_t* vrms_runtime, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix);
} vrms_runtime_interface_t;

vrms_runtime_t* vrms_runtime_init(int width, int height, double physical_width);

void vrms_runtime_display(vrms_runtime_t* vrms_runtime);

void vrms_runtime_reshape(vrms_runtime_t* vrms_runtime, int w, int h);

void vrms_runtime_process(vrms_runtime_t* vrms_runtime);

void vrms_runtime_end(vrms_runtime_t* vrms_runtime);

uint32_t vrms_runtime_create_scene(vrms_runtime_t* vrms_runtime, char* name);

uint32_t vrms_runtime_create_memory(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t fd, uint32_t size);

uint32_t vrms_runtime_create_object_data(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, vrms_data_type_t type);

uint32_t vrms_runtime_create_object_texture(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);

uint32_t vrms_runtime_create_program(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t data_id);

uint32_t vrms_runtime_run_program(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t program_id, uint32_t register_id);

uint32_t vrms_runtime_update_system_matrix(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t data_id, uint32_t data_index, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type);

uint32_t vrms_runtime_set_skybox(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t texture_id);

uint32_t vrms_runtime_destroy_scene(vrms_runtime_t* vrms_runtime, uint32_t scene_id);

uint32_t vrms_runtime_destroy_object(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t object_id);

uint32_t vrms_runtime_update_system_matrix_module(vrms_runtime_t* vrms_runtime, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix);

#endif
