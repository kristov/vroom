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

typedef enum vrms_queue_item_type {
    VRMS_QUEUE_DATA_LOAD,
    VRMS_QUEUE_TEXTURE_LOAD,
    VRMS_QUEUE_UPDATE_SYSTEM_MATRIX,
    VRMS_QUEUE_EVENT
} vrms_queue_item_type_t;

typedef struct vrms_queue_item_data_load {
    vrms_data_type_t type;
    uint32_t* destination;
    uint8_t* buffer;
    uint32_t size;
} vrms_queue_item_data_load_t;

typedef struct vrms_queue_item_update_system_matrix {
    vrms_matrix_type_t matrix_type;
    vrms_update_type_t update_type;
    uint8_t* buffer;
} vrms_queue_item_update_system_matrix_t;

typedef struct vrms_queue_item_texture_load {
    uint32_t* destination;
    uint8_t* buffer;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    vrms_texture_format_t format;
    vrms_texture_type_t type;
} vrms_queue_item_texture_load_t;

typedef struct vrms_queue_item_event {
    char* data;
} vrms_queue_item_event_t;

typedef struct vrms_queue_item {
    vrms_queue_item_type_t type;
    union {
        vrms_queue_item_data_load_t* data_load;
        vrms_queue_item_texture_load_t* texture_load;
        vrms_queue_item_update_system_matrix_t* update_system_matrix;
        vrms_queue_item_event_t* event;
    } item;
} vrms_queue_item_t;

vrms_server_t* vrms_server_create() {
    vrms_server_t* server = SAFEMALLOC(sizeof(vrms_server_t));
    //vrms_server_t* server = SAFEMALLOC(-1UL);
    memset(server, 0, sizeof(vrms_server_t));

    server->scenes = SAFEMALLOC(sizeof(vrms_scene_t) * 10);
    memset(server->scenes, 0, sizeof(vrms_scene_t) * 10);
    server->next_scene_id = 1;

    server->inbound_queue = SAFEMALLOC(sizeof(vrms_queue_item_t) * 10);
    memset(server->inbound_queue, 0, sizeof(vrms_queue_item_t) * 10);
    server->inbound_queue_index = 0;
    server->inbound_queue_lock = SAFEMALLOC(sizeof(pthread_mutex_t));
    memset(server->inbound_queue_lock, 0, sizeof(pthread_mutex_t));

    mat4_identity(server->head_matrix);
    mat4_identity(server->body_matrix);

    return server;
}

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

uint32_t vrms_server_queue_add_data_load(vrms_server_t* server, uint32_t size, uint32_t* gl_id_ref, vrms_data_type_t type, uint8_t* buffer) {
    vrms_queue_item_data_load_t* data_load = SAFEMALLOC(sizeof(vrms_queue_item_data_load_t));
    memset(data_load, 0, sizeof(vrms_queue_item_data_load_t));

    data_load->size = size;
    data_load->destination = gl_id_ref;
    data_load->type = type;
    data_load->buffer = buffer;

    vrms_queue_item_t* queue_item = SAFEMALLOC(sizeof(vrms_queue_item_t));
    memset(queue_item, 0, sizeof(vrms_queue_item_t));
    queue_item->type = VRMS_QUEUE_DATA_LOAD;

    queue_item->item.data_load = data_load;

    pthread_mutex_lock(server->inbound_queue_lock);
    uint32_t idx = server->inbound_queue_index;
    server->inbound_queue[server->inbound_queue_index] = queue_item;
    server->inbound_queue_index++;
    pthread_mutex_unlock(server->inbound_queue_lock);

    return idx;
}

uint32_t vrms_server_queue_add_texture_load(vrms_server_t* server, uint32_t size, uint32_t* gl_id_ref, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type, uint8_t* buffer) {
    vrms_queue_item_texture_load_t* texture_load = SAFEMALLOC(sizeof(vrms_queue_item_texture_load_t));
    memset(texture_load, 0, sizeof(vrms_queue_item_texture_load_t));

    texture_load->size = size;
    texture_load->destination = gl_id_ref;
    texture_load->width = width;
    texture_load->height = height;
    texture_load->format = format;
    texture_load->type = type;
    texture_load->buffer = buffer;

    vrms_queue_item_t* queue_item = SAFEMALLOC(sizeof(vrms_queue_item_t));
    memset(queue_item, 0, sizeof(vrms_queue_item_t));
    queue_item->type = VRMS_QUEUE_TEXTURE_LOAD;

    queue_item->item.texture_load = texture_load;

    pthread_mutex_lock(server->inbound_queue_lock);
    uint32_t idx = server->inbound_queue_index;
    server->inbound_queue[server->inbound_queue_index] = queue_item;
    server->inbound_queue_index++;
    pthread_mutex_unlock(server->inbound_queue_lock);

    return idx;
}

