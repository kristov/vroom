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

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

typedef enum vrms_queue_item_type {
    VRMS_QUEUE_DATA_LOAD,
    VRMS_QUEUE_TEXTURE_LOAD,
    VRMS_QUEUE_SCENE_DESTROY,
    VRMS_QUEUE_UPDATE_SYSTEM_MATRIX,
    VRMS_QUEUE_EVENT
} vrms_queue_item_type_t;

typedef struct vrms_queue_item_scene_destroy {
    uint32_t scene_id;
} vrms_queue_item_scene_destroy_t;

typedef struct vrms_queue_item_data_load {
    vrms_data_type_t type;
    GLuint* destination;
    uint8_t* buffer;
    uint32_t size;
} vrms_queue_item_data_load_t;

typedef struct vrms_queue_item_update_system_matrix {
    vrms_matrix_type_t matrix_type;
    vrms_update_type_t update_type;
    uint8_t* buffer;
} vrms_queue_item_update_system_matrix_t;

typedef struct vrms_queue_item_texture_load {
    GLuint* destination;
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
        vrms_queue_item_scene_destroy_t* scene_destroy;
        vrms_queue_item_update_system_matrix_t* update_system_matrix;
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

    server->head_matrix = esmCreate();
    server->body_matrix = esmCreate();

    return server;
}

vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id) {
    vrms_scene_t* scene;
    if (scene_id >= vrms_server->next_scene_id) {
        debug_print("invalid scene_id [%d] requested: id out of range\n", scene_id);
        return NULL;
    }
    scene = vrms_server->scenes[scene_id];
    if (NULL == scene) {
        debug_print("invalid scene_id [%d] requested: scene does not exist\n", scene_id);
        return NULL;
    }
    return scene;
}

uint32_t vrms_server_create_scene(vrms_server_t* server, char* name) {
    vrms_scene_t* scene = vrms_scene_create(name);

    // TODO: Redo all this stuff somehow... Shaders are created in opengl_stereo
    // then passed to vrms_server_t object in socket, then passed to vrms_secne_t
    // somewhere. Scene shaders should be created in vrms_server.c except the
    // screen plane shader in opengl_stereo which should be put in some standard
    // place.
    scene->onecolor_shader_id = server->onecolor_shader_id;
    scene->texture_shader_id = server->texture_shader_id;
    scene->cubemap_shader_id = server->cubemap_shader_id;

    scene->server = server;

    server->scenes[server->next_scene_id] = scene;
    scene->id = server->next_scene_id;
    server->next_scene_id++;

    return scene->id;
}

