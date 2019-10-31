#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include "gl.h"
#include "safemalloc.h"
#include "object.h"
#include "server.h"
#include "scene.h"
#include "opengl_stereo.h"
#include "vroom.h"
#include "runtime.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

opengl_stereo ostereo;

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

int vrms_module_debug(vrms_module_t* module, const char *format, ...) {
    va_list arg;
    int done;

    fprintf(stderr, "M|DEBUG|%s|", module->name);
    va_start(arg, format);
    done = vfprintf(stderr, format, arg);
    va_end(arg);
    fprintf(stderr, "\n");

    return done;
}

int vrms_module_error(vrms_module_t* module, const char *format, ...) {
    va_list arg;
    int done;

    fprintf(stderr, "M|ERROR|%s|", module->name);
    va_start(arg, format);
    done = vfprintf(stderr, format, arg);
    va_end(arg);
    fprintf(stderr, "\n");

    return done;
}

uint32_t vrms_module_create_scene(vrms_module_t* module, char* name) {
    if (!assert_vrms_server(module->runtime)) {
        return 0;
    }
    return vrms_server_create_scene(module->runtime->vrms_server, name);
}

uint32_t vrms_module_create_memory(vrms_module_t* module, uint32_t scene_id, uint32_t fd, uint32_t size) {
    if (!assert_vrms_server(module->runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(module->runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_create_memory(vrms_scene, fd, size);
}

uint32_t vrms_module_create_object_data(vrms_module_t* module, uint32_t scene_id, uint32_t memory_id, uint32_t memory_offset, uint32_t memory_length, vrms_data_type_t type) {
    if (!assert_vrms_server(module->runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(module->runtime->vrms_server, scene_id);
    if (!vrms_scene)
        return 0;
    return vrms_scene_create_object_data(vrms_scene, memory_id, memory_offset, memory_length, type);
}

uint32_t vrms_module_create_object_texture(vrms_module_t* module, uint32_t scene_id, uint32_t data_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type) {
    if (!assert_vrms_server(module->runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(module->runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_create_object_texture(vrms_scene, data_id, width, height, format, type);
}

uint32_t vrms_module_attach_memory(vrms_module_t* module, uint32_t scene_id, uint32_t data_id) {
    if (!assert_vrms_server(module->runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(module->runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_attach_memory(vrms_scene, data_id);
}

uint32_t vrms_module_run_program(vrms_module_t* module, uint32_t scene_id, uint32_t program_id, uint32_t register_id) {
    if (!assert_vrms_server(module->runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(module->runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_run_program(vrms_scene, program_id, register_id);
}


uint32_t vrms_module_update_system_matrix(vrms_module_t* module, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix) {
    if (!assert_vrms_server(module->runtime)) {
        return 0;
    }

    vrms_server_queue_update_system_matrix(module->runtime->vrms_server, matrix_type, update_type, (uint8_t*)matrix);
    return 1;
}

uint32_t vrms_module_set_skybox(vrms_module_t* module, uint32_t scene_id, uint32_t texture_id) {
    if (!assert_vrms_server(module->runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(module->runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    return vrms_scene_set_skybox(vrms_scene, texture_id);
}

uint32_t vrms_module_destroy_scene(vrms_module_t* module, uint32_t scene_id) {
    if (!assert_vrms_server(module->runtime)) {
        return 0;
    }

    vrms_server_destroy_scene(module->runtime->vrms_server, scene_id);
    return 1;
}

uint32_t vrms_module_destroy_object(vrms_module_t* module, uint32_t scene_id, uint32_t object_id) {
    if (!assert_vrms_server(module->runtime)) {
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(module->runtime->vrms_server, scene_id);
    if (!vrms_scene) {
        return 0;
    }
    vrms_scene_destroy_object(vrms_scene, object_id);

    return 1;
}

void run_module(vrms_module_t* module) {
    void* (*run_module)(void*);
    void* handle = NULL;
    char* full_module_path;
    uint32_t full_module_path_len;
    uint32_t module_name_len;
    uint32_t module_load_path_len;
    const char* error_message = NULL;

    module_name_len = strlen(module->name);
    module_load_path_len = strlen(module->runtime->module_load_path);
    full_module_path_len = (module_name_len + module_load_path_len + 2) * sizeof(char);

    full_module_path = SAFEMALLOC(full_module_path_len);
    memset(full_module_path, 0, full_module_path_len);

    memcpy(full_module_path, module->runtime->module_load_path, module_load_path_len);
    memcpy(full_module_path + module_load_path_len, "/", 1);
    memcpy(full_module_path + module_load_path_len + 1, module->name, module_name_len);
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

    (*run_module)(module);

    dlclose(handle);

    return;
}

void* start_module_thread(void* data) {
    vrms_module_t* module;

    module = (vrms_module_t*)data;
    run_module(module);

    return NULL;
}

vrms_module_t* vrms_runtime_module_create(char* module_name) {
    vrms_module_t* module = SAFEMALLOC(sizeof(vrms_module_t));
    module->name = SAFEMALLOC(strlen(module_name) + 1);
    strcpy(module->name, module_name);

    module->interface.debug = vrms_module_debug;
    module->interface.error = vrms_module_error;
    module->interface.create_scene = vrms_module_create_scene;
    module->interface.create_memory = vrms_module_create_memory;
    module->interface.create_object_data = vrms_module_create_object_data;
    module->interface.create_object_texture = vrms_module_create_object_texture;
    module->interface.attach_memory = vrms_module_attach_memory;
    module->interface.run_program = vrms_module_run_program;
    module->interface.set_skybox = vrms_module_set_skybox;
    module->interface.destroy_scene = vrms_module_destroy_scene;
    module->interface.destroy_object = vrms_module_destroy_object;
    module->interface.update_system_matrix = vrms_module_update_system_matrix;

    return module;
}

void vrms_module_destroy(vrms_module_t* module) {
    if (!module) {
        return;
    }
    if (module->name) {
        free(module->name);
    }
    free(module);
}

void vrms_runtime_load_modules(vrms_runtime_t* vrms_runtime) {
    DIR* dir = opendir(vrms_runtime->module_load_path);
    if (!dir) {
        fprintf(stderr, "could not open module dir: %s\n", vrms_runtime->module_load_path);
        return;
    }

    vrms_module_t* module;
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

        module = vrms_runtime_module_create(entry->d_name);
        module->runtime = vrms_runtime;
        thread_ret = pthread_create(&module->pthread, NULL, start_module_thread, module);
        if (thread_ret != 0) {
            vrms_module_destroy(module);
            free(module);
            fprintf(stderr, "unable to start thread for module\n");
            exit(1);
        }
        vrms_runtime->modules[index] = module;
        index++;
    }
    vrms_runtime->nr_modules = index;
    closedir(dir);
}

void draw_scene(opengl_stereo* ostereo, void* data) {
    if (NULL != data) {
        vrms_server_draw_scenes((vrms_server_t*)data, ostereo->projection_matrix, ostereo->view_matrix, ostereo->model_matrix, ostereo->skybox_camera.projection_matrix);
    }
}

uint32_t vrms_runtime_update_system_matrix(vrms_runtime_t* vrms_runtime, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix) {
    vrms_server_queue_update_system_matrix(vrms_runtime->vrms_server, matrix_type, update_type, (uint8_t*)matrix);
    return 1;
}

void vrms_runtime_system_matrix_update(vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix) {
    memcpy(ostereo.hmd_matrix, matrix, sizeof(float) * 16);
}

vrms_runtime_t* vrms_runtime_init(int width, int height, double physical_width) {
    vrms_runtime_t* vrms_runtime = malloc(sizeof(vrms_runtime_t));
    memset(vrms_runtime, 0, sizeof(vrms_runtime_t));

    vrms_runtime->w = width;
    vrms_runtime->h = height;

    vrms_runtime->module_load_path = "/home/ceade/src/personal/github/vroom/module";
    vrms_server_t* vrms_server = vrms_server_create();
    vrms_runtime->vrms_server = vrms_server;

    opengl_stereo_init(&ostereo, width, height, physical_width, OSTEREO_MODE_STEREO);
    opengl_stereo_draw_scene_callback(&ostereo, draw_scene, vrms_server);

    vrms_server->color_shader_id = ostereo.color_shader_id;
    vrms_server->texture_shader_id = ostereo.texture_shader_id;
    vrms_server->cubemap_shader_id = ostereo.cubemap_shader_id;
    vrms_server->system_matrix_update = vrms_runtime_system_matrix_update;

    vrms_runtime_load_modules(vrms_runtime);

    return vrms_runtime;
}

void vrms_runtime_display(vrms_runtime_t* vrms_runtime) {
    opengl_stereo_display(&ostereo);
}

void vrms_runtime_reshape(vrms_runtime_t* vrms_runtime, int w, int h) {
    vrms_runtime->w = w;
    vrms_runtime->h = h;
    opengl_stereo_reshape(&ostereo, w, h);
}

void vrms_runtime_process(vrms_runtime_t* vrms_runtime) {
    vrms_server_process_queue(vrms_runtime->vrms_server);
}

void vrms_runtime_end(vrms_runtime_t* vrms_runtime) {
    uint8_t index;
    vrms_module_t* module;

    index = 0;
    for (index = 0; index < vrms_runtime->nr_modules; index++) {
        module = vrms_runtime->modules[index];
        if (!module) {
            continue;
        }
        // pthread_join(module_thread->pthread);
    }
}
