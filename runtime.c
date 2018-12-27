#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include "gl.h"
#include "safemalloc.h"
#include "object.h"
#include "server.h"
#include "scene.h"
#include "esm.h"
#include "opengl_stereo.h"
#include "vroom.h"
#include "runtime.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

opengl_stereo* ostereo;

vrms_runtime_interface_t runtime_interface;

void fill_interface() {
    runtime_interface.create_scene = vrms_runtime_create_scene;
    runtime_interface.create_memory = vrms_runtime_create_memory;
    runtime_interface.create_object_data = vrms_runtime_create_object_data;
    runtime_interface.create_object_texture = vrms_runtime_create_object_texture;
    runtime_interface.attach_memory = vrms_runtime_attach_memory;
    runtime_interface.run_program = vrms_runtime_run_program;
    runtime_interface.set_skybox = vrms_runtime_set_skybox;
    runtime_interface.destroy_scene = vrms_runtime_destroy_scene;
    runtime_interface.destroy_object = vrms_runtime_destroy_object;
    runtime_interface.update_system_matrix = vrms_runtime_update_system_matrix;
    runtime_interface.update_system_matrix_module = vrms_runtime_update_system_matrix_module;
}

uint8_t assert_vrms_server(vrms_runtime_t* vrms_runtime) {
    if (!vrms_runtime) {
        fprintf(stderr, "invalid runtime object\n");
        return 0;
    }
    if (!vrms_runtime->vrms_server) {
        fprintf(stderr, "invalid server object\n");
        return 0;
    }
    return 1;
}

uint32_t vrms_runtime_create_scene(vrms_runtime_t* vrms_runtime, char* name) {
    if (!assert_vrms_server(vrms_runtime)) {
        return 0;
    }
    return vrms_server_create_scene(vrms_runtime->vrms_server, name);
}

