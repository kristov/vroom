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
#include "object.h"
#include "scene.h"
#include "server.h"
#include "gl-matrix.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id) {
    vrms_scene_t* scene;
    if (scene_id >= vrms_server->next_scene_id) {
        debug_print("invalid scene_id [%d] requested: id out of range\n", scene_id);
        return NULL;
    }
    scene = vrms_server->scenes[scene_id];
    if (!scene) {
        debug_print("invalid scene_id [%d] requested: scene does not exist\n", scene_id);
        return NULL;
    }
    return scene;
}

uint32_t vrms_server_create_scene(vrms_server_t* server, char* name) {
    vrms_scene_t* scene = vrms_scene_create(name);
    scene->server = server;

    server->scenes[server->next_scene_id] = scene;
    scene->id = server->next_scene_id;
    server->next_scene_id++;

    return scene->id;
}

uint32_t vrms_server_destroy_scene(vrms_server_t* server, uint32_t scene_id) {
    vrms_scene_t* scene = vrms_server_get_scene(server, scene_id);
    if (!scene) {
        debug_print("no scene found\n");
        return 0;
    }

    server->scenes[scene_id] = NULL;
    vrms_scene_destroy(scene);
    return 1;
}

uint32_t vrms_server_queue_add_data_load(vrms_server_t* server, uint32_t size, uint32_t scene_id, uint32_t object_id, vrms_data_type_t type, uint8_t* buffer) {
    vrms_queue_item_data_load_t* data_load = SAFEMALLOC(sizeof(vrms_queue_item_data_load_t));
    memset(data_load, 0, sizeof(vrms_queue_item_data_load_t));

    data_load->size = size;
    data_load->scene_id = scene_id;
    data_load->object_id = object_id;
    data_load->type = type;
    data_load->buffer = buffer;

    pthread_mutex_lock(&server->inbound_queue_lock);
    uint32_t idx = server->inbound_queue_index;
    vrms_queue_item_t* queue_item = &server->inbound_queue[idx];
    server->inbound_queue_index++;
    pthread_mutex_unlock(&server->inbound_queue_lock);

    queue_item->type = VRMS_QUEUE_DATA_LOAD;
    queue_item->item.data_load = data_load;

    return idx;
}

uint32_t vrms_server_queue_add_texture_load(vrms_server_t* server, uint32_t size, uint32_t scene_id, uint32_t object_id, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type, uint8_t* buffer) {
    vrms_queue_item_texture_load_t* texture_load = SAFEMALLOC(sizeof(vrms_queue_item_texture_load_t));
    memset(texture_load, 0, sizeof(vrms_queue_item_texture_load_t));

    texture_load->size = size;
    texture_load->scene_id = scene_id;
    texture_load->object_id = object_id;
    texture_load->width = width;
    texture_load->height = height;
    texture_load->format = format;
    texture_load->type = type;
    texture_load->buffer = buffer;

    pthread_mutex_lock(&server->inbound_queue_lock);
    uint32_t idx = server->inbound_queue_index;
    vrms_queue_item_t* queue_item = &server->inbound_queue[idx];
    server->inbound_queue_index++;
    pthread_mutex_unlock(&server->inbound_queue_lock);

    queue_item->type = VRMS_QUEUE_TEXTURE_LOAD;
    queue_item->item.texture_load = texture_load;

    return idx;
}

void vrms_server_queue_update_system_matrix(vrms_server_t* server, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, uint8_t* buffer) {
    vrms_queue_item_update_system_matrix_t* update_system_matrix = SAFEMALLOC(sizeof(vrms_queue_item_update_system_matrix_t));
    memset(update_system_matrix, 0, sizeof(vrms_queue_item_update_system_matrix_t));

    update_system_matrix->matrix_type = matrix_type;
    update_system_matrix->update_type = update_type;
    update_system_matrix->buffer = buffer;

    pthread_mutex_lock(&server->inbound_queue_lock);
    uint32_t idx = server->inbound_queue_index;
    vrms_queue_item_t* queue_item = &server->inbound_queue[idx];
    server->inbound_queue_index++;
    pthread_mutex_unlock(&server->inbound_queue_lock);

    queue_item->type = VRMS_QUEUE_UPDATE_SYSTEM_MATRIX;
    queue_item->item.update_system_matrix = update_system_matrix;
}

void vrms_server_draw_skybox(vrms_server_t* server, float view_matrix[16], float projection_matrix[16]) {
    vrms_gl_render_t render;
    vrms_gl_matrix_t matrix;

    mat4_copy(matrix.mvp, projection_matrix);
    mat4_multiply(matrix.mvp, view_matrix);

    render.shader_id = server->skybox.shader_id;
    render.vertex_id = server->skybox.vertex_gl_id;
    render.texture_id = server->skybox.texture_gl_id;
    render.index_id = server->skybox.index_gl_id;

    vrms_gl_draw_skybox(render, matrix);
}