void vrms_server_destroy_scene(vrms_server_t* server, uint32_t scene_id) {
    vrms_scene_t* scene = vrms_server_get_scene(server, scene_id);
    if (NULL == scene) {
        debug_print("request to destroy already destroyed scene\n");
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

void vrms_server_queue_add_texture_load(vrms_server_t* server, uint32_t size, GLuint* gl_id_ref, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type, void* buffer) {
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

void vrms_server_queue_update_system_matrix(vrms_server_t* server, vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, void* buffer) {
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

void vrms_server_draw_mesh_color(vrms_scene_t* scene, GLuint shader_id, vrms_object_mesh_color_t* mesh, float* projection_matrix, float* view_matrix, float* model_matrix) {
    GLuint b_vertex, b_normal, u_color, m_mvp, m_mv;
    float* mvp_matrix;
    float* mv_matrix;

    glUseProgram(shader_id);

    mv_matrix = esmCreateCopy(view_matrix);
    esmMultiply(mv_matrix, model_matrix);

    mvp_matrix = esmCreateCopy(projection_matrix);
    esmMultiply(mvp_matrix, view_matrix);
    esmMultiply(mvp_matrix, model_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_gl_id);
    b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->normal_gl_id);
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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_gl_id);
    glDrawElements(GL_TRIANGLES, mesh->nr_indicies, GL_UNSIGNED_SHORT, NULL);

    esmDestroy(mvp_matrix);
    esmDestroy(mv_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void vrms_server_draw_mesh_texture(vrms_scene_t* scene, GLuint shader_id, vrms_object_mesh_texture_t* mesh, float* projection_matrix, float* view_matrix, float* model_matrix) {

    GLuint b_vertex, b_normal, b_uv, s_tex, m_mvp, m_mv;
    float* mvp_matrix;
    float* mv_matrix;

    glUseProgram(shader_id);

    mv_matrix = esmCreateCopy(view_matrix);
    esmMultiply(mv_matrix, model_matrix);

    mvp_matrix = esmCreateCopy(projection_matrix);
    esmMultiply(mvp_matrix, view_matrix);
    esmMultiply(mvp_matrix, model_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_gl_id);
    b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);
printOpenGLError();

    glBindBuffer(GL_ARRAY_BUFFER, mesh->normal_gl_id);
    b_normal = glGetAttribLocation(shader_id, "b_normal");
    glVertexAttribPointer(b_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_normal);
printOpenGLError();

    glBindBuffer(GL_ARRAY_BUFFER, mesh->uv_gl_id);
    b_uv = glGetAttribLocation(shader_id, "b_uv");
    glVertexAttribPointer(b_uv, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_uv);
printOpenGLError();

    s_tex = glGetUniformLocation(shader_id, "s_tex");
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(s_tex, 1);
    glBindTexture(GL_TEXTURE_2D, mesh->texture_gl_id);
printOpenGLError();

    m_mvp = glGetUniformLocation(shader_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, mvp_matrix);
printOpenGLError();

    m_mv = glGetUniformLocation(shader_id, "m_mv");
    glUniformMatrix4fv(m_mv, 1, GL_FALSE, mv_matrix);
printOpenGLError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_gl_id);
    glDrawElements(GL_TRIANGLES, mesh->nr_indicies, GL_UNSIGNED_SHORT, NULL);
printOpenGLError();

    esmDestroy(mvp_matrix);
    esmDestroy(mv_matrix);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void vrms_server_draw_mesh_cubemap(vrms_scene_t* scene, GLuint shader_id, vrms_object_mesh_texture_t* mesh, float* projection_matrix, float* view_matrix, float* model_matrix) {

    GLuint b_vertex, b_normal, b_uv, s_tex, m_mvp, m_mv;
    float* mvp_matrix;
    float* mv_matrix;

    debug_print("rendering cubemap texture\n");
    glUseProgram(shader_id);

    mv_matrix = esmCreateCopy(view_matrix);
    esmMultiply(mv_matrix, model_matrix);

    mvp_matrix = esmCreateCopy(projection_matrix);
    esmMultiply(mvp_matrix, view_matrix);
    esmMultiply(mvp_matrix, model_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_gl_id);
    b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->normal_gl_id);
    b_normal = glGetAttribLocation(shader_id, "b_normal");
    glVertexAttribPointer(b_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_normal);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->uv_gl_id);
    b_uv = glGetAttribLocation(shader_id, "b_uv");
    glVertexAttribPointer(b_uv, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_uv);

    s_tex = glGetUniformLocation(shader_id, "s_tex");
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(s_tex, 1);
    glBindTexture(GL_TEXTURE_2D, mesh->texture_gl_id);

    m_mvp = glGetUniformLocation(shader_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, mvp_matrix);

    m_mv = glGetUniformLocation(shader_id, "m_mv");
    glUniformMatrix4fv(m_mv, 1, GL_FALSE, mv_matrix);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_gl_id);
    glDrawElements(GL_TRIANGLES, mesh->nr_indicies, GL_UNSIGNED_SHORT, NULL);

    esmDestroy(mvp_matrix);
    esmDestroy(mv_matrix);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void vrms_server_draw_scene_object(vrms_scene_t* scene, uint32_t matrix_id, uint32_t matrix_idx, uint32_t mesh_id, float* projection_matrix, float* view_matrix, float* model_matrix) {
    vrms_object_t* matrix_object;
    vrms_object_t* mesh_object;
    vrms_object_data_t* matrix;
    float matrix_buffer[16];

    if (matrix_id >= scene->next_object_id) {
        debug_print("matrix object: %d is out of bounds\n", matrix_id);
        return;
    }

    if (mesh_id >= scene->next_object_id) {
        debug_print("mesh object: %d is out of bounds\n", mesh_id);
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

    mesh_object = vrms_scene_get_mesh_by_id(scene, mesh_id);
    if (NULL == mesh_object) {
        return;
    }

    switch (mesh_object->type) {
        case VRMS_OBJECT_MESH_COLOR:
            vrms_server_draw_mesh_color(scene, scene->onecolor_shader_id, mesh_object->object.object_mesh_color, projection_matrix, view_matrix, model_matrix);
            break;
        case VRMS_OBJECT_MESH_TEXTURE:
            if (0 == mesh_object->object.object_mesh_texture->uv_gl_id) {
                vrms_server_draw_mesh_cubemap(scene, scene->cubemap_shader_id, mesh_object->object.object_mesh_texture, projection_matrix, view_matrix, model_matrix);
            }
            else {
                vrms_server_draw_mesh_texture(scene, scene->texture_shader_id, mesh_object->object.object_mesh_texture, projection_matrix, view_matrix, model_matrix);
            }
            break;
        default:
            break;
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
        debug_print("lock on render bufer\n");
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
            debug_print("unable to load gl element buffer");
            printOpenGLError();
        }
    }
}

void vrms_queue_load_gl_texture_buffer(vrms_queue_item_texture_load_t* texture_load) {
    GLint ifmt;
    GLenum dfmt;
    GLenum bfmt;
    uint32_t w, h;
    uint32_t part_offset;
    uint32_t off;
    uint8_t* buffer;
    uint8_t* tmp;

    if (NULL == texture_load->buffer) {
        return;
    }
    buffer = texture_load->buffer;

    w = texture_load->width;
    h = texture_load->height;

    switch (texture_load->format) {
        case VRMS_RGB8:
            dfmt = GL_RGB;
            ifmt = GL_RGB8;
            bfmt = GL_UNSIGNED_BYTE;
            break;
        default:
            debug_print("texture format unrecognized: %d\n", texture_load->format);
            break;
    }

    switch (texture_load->type) {
        case VRMS_TEXTURE_2D:
            glGenTextures(1, texture_load->destination);
            glBindTexture(GL_TEXTURE_2D, *texture_load->destination);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, ifmt, w, h, 0, dfmt, bfmt, (void*)buffer);
            break;
        case VRMS_TEXTURE_CUBE_MAP:
            off = 0;
            part_offset = (w * h) * 3;
            glGenTextures(1, texture_load->destination);
            glBindTexture(GL_TEXTURE_CUBE_MAP, *texture_load->destination);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, ifmt, w, h, 0, dfmt, bfmt, (void*)tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, ifmt, w, h, 0, dfmt, bfmt, (void*)tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, ifmt, w, h, 0, dfmt, bfmt, (void*)tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, ifmt, w, h, 0, dfmt, bfmt, (void*)tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, ifmt, w, h, 0, dfmt, bfmt, (void*)tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, ifmt, w, h, 0, dfmt, bfmt, (void*)tmp);
            break;
        default:
            debug_print("unknown texture type: %d\n", texture_load->type);
            break;
    }
    if (0 == *texture_load->destination) {
        debug_print("unable to load gl texture buffer");
        printOpenGLError();
    }
}