uint32_t vrms_runtime_create_memory(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t fd, uint32_t size) {
    if (!assert_vrms_server(vrms_runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_create_memory(vrms_scene, fd, size);
}

uint32_t vrms_runtime_create_object_data(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, vrms_data_type_t type) {
    if (!assert_vrms_server(vrms_runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_runtime->vrms_server, scene_id);
    if (!vrms_scene)
        return 0;
    return vrms_scene_create_object_data(vrms_scene, memory_id, memory_offset, memory_length, type);
}

uint32_t vrms_runtime_create_object_texture(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type) {
    if (!assert_vrms_server(vrms_runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_create_object_texture(vrms_scene, data_id, width, height, format, type);
}

uint32_t vrms_runtime_attach_memory(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t data_id) {
    if (!assert_vrms_server(vrms_runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_attach_memory(vrms_scene, data_id);
}

uint32_t vrms_runtime_run_program(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t program_id, uint32_t register_id) {
    if (!assert_vrms_server(vrms_runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_run_program(vrms_scene, program_id, register_id);
}


uint32_t vrms_runtime_update_system_matrix_module(vrms_runtime_t* vrms_runtime, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix) {
    vrms_server_queue_update_system_matrix(vrms_runtime->vrms_server, matrix_type, update_type, (uint8_t*)matrix);
    return 1;
}

uint32_t vrms_runtime_update_system_matrix(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t data_id, uint32_t data_index, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type) {
    if (!assert_vrms_server(vrms_runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_update_system_matrix(vrms_scene, data_id, data_index, matrix_type, update_type);
}

uint32_t vrms_runtime_set_skybox(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t texture_id) {
    if (!assert_vrms_server(vrms_runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_set_skybox(vrms_scene, texture_id);
}

uint32_t vrms_runtime_destroy_scene(vrms_runtime_t* vrms_runtime, uint32_t scene_id) {
    if (!assert_vrms_server(vrms_runtime)) {
        return 0;
    }

    vrms_server_destroy_scene(vrms_runtime->vrms_server, scene_id);
    return 1;
}

uint32_t vrms_runtime_destroy_object(vrms_runtime_t* vrms_runtime, uint32_t scene_id, uint32_t object_id) {
    if (!assert_vrms_server(vrms_runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    vrms_scene_destroy_object(vrms_scene, object_id);

    return 1;
}

void run_module(vrms_runtime_t* vrms_runtime, char* module_name) {
    void* (*run_module)(void*);
    void* handle = NULL;
    char* full_module_path;
    uint32_t full_module_path_len;
    uint32_t module_name_len;
    uint32_t module_load_path_len;
    const char* error_message = NULL;

    module_name_len = strlen(module_name);
    module_load_path_len = strlen(vrms_runtime->module_load_path);
    full_module_path_len = (module_name_len + module_load_path_len + 2) * sizeof(char);

    full_module_path = SAFEMALLOC(full_module_path_len);
    memset(full_module_path, 0, full_module_path_len);

    memcpy(full_module_path, vrms_runtime->module_load_path, module_load_path_len);
    memcpy(full_module_path + module_load_path_len, "/", 1);
    memcpy(full_module_path + module_load_path_len + 1, module_name, module_name_len);
    fprintf(stderr, "run_module(): loading '%s'\n", full_module_path);

    handle = dlopen(full_module_path, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "unable to load module %s\n", dlerror());
        return;
    }
    dlerror();

    run_module = (void*(*)())dlsym(handle, "run_module");
    error_message = dlerror();
    if (error_message) {
        fprintf(stderr, "unable to find run_module function: %s\n", error_message);
        dlclose(handle);
    }

    (*run_module)(vrms_runtime);

    dlclose(handle);

    return;
}

void* start_module_thread(void* data) {
    vrms_runtime_module_thread_t* module_thread;

    module_thread = (vrms_runtime_module_thread_t*)data;
    run_module(module_thread->vrms_runtime, module_thread->module_name);

    return NULL;
}

vrms_runtime_module_thread_t* vrms_runtime_module_thread_create(char* module_name) {
    vrms_runtime_module_thread_t* module_thread;

   module_thread = SAFEMALLOC(sizeof(vrms_runtime_module_thread_t));
   module_thread->module_name = SAFEMALLOC(strlen(module_name) + 1);
   strcpy(module_thread->module_name, module_name);

   return module_thread;
}

void vrms_runtime_module_thread_destroy(vrms_runtime_module_thread_t* module_thread) {
    if (!module_thread) {
        return;
    }
    if (module_thread->module_name) {
        free(module_thread->module_name);
    }
    free(module_thread);
}

void vrms_runtime_load_modules(vrms_runtime_t* vrms_runtime) {
    DIR* dir = opendir(vrms_runtime->module_load_path);
    if (!dir) {
        fprintf(stderr, "could not open module dir: %s\n", vrms_runtime->module_load_path);
        return;
    }

    vrms_runtime_module_thread_t* module_thread;
    int32_t thread_ret;
    struct dirent* entry;
    uint8_t index = 0;
    while ((entry = readdir(dir))) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }
        char* ext = strrchr(entry->d_name, '.');
        if (!ext) {
            continue;
        }
        if (strcmp(ext + 1, "so")) {
            continue;
        }

        module_thread = vrms_runtime_module_thread_create(entry->d_name);
        module_thread->vrms_runtime = vrms_runtime;
        thread_ret = pthread_create(&module_thread->pthread, NULL, start_module_thread, module_thread);
        if (thread_ret != 0) {
            vrms_runtime_module_thread_destroy(module_thread);
            free(module_thread);
            fprintf(stderr, "unable to start thread for module\n");
            exit(1);
        }
        vrms_runtime->module_threads[index] = module_thread;
        index++;
    }
    vrms_runtime->nr_module_threads = index;
    closedir(dir);
}

void draw_scene(opengl_stereo* ostereo, void* data) {
    if (NULL != data) {
        vrms_server_draw_scenes((vrms_server_t*)data, ostereo->projection_matrix, ostereo->view_matrix, ostereo->model_matrix, ostereo->skybox_camera->projection_matrix);
    }
}

void vrms_runtime_system_matrix_update(vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix) {
    memcpy(ostereo->hmd_matrix, matrix, sizeof(float) * 16);
}

vrms_runtime_t* vrms_runtime_init(int width, int height, double physical_width) {
    vrms_runtime_t* vrms_runtime = malloc(sizeof(vrms_runtime_t));
    memset(vrms_runtime, 0, sizeof(vrms_runtime_t));

    fill_interface();
    vrms_runtime->interface = &runtime_interface;

    vrms_runtime->w = width;
    vrms_runtime->h = height;

    vrms_runtime->module_load_path = "/home/ceade/src/personal/github/vroom/module";
    vrms_server_t* vrms_server = vrms_server_create();
    vrms_runtime->vrms_server = vrms_server;

    ostereo = opengl_stereo_create(width, height, physical_width);
    opengl_stereo_draw_scene_callback(ostereo, draw_scene, vrms_server);

    vrms_server->color_shader_id = ostereo->color_shader_id;
    vrms_server->texture_shader_id = ostereo->texture_shader_id;
    vrms_server->cubemap_shader_id = ostereo->cubemap_shader_id;
    vrms_server->system_matrix_update = vrms_runtime_system_matrix_update;

    vrms_runtime_load_modules(vrms_runtime);

    return vrms_runtime;
}

void vrms_runtime_display(vrms_runtime_t* vrms_runtime) {
    opengl_stereo_display(ostereo);
}

void vrms_runtime_reshape(vrms_runtime_t* vrms_runtime, int w, int h) {
    vrms_runtime->w = w;
    vrms_runtime->h = h;
    opengl_stereo_reshape(ostereo, w, h);
}

void vrms_runtime_process(vrms_runtime_t* vrms_runtime) {
    vrms_server_process_queue(vrms_runtime->vrms_server);
}

void vrms_runtime_end(vrms_runtime_t* vrms_runtime) {
    uint8_t index;
    vrms_runtime_module_thread_t* module_thread;

    index = 0;
    for (index = 0; index < vrms_runtime->nr_module_threads; index++) {
        module_thread = vrms_runtime->module_threads[index];
        if (!module_thread) {
            continue;
        }
        // pthread_join(module_thread->pthread);
    }
}
