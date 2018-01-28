#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#ifdef RASPBERRYPI
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else /* not RASPBERRYPI */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif /* RASPBERRYPI */

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
#include "vrms_server.h"
#include "esm.h"

typedef enum vrms_queue_item_type {
    VRMS_QUEUE_DATA_LOAD,
    VRMS_QUEUE_TEXTURE_LOAD,
    VRMS_QUEUE_SCENE_DESTROY,
    VRMS_QUEUE_EVENT
} vrms_queue_item_type_t;

typedef struct vrms_queue_item_scene_destroy {
    uint32_t scene_id;
} vrms_queue_item_scene_destroy_t;

typedef struct vrms_queue_item_data_load {
    vrms_data_type_t type;
    GLuint* destination;
    void* buffer;
    uint32_t size;
} vrms_queue_item_data_load_t;

typedef struct vrms_queue_item_texture_load {
    GLuint* destination;
    void* buffer;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    vrms_texture_format_t format;
} vrms_queue_item_texture_load_t;

typedef struct vrms_queue_item_event {
    char* data;
} vrms_queue_item_event_t;

typedef struct vrms_queue_item {
    vrms_queue_item_type_t type;
    union {
        vrms_queue_item_data_load_t* data_load;
        vrms_queue_item_texture_load_t* texture_load;
        vrms_queue_item_scene_destroy_t* scene_destroy;
        vrms_queue_item_event_t* event;
    } item;
} vrms_queue_item_t;