void vrms_queue_load_gl_buffer(vrms_queue_item_data_load_t* data_load) {
    if (NULL != data_load->buffer) {
        glGenBuffers(1, data_load->destination);
        glBindBuffer(GL_ARRAY_BUFFER, *data_load->destination);
        glBufferData(GL_ARRAY_BUFFER, data_load->size, data_load->buffer, GL_STATIC_DRAW);
        if (0 == *data_load->destination) {
            debug_print("unable to load gl buffer");
            printOpenGLError();
        }
    }
}

void vrms_queue_update_system_matrix(vrms_server_t* server, vrms_queue_item_update_system_matrix_t* update_system_matrix) {
    float* matrix;

    matrix = (float*)update_system_matrix->buffer;
    if (NULL != server->system_matrix_update) {
        server->system_matrix_update(update_system_matrix->matrix_type, update_system_matrix->update_type, matrix);
    }
}

void vrms_server_queue_item_process(vrms_server_t* server, vrms_queue_item_t* queue_item) {
    vrms_queue_item_data_load_t* data_load;
    vrms_queue_item_texture_load_t* texture_load;
    vrms_queue_item_scene_destroy_t* scene_destroy;
    vrms_queue_item_update_system_matrix_t* update_system_matrix;

    switch (queue_item->type) {
        case VRMS_QUEUE_DATA_LOAD:
            data_load = queue_item->item.data_load;
            if (VRMS_INDEX == data_load->type) {
                vrms_queue_load_gl_element_buffer(data_load);
            }
            else {
                vrms_queue_load_gl_buffer(data_load);
            }
            free(data_load);
            break;
        case VRMS_QUEUE_TEXTURE_LOAD:
            texture_load = queue_item->item.texture_load;
            vrms_queue_load_gl_texture_buffer(texture_load);
            break;
        case VRMS_QUEUE_SCENE_DESTROY:
            scene_destroy = queue_item->item.scene_destroy;
            vrms_server_destroy_scene(server, scene_destroy->scene_id);
            free(scene_destroy);
            break;
        case VRMS_QUEUE_UPDATE_SYSTEM_MATRIX:
            update_system_matrix = queue_item->item.update_system_matrix;
            vrms_queue_update_system_matrix(server, update_system_matrix);
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
    vrms_queue_item_t* queue_item;
    for (idx = 0; idx < server->inbound_queue_index; idx++) {
        queue_item = server->inbound_queue[idx];
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