void vrms_server_draw_scenes(vrms_server_t* server, float projection_matrix[16], float view_matrix[16], float model_matrix[16], float skybox_projection_matrix[16]) {
    vrms_scene_t* scene;

    if (server->skybox.texture_gl_id) {
        vrms_server_draw_skybox(server, view_matrix, skybox_projection_matrix);
    }

    // TODO: Dont loop through scenes like this, but do round robin on the VM calls
    // across the scenes, so its not that one scene gets to draw everything, and
    // other scenes starve. Sounds a bit like a task scheduler.
    uint32_t si = 0;
    uint32_t usec_elapsed = 0;
    for (si = 1; si < server->next_scene_id; si++) {
        mat4_identity(model_matrix);
        scene = server->scenes[si];
        if (NULL != scene) {
            usec_elapsed += vrms_scene_draw(scene, projection_matrix, view_matrix, model_matrix, skybox_projection_matrix);
        }
        if (si >= 2000) break;
    }

    // Maintain a list of NR_RENDER_AVG render times for calculating an average
    uint8_t i = 0;
    for (i = NR_RENDER_AVG; i > 0; i--) {
        server->render_usecs[i] = server->render_usecs[i - 1];
        //fprintf(stderr, "%d ", server->render_usecs[i - 1]);
    }
    //fprintf(stderr, "%d\n", usec_elapsed);
    server->render_usecs[0] = usec_elapsed;
}

void vrms_queue_update_system_matrix(vrms_server_t* server, vrms_queue_item_update_system_matrix_t* update_system_matrix) {
    float* matrix;

    matrix = (float*)update_system_matrix->buffer;
    if (NULL != server->system_matrix_update) {
        server->system_matrix_update(update_system_matrix->matrix_type, update_system_matrix->update_type, matrix);
    }
}

void vrms_server_queue_item_process(vrms_server_t* server, vrms_queue_item_t* queue_item) {
    uint32_t gl_id;
    vrms_scene_t* scene;
    switch (queue_item->type) {
        vrms_queue_item_data_load_t* data_load;
        vrms_queue_item_texture_load_t* texture_load;
        vrms_queue_item_update_system_matrix_t* update_system_matrix;
        case VRMS_QUEUE_DATA_LOAD:
            data_load = queue_item->item.data_load;
            if (!data_load->buffer) {
                return;
            }
            vrms_gl_load_buffer(data_load->buffer, &gl_id, data_load->size, data_load->type);
            scene = vrms_server_get_scene(server, data_load->scene_id);
            if (scene) {
                vrms_scene_queue_add_gl_loaded(scene, VRMS_OBJECT_DATA, data_load->object_id, gl_id);
            }
            free(data_load);
            break;
        case VRMS_QUEUE_TEXTURE_LOAD:
            texture_load = queue_item->item.texture_load;
            if (!texture_load->buffer) {
                return;
            }
            vrms_gl_load_texture_buffer(texture_load->buffer, &gl_id, texture_load->width, texture_load->height, texture_load->format, texture_load->type);
            scene = vrms_server_get_scene(server, texture_load->scene_id);
            if (scene) {
                vrms_scene_queue_add_gl_loaded(scene, VRMS_OBJECT_TEXTURE, texture_load->object_id, gl_id);
            }
            free(texture_load);
            break;
        case VRMS_QUEUE_UPDATE_SYSTEM_MATRIX:
            update_system_matrix = queue_item->item.update_system_matrix;
            vrms_queue_update_system_matrix(server, update_system_matrix);
            free(update_system_matrix);
            break;
        case VRMS_QUEUE_EVENT:
            debug_print("not supposed to get a VRMS_QUEUE_EVENT from a client\n");
            break;
        default:
            debug_print("vrms_server_queue_item_process(): unknown type!!\n");
    }
}

void vrms_server_process_queue(vrms_server_t* server) {
    if (!pthread_mutex_trylock(&server->inbound_queue_lock)) {
        uint8_t idx;
        for (idx = 0; idx < server->inbound_queue_index; idx++) {
            vrms_server_queue_item_process(server, &server->inbound_queue[idx]);
        }
        server->inbound_queue_index = 0;
        pthread_mutex_unlock(&server->inbound_queue_lock);
    }
    else {
        debug_print("vrms_server_process_queue(): lock on queue\n");
    }
}

void vrms_server_setup_skybox(vrms_server_t* server) {
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

    vrms_gl_load_buffer((uint8_t*)vertex_data, &server->skybox.vertex_gl_id, (24 * sizeof(float)), VRMS_VERTEX);
    vrms_gl_load_buffer((uint8_t*)index_data, &server->skybox.index_gl_id, (36 * sizeof(uint16_t)), VRMS_INDEX);

    server->skybox.shader_id = server->cubemap_shader_id;
}

vrms_server_t* vrms_server_create() {
    vrms_server_t* server = SAFEMALLOC(sizeof(vrms_server_t));
    //vrms_server_t* server = SAFEMALLOC(-1UL);
    memset(server, 0, sizeof(vrms_server_t));

    server->scenes = SAFEMALLOC(sizeof(vrms_scene_t) * 10);
    memset(server->scenes, 0, sizeof(vrms_scene_t) * 10);
    server->next_scene_id = 1;

    server->inbound_queue_index = 0;

    mat4_identity(server->head_matrix);
    mat4_identity(server->body_matrix);

    vrms_server_setup_skybox(server);

    return server;
}