int VprintGlError(char *file, int line) {
    GLenum glErr;
    int retCode = 0;
    glErr = glGetError();
    switch (glErr) {
        case GL_INVALID_ENUM:
            printf("GL_INVALID_ENUM in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_INVALID_VALUE:
            printf("GL_INVALID_VALUE in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_INVALID_OPERATION:
            printf("GL_INVALID_OPERATION in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_STACK_OVERFLOW:
            printf("GL_STACK_OVERFLOW in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_STACK_UNDERFLOW:
            printf("GL_STACK_UNDERFLOW in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_OUT_OF_MEMORY:
            printf("GL_OUT_OF_MEMORY in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            printf("GL_INVALID_FRAMEBUFFER_OPERATION in file %s @ line %d: %d\n", file, line, glErr);
            retCode = 1;
        break;
    }
    return retCode;
}

#define printOpenGLError() VprintGlError(__FILE__, __LINE__)

vrms_server_t* vrms_server_create() {
    vrms_server_t* server = SAFEMALLOC(sizeof(vrms_server_t));
    //vrms_server_t* server = SAFEMALLOC(-1UL);

    server->scenes = SAFEMALLOC(sizeof(vrms_scene_t) * 10);
    memset(server->scenes, 0, sizeof(vrms_scene_t) * 10);
    server->next_scene_id = 1;

    server->inbound_queue = SAFEMALLOC(sizeof(vrms_queue_item_t) * 10);
    memset(server->inbound_queue, 0, sizeof(vrms_queue_item_t) * 10);
    server->inbound_queue_index = 0;
    server->inbound_queue_lock = SAFEMALLOC(sizeof(pthread_mutex_t));
    memset(server->inbound_queue_lock, 0, sizeof(pthread_mutex_t));

    return server;
}

vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id) {
    vrms_scene_t* scene;
    if (scene_id >= vrms_server->next_scene_id) {
        fprintf(stderr, "invalid scene_id [%d] requested: id out of range\n", scene_id);
        return NULL;
    }
    scene = vrms_server->scenes[scene_id];
    if (NULL == scene) {
        fprintf(stderr, "invalid scene_id [%d] requested: scene does not exist\n", scene_id);
        return NULL;
    }
    return scene;
}

uint32_t vrms_server_create_scene(vrms_server_t* server, char* name) {
    vrms_scene_t* scene = vrms_scene_create(name);
    scene->onecolor_shader_id = server->onecolor_shader_id;
    scene->texture_shader_id = server->texture_shader_id;

    scene->server = server;

    server->scenes[server->next_scene_id] = scene;
    scene->id = server->next_scene_id;
    server->next_scene_id++;

    return scene->id;
}

void vrms_server_destroy_scene(vrms_server_t* server, uint32_t scene_id) {
    vrms_scene_t* scene = vrms_server_get_scene(server, scene_id);
    if (NULL == scene) {
        fprintf(stderr, "request to destroy already destroyed scene\n");
        return;
    }

    server->scenes[scene_id] = NULL;

    vrms_scene_destroy(scene);
}

void vrms_server_queue_destroy_scene(vrms_server_t* server, uint32_t scene_id) {
    vrms_queue_item_scene_destroy_t* scene_destroy = SAFEMALLOC(sizeof(vrms_queue_item_scene_destroy_t));
    memset(scene_destroy, 0, sizeof(vrms_queue_item_scene_destroy_t));

    vrms_queue_item_t* queue_item = SAFEMALLOC(sizeof(vrms_queue_item_t));
    memset(queue_item, 0, sizeof(vrms_queue_item_t));
    queue_item->type = VRMS_QUEUE_SCENE_DESTROY;

    queue_item->item.scene_destroy = scene_destroy;

    pthread_mutex_lock(server->inbound_queue_lock);
    server->inbound_queue[server->inbound_queue_index] = queue_item;
    server->inbound_queue_index++;
    pthread_mutex_unlock(server->inbound_queue_lock);
}

void vrms_server_queue_add_data_load(vrms_server_t* server, uint32_t size, GLuint* gl_id_ref, vrms_data_type_t type, void* buffer) {
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

void vrms_server_queue_add_texture_load(vrms_server_t* server, uint32_t size, GLuint* gl_id_ref, uint32_t width, uint32_t height, vrms_texture_format_t format, void* buffer) {
    vrms_queue_item_texture_load_t* texture_load = SAFEMALLOC(sizeof(vrms_queue_item_texture_load_t));
    memset(texture_load, 0, sizeof(vrms_queue_item_texture_load_t));

    texture_load->size = size;
    texture_load->destination = gl_id_ref;
    texture_load->width = width;
    texture_load->height = height;
    texture_load->format = format;
    texture_load->buffer = buffer;

    vrms_queue_item_t* queue_item = SAFEMALLOC(sizeof(vrms_queue_item_t));
    memset(queue_item, 0, sizeof(vrms_queue_item_t));
    queue_item->type = VRMS_QUEUE_TEXTURE_LOAD;

    queue_item->item.texture_load = texture_load;

    pthread_mutex_lock(server->inbound_queue_lock);
    server->inbound_queue[server->inbound_queue_index] = queue_item;
    server->inbound_queue_index++;
    pthread_mutex_unlock(server->inbound_queue_lock);
}

void vrms_server_queue_add_matrix_load(vrms_server_t* server, uint32_t size, GLuint* gl_id_ref, vrms_data_type_t type, void* buffer) {
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

void vrms_server_draw_mesh_color(vrms_scene_t* scene, GLuint shader_id, vrms_object_mesh_color_t* mesh, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
    GLuint b_vertex, b_normal, u_color, m_mvp, m_mv;
    GLfloat* mvp_matrix;
    GLfloat* mv_matrix;

    vrms_object_t* object;
    vrms_object_geometry_t* geometry;
    vrms_object_data_t* vertex;
    vrms_object_data_t* normal;
    vrms_object_data_t* index;

    object = vrms_scene_get_object_by_id(scene, mesh->geometry_id);
    geometry = object->object.object_geometry;

    object = vrms_scene_get_object_by_id(scene, geometry->vertex_id);
    vertex = object->object.object_data;

    object = vrms_scene_get_object_by_id(scene, geometry->normal_id);
    normal = object->object.object_data;

    object = vrms_scene_get_object_by_id(scene, geometry->index_id);
    index = object->object.object_data;

    if ((0 == vertex->gl_id) || (0 == normal->gl_id) || (0 == index->gl_id)) {
        fprintf(stderr, "request to render unrealized geometry: V[%d] N[%d] I[%d]\n", vertex->gl_id, normal->gl_id, index->gl_id);
        return;
    }

    glUseProgram(shader_id);

    mv_matrix = esmCreateCopy(view_matrix);
    esmMultiply(mv_matrix, model_matrix);

    mvp_matrix = esmCreateCopy(projection_matrix);
    esmMultiply(mvp_matrix, view_matrix);
    esmMultiply(mvp_matrix, model_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, vertex->gl_id);
    b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);

    glBindBuffer(GL_ARRAY_BUFFER, normal->gl_id);
    b_normal = glGetAttribLocation(shader_id, "b_normal");
    glVertexAttribPointer(b_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_normal);

    u_color = glGetUniformLocation(shader_id, "u_color");
    glUniform4f(u_color, mesh->r, mesh->g, mesh->b, mesh->a);
    glEnableVertexAttribArray(u_color);

    m_mvp = glGetUniformLocation(shader_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, mvp_matrix);

    m_mv = glGetUniformLocation(shader_id, "m_mv");
    glUniformMatrix4fv(m_mv, 1, GL_FALSE, mv_matrix);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index->gl_id);
    glDrawElements(GL_TRIANGLES, index->nr_strides, GL_UNSIGNED_SHORT, NULL);

    esmDestroy(mvp_matrix);
    esmDestroy(mv_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void vrms_server_draw_mesh_texture(vrms_scene_t* scene, GLuint shader_id, vrms_object_mesh_texture_t* mesh, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {

    GLuint b_vertex, b_normal, b_uv, s_tex, m_mvp, m_mv;
    GLfloat* mvp_matrix;
    GLfloat* mv_matrix;

    vrms_object_t* object;
    vrms_object_geometry_t* geometry;
    vrms_object_data_t* vertex;
    vrms_object_data_t* normal;
    vrms_object_data_t* index;
    vrms_object_data_t* uv;
    vrms_object_texture_t* texture;

    object = vrms_scene_get_object_by_id(scene, mesh->geometry_id);
    geometry = object->object.object_geometry;

    object = vrms_scene_get_object_by_id(scene, geometry->vertex_id);
    vertex = object->object.object_data;

    object = vrms_scene_get_object_by_id(scene, geometry->normal_id);
    normal = object->object.object_data;

    object = vrms_scene_get_object_by_id(scene, geometry->index_id);
    index = object->object.object_data;

    object = vrms_scene_get_object_by_id(scene, mesh->uv_id);
    uv = object->object.object_data;

    object = vrms_scene_get_object_by_id(scene, mesh->texture_id);
    texture = object->object.object_texture;

    if ((0 == vertex->gl_id) || (0 == normal->gl_id) || (0 == index->gl_id) || (0 == uv->gl_id) || (0 == texture->gl_id)) {
        fprintf(stderr, "request to render unrealized geometry: V[%d] N[%d] I[%d] U[%d] T[%d]\n", vertex->gl_id, normal->gl_id, index->gl_id, uv->gl_id, texture->gl_id);
        return;
    }

    glUseProgram(shader_id);

    mv_matrix = esmCreateCopy(view_matrix);
    esmMultiply(mv_matrix, model_matrix);

    mvp_matrix = esmCreateCopy(projection_matrix);
    esmMultiply(mvp_matrix, view_matrix);
    esmMultiply(mvp_matrix, model_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, vertex->gl_id);
    b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);
printOpenGLError(); // DEBUG

    glBindBuffer(GL_ARRAY_BUFFER, normal->gl_id);
    b_normal = glGetAttribLocation(shader_id, "b_normal");
    glVertexAttribPointer(b_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_normal);
printOpenGLError(); // DEBUG

    glBindBuffer(GL_ARRAY_BUFFER, uv->gl_id);
    b_uv = glGetAttribLocation(shader_id, "b_uv");
    glVertexAttribPointer(b_uv, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_uv);
printOpenGLError(); // DEBUG

//glEnable(GL_TEXTURE_2D); // "If you use GLSL shaders in OpenGL the call glEnable(GL_TEXTURE) has not influence." -- https://stackoverflow.com/questions/4041124/should-glenablegl-texture2d-be-applied-per-texture-unit
    s_tex = glGetUniformLocation(shader_id, "s_tex");
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(s_tex, 1);
    glBindTexture(GL_TEXTURE_2D, texture->gl_id);
printOpenGLError(); // DEBUG

    m_mvp = glGetUniformLocation(shader_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, mvp_matrix);

    m_mv = glGetUniformLocation(shader_id, "m_mv");
    glUniformMatrix4fv(m_mv, 1, GL_FALSE, mv_matrix);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index->gl_id);
    glDrawElements(GL_TRIANGLES, index->nr_strides, GL_UNSIGNED_SHORT, NULL);
printOpenGLError(); // DEBUG

    esmDestroy(mvp_matrix);
    esmDestroy(mv_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void vrms_server_draw_scene_object(vrms_scene_t* scene, uint32_t matrix_id, uint32_t matrix_idx, uint32_t mesh_id, float* projection_matrix, float* view_matrix, float* model_matrix) {
    vrms_object_t* matrix_object;
    vrms_object_t* mesh_object;
    vrms_object_data_t* matrix;
    float matrix_buffer[16];

    if (matrix_id >= scene->next_object_id) {
        fprintf(stderr, "matrix object: %d is out of bounds\n", matrix_id);
        return;
    }

    if (mesh_id >= scene->next_object_id) {
        fprintf(stderr, "mesh object: %d is out of bounds\n", mesh_id);
        return;
    }

    matrix_object = vrms_scene_get_object_by_id(scene, matrix_id);
    matrix = matrix_object->object.object_data;

    if (NULL != matrix->local_storage) {
        uint32_t offset = matrix_idx * 16;
        uint32_t size = sizeof(float) * 16;
        memcpy(matrix_buffer, &((char*)matrix->local_storage)[offset], size);
        esmMultiply(model_matrix, matrix_buffer);
    }

    mesh_object = vrms_scene_get_object_by_id(scene, mesh_id);

    if (VRMS_OBJECT_MESH_COLOR == mesh_object->type) {
        vrms_server_draw_mesh_color(scene, scene->onecolor_shader_id, mesh_object->object.object_mesh_color, projection_matrix, view_matrix, model_matrix);
    }
    else if (VRMS_OBJECT_MESH_TEXTURE == mesh_object->type) {
        vrms_server_draw_mesh_texture(scene, scene->texture_shader_id, mesh_object->object.object_mesh_texture, projection_matrix, view_matrix, model_matrix);
    }
}

void vrms_server_draw_scene_buffer(vrms_scene_t* scene, float* projection_matrix, float* view_matrix, float* model_matrix) {
    uint32_t matrix_id, matrix_idx, mesh_id;

    if (!pthread_mutex_trylock(scene->render_buffer_lock)) {
        int i = 0;
        int idx = 0;
        for (i = 0; i < scene->render_buffer_nr_objects; i++) {
            matrix_id = scene->render_buffer[idx + 0];
            matrix_idx = scene->render_buffer[idx + 1];
            mesh_id = scene->render_buffer[idx + 2];
            vrms_server_draw_scene_object(scene, matrix_id, matrix_idx, mesh_id, projection_matrix, view_matrix, model_matrix);
            idx += 3;
        }
        pthread_mutex_unlock(scene->render_buffer_lock);
    }
    else {
        fprintf(stderr, "lock on render bufer\n");
    }
}

void vrms_server_draw_scenes(vrms_server_t* server, float* projection_matrix, float* view_matrix, float* model_matrix) {
    int si;//, oi;
    vrms_scene_t* scene;

    for (si = 1; si < server->next_scene_id; si++) {
        scene = server->scenes[si];
        if (NULL != scene) {
            vrms_server_draw_scene_buffer(scene, projection_matrix, view_matrix, model_matrix);
        }
        if (si >= 2000) break;
    }
}

void vrms_queue_load_gl_element_buffer(vrms_queue_item_data_load_t* data_load) {
    if (NULL != data_load->buffer) {
        glGenBuffers(1, data_load->destination);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *data_load->destination);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_load->size, data_load->buffer, GL_STATIC_DRAW);
        if (0 == *data_load->destination) {
            fprintf(stderr, "unable to load gl element buffer");
            printOpenGLError();
        }
        free(data_load->buffer);
    }
}

void vrms_queue_load_gl_texture_buffer(vrms_queue_item_texture_load_t* texture_load) {
    if (NULL != texture_load->buffer) {
        glGenTextures(1, texture_load->destination);
        glBindTexture(GL_TEXTURE_2D, *texture_load->destination);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture_load->width, texture_load->height, 0, GL_BGR, GL_UNSIGNED_BYTE, texture_load->buffer);
        if (0 == *texture_load->destination) {
            fprintf(stderr, "unable to load gl texture buffer");
            printOpenGLError();
        }
        free(texture_load->buffer);
    }
}

void vrms_queue_load_gl_buffer(vrms_queue_item_data_load_t* data_load) {
    if (NULL != data_load->buffer) {
        glGenBuffers(1, data_load->destination);
        glBindBuffer(GL_ARRAY_BUFFER, *data_load->destination);
        glBufferData(GL_ARRAY_BUFFER, data_load->size, data_load->buffer, GL_STATIC_DRAW);
        if (0 == *data_load->destination) {
            fprintf(stderr, "unable to load gl buffer");
            printOpenGLError();
        }
        free(data_load->buffer);
    }
}

void vrms_server_queue_item_process(vrms_server_t* server, vrms_queue_item_t* queue_item) {
    if (VRMS_QUEUE_DATA_LOAD == queue_item->type) {
        vrms_queue_item_data_load_t* data_load = queue_item->item.data_load;
        if (VRMS_INDEX == data_load->type) {
            vrms_queue_load_gl_element_buffer(data_load);
        }
        else {
            vrms_queue_load_gl_buffer(data_load);
        }
        free(data_load);
    }
    else if (VRMS_QUEUE_TEXTURE_LOAD == queue_item->type) {
        vrms_queue_item_texture_load_t* texture_load = queue_item->item.texture_load;
        vrms_queue_load_gl_texture_buffer(texture_load);
    }
    else if (VRMS_QUEUE_SCENE_DESTROY == queue_item->type) {
        vrms_queue_item_scene_destroy_t* scene_destroy = queue_item->item.scene_destroy;
        vrms_server_destroy_scene(server, scene_destroy->scene_id);
        free(scene_destroy);
    }
    else if (VRMS_QUEUE_EVENT == queue_item->type) {
        fprintf(stderr, "not supposed to get a VRMS_QUEUE_EVENT from a client\n");
    }
    else {
        fprintf(stderr, "unknown queue type!!\n");
    }
    free(queue_item);
}

void vrms_scene_queue_item_flush(vrms_server_t* server) {
    uint32_t idx;
    vrms_queue_item_t* queue_item;
    for (idx = 0; idx < server->inbound_queue_index; idx++) {
        queue_item = server->inbound_queue[idx];
        if (NULL != queue_item) {
            vrms_server_queue_item_process(server, queue_item);
        }
        else {
            fprintf(stderr, "null queue item\n");
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
        fprintf(stderr, "socket thread has lock on queue\n");
    }
}

void vrms_server_process_queue(vrms_server_t* server) {
    vrms_server_flush_client_inbound_queue(server);
}
