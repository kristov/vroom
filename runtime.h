#ifndef VRMS_RUNTIME_H
#define VRMS_RUNTIME_H

#include "vroom.h"
#include <pthread.h>

typedef struct vrms_server vrms_server_t;
typedef struct vrms_module vrms_module_t;
typedef struct vrms_module_interface vrms_module_interface_t;

typedef struct vrms_runtime {
    vrms_module_t* modules[10];
    uint8_t nr_modules;
    vrms_server_t* vrms_server;
    char* module_load_path;
    uint32_t w;
    uint32_t h;
} vrms_runtime_t;

typedef struct vrms_module_interface {
    int (*log)(vrms_module_t* module, const char *format, ...);
    uint32_t (*create_scene)(vrms_module_t* module, char* name);
    uint32_t (*create_memory)(vrms_module_t* module, uint32_t scene_id, uint32_t fd, uint32_t size);
    uint32_t (*create_object_data)(vrms_module_t* module, uint32_t scene_id, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, vrms_data_type_t type);
    uint32_t (*create_object_texture)(vrms_module_t* module, uint32_t scene_id, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);
    uint32_t (*attach_memory)(vrms_module_t* module, uint32_t scene_id, uint32_t data_id);
    uint32_t (*run_program)(vrms_module_t* module, uint32_t scene_id, uint32_t program_id, uint32_t register_id);
    uint32_t (*set_skybox)(vrms_module_t* module, uint32_t scene_id, uint32_t texture_id);
    uint32_t (*destroy_scene)(vrms_module_t* module, uint32_t scene_id);
    uint32_t (*destroy_object)(vrms_module_t* module, uint32_t scene_id, uint32_t object_id);
    uint32_t (*update_system_matrix)(vrms_module_t* module, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix);
} vrms_module_interface_t;

typedef struct vrms_module {
    char* name;
    vrms_module_interface_t interface;
    vrms_runtime_t* runtime;
    pthread_t pthread;
} vrms_module_t;

vrms_runtime_t* vrms_runtime_init(int width, int height, double physical_width);

void vrms_runtime_display(vrms_runtime_t* vrms_runtime);

void vrms_runtime_reshape(vrms_runtime_t* vrms_runtime, int w, int h);

void vrms_runtime_process(vrms_runtime_t* vrms_runtime);

void vrms_runtime_end(vrms_runtime_t* vrms_runtime);

int vrms_module_log(vrms_module_t* module, const char *format, ...);

uint32_t vrms_module_create_scene(vrms_module_t* module, char* name);

uint32_t vrms_module_create_memory(vrms_module_t* module, uint32_t scene_id, uint32_t fd, uint32_t size);

uint32_t vrms_module_create_object_data(vrms_module_t* module, uint32_t scene_id, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, vrms_data_type_t type);

uint32_t vrms_module_create_object_texture(vrms_module_t* module, uint32_t scene_id, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);

uint32_t vrms_module_create_program(vrms_module_t* module, uint32_t scene_id, uint32_t data_id);

uint32_t vrms_module_attach_memory(vrms_module_t* module, uint32_t scene_id, uint32_t data_id);

uint32_t vrms_module_run_program(vrms_module_t* module, uint32_t scene_id, uint32_t program_id, uint32_t register_id);

uint32_t vrms_module_update_system_matrix(vrms_module_t* module, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix);

uint32_t vrms_module_set_skybox(vrms_module_t* module, uint32_t scene_id, uint32_t texture_id);

uint32_t vrms_module_destroy_scene(vrms_module_t* module, uint32_t scene_id);

uint32_t vrms_module_destroy_object(vrms_module_t* module, uint32_t scene_id, uint32_t object_id);

uint32_t vrms_runtime_update_system_matrix(vrms_runtime_t* vrms_runtime, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix);

#endif