void vrms_server_queue_add_matrix_load(vrms_server_t* server, uint32_t size, uint32_t* gl_id_ref, vrms_data_type_t type, void* buffer) {
    vrms_queue_item_data_load_t* data_load = SAFEMALLOC(sizeof(vrms_queue_item_data_load_t));
    memset(data_load, 0, sizeof(vrms_queue_item_data_load_t));

    data_load->size = size;
    data_load->destination = gl_id_ref;
    data_load->type = type;
    data_load->buffer = buffer;

    vrms_queue_item_t* queue_item = SAFEMALLOC(sizeof(vrms_queue_item_t));
    memset(queue_item, 0, sizeof(vrms_queue_item_t));
    queue_item->type = VRMS_QUEUE_DATA_LOAD;

    queue_item->item.data_load = data_load;

    pthread_mutex_lock(server->inbound_queue_lock);
    server->inbound_queue[server->inbound_queue_index] = queue_item;
    server->inbound_queue_index++;
    pthread_mutex_unlock(server->inbound_queue_lock);
}

void vrms_server_queue_update_system_matrix(vrms_server_t* server, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, uint8_t* buffer) {
    vrms_queue_item_update_system_matrix_t* update_system_matrix = SAFEMALLOC(sizeof(vrms_queue_item_update_system_matrix_t));
    memset(update_system_matrix, 0, sizeof(vrms_queue_item_update_system_matrix_t));

    update_system_matrix->matrix_type = matrix_type;
    update_system_matrix->update_type = update_type;
    update_system_matrix->buffer = buffer;

    vrms_queue_item_t* queue_item = SAFEMALLOC(sizeof(vrms_queue_item_t));
    memset(queue_item, 0, sizeof(vrms_queue_item_t));
    queue_item->type = VRMS_QUEUE_UPDATE_SYSTEM_MATRIX;

    queue_item->item.update_system_matrix = update_system_matrix;

    pthread_mutex_lock(server->inbound_queue_lock);
    server->inbound_queue[server->inbound_queue_index] = queue_item;
    server->inbound_queue_index++;
    pthread_mutex_unlock(server->inbound_queue_lock);
}

/*
void vrms_server_draw_skybox(vrms_server_t* server, vrms_object_skybox_t* skybox, float projection_matrix[16], float view_matrix[16], float model_matrix[16]) {
    float mvp_matrix[16];
    vrms_gl_render_t render;
    vrms_gl_matrix_t matrix;

    mat4_copy(mvp_matrix, projection_matrix);
    mat4_multiply(mvp_matrix, view_matrix);

    matrix.mvp = mvp_matrix;

    render.shader_id = server->cubemap_shader_id;
    render.vertex_id = skybox->vertex_gl_id;
    render.texture_id = skybox->texture_gl_id;
    render.index_id = skybox->index_gl_id;

    vrms_gl_draw_skybox(render, matrix);
}
*/

void vrms_server_draw_scenes(vrms_server_t* server, float projection_matrix[16], float view_matrix[16], float model_matrix[16], float skybox_projection_matrix[16]) {
    int si;//, oi;
    vrms_scene_t* scene;
    uint32_t usec_elapsed;
    uint8_t i;

    // TODO: Dont loop through scenes like this, but do round robin on the VM calls
    // across the scenes, so its not that one scene gets to draw everything, and
    // other scenes starve. Sounds a bit like a task scheduler.
    usec_elapsed = 0;
    for (si = 1; si < server->next_scene_id; si++) {
        mat4_identity(model_matrix);
        scene = server->scenes[si];
        if (NULL != scene) {
            usec_elapsed += vrms_scene_draw(scene, projection_matrix, view_matrix, model_matrix, skybox_projection_matrix);
        }
        if (si >= 2000) break;
    }

    // Maintain a list of NR_RENDER_AVG render times for calculating an average
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
    switch (queue_item->type) {
        vrms_queue_item_data_load_t* data_load;
        vrms_queue_item_texture_load_t* texture_load;
        vrms_queue_item_update_system_matrix_t* update_system_matrix;
        case VRMS_QUEUE_DATA_LOAD:
            data_load = queue_item->item.data_load;
            if (!data_load->buffer) {
                return;
            }
            vrms_gl_load_buffer(data_load->buffer, data_load->destination, data_load->size, data_load->type);
            free(data_load);
            break;
        case VRMS_QUEUE_TEXTURE_LOAD:
            texture_load = queue_item->item.texture_load;
            if (!texture_load->buffer) {
                return;
            }
            vrms_gl_load_texture_buffer(texture_load->buffer, texture_load->destination, texture_load->width, texture_load->height, texture_load->format, texture_load->type);
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
            debug_print("unknown queue type!!\n");
    }

    free(queue_item);
}

void vrms_scene_queue_item_flush(vrms_server_t* server) {
    uint32_t idx;
    for (idx = 0; idx < server->inbound_queue_index; idx++) {
        vrms_queue_item_t* queue_item = server->inbound_queue[idx];
        if (NULL != queue_item) {
            vrms_server_queue_item_process(server, queue_item);
        }
        else {
            debug_print("null queue item\n");
        }
    }
    server->inbound_queue_index = 0;
}

void vrms_server_flush_client_inbound_queue(vrms_server_t* server) {
    if (!pthread_mutex_trylock(server->inbound_queue_lock)) {
        vrms_scene_queue_item_flush(server);
        pthread_mutex_unlock(server->inbound_queue_lock);
    }
    else {
        debug_print("socket thread has lock on queue\n");
    }
}

void vrms_server_process_queue(vrms_server_t* server) {
    vrms_server_flush_client_inbound_queue(server);
}

