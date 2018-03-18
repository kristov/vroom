#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include "vrms_gl.h"

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
#include "vrms_render_vm.h"
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

void vrms_server_draw_mesh_color(vrms_server_t* server, vrms_object_mesh_color_t* mesh, float* projection_matrix, float* view_matrix, float* model_matrix) {
    GLuint b_vertex;
    GLuint b_normal;
    GLuint u_color;
    GLuint m_mvp;
    GLuint m_mv;
    GLuint shader_id;
    float* mvp_matrix;
    float* mv_matrix;

    shader_id = server->color_shader_id;
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

void vrms_server_draw_mesh_texture(vrms_server_t* server, vrms_object_mesh_texture_t* mesh, float* projection_matrix, float* view_matrix, float* model_matrix) {
    GLuint b_vertex;
    GLuint b_normal;
    GLuint b_uv;
    GLuint s_tex;
    GLuint m_mvp;
    GLuint m_mv;
    GLuint shader_id;
    float* mvp_matrix;
    float* mv_matrix;

    shader_id = server->texture_shader_id;
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

void vrms_server_draw_skybox(vrms_server_t* server, vrms_object_skybox_t* skybox, float* projection_matrix, float* view_matrix, float* model_matrix) {
    GLuint b_vertex;
    GLuint s_tex;
    GLuint m_mvp;
    GLuint shader_id;
    float* mvp_matrix;

    shader_id = server->cubemap_shader_id;
    glUseProgram(shader_id);

    glDisable(GL_DEPTH_TEST);
    mvp_matrix = esmCreateCopy(projection_matrix);
    esmMultiply(mvp_matrix, view_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, skybox->vertex_gl_id);
    b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);
printOpenGLError();

    s_tex = glGetUniformLocation(shader_id, "s_tex");
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(s_tex, 1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->texture_gl_id);
printOpenGLError();

    m_mvp = glGetUniformLocation(shader_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, mvp_matrix);
printOpenGLError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox->index_gl_id);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, NULL);
printOpenGLError();

    esmDestroy(mvp_matrix);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
}

void vrms_server_draw_scenes(vrms_server_t* server, float* projection_matrix, float* view_matrix, float* model_matrix, float* skybox_projection_matrix) {
    int si;//, oi;
    vrms_scene_t* scene;
    uint32_t usec_elapsed;
    uint8_t i;

    // TODO: Dont loop through scenes like this, but do round robin on the VM calls
    // across the scenes, so its not that one scene gets to draw everything, and
    // other scenes starve. Sounds a bit like a task scheduler.
    usec_elapsed = 0;
    for (si = 1; si < server->next_scene_id; si++) {
        esmLoadIdentity(model_matrix);
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

void vrms_queue_load_gl_element_buffer(vrms_queue_item_data_load_t* load) {
    if (NULL == load->buffer) {
        return;
    }

    glGenBuffers(1, load->destination);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *load->destination);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, load->size, load->buffer, GL_STATIC_DRAW);
    if (0 == *load->destination) {
        debug_print("unable to load gl element buffer");
        printOpenGLError();
    }
    else {
        debug_print("vrms_queue_load_gl_element_buffer() loaded GL id: %d\n", *load->destination);
    }
}

void vrms_queue_load_gl_texture_buffer(vrms_queue_item_texture_load_t* load) {
    GLint ifmt;
    GLenum dfmt;
    GLenum bfmt;
    uint32_t w, h;
    uint32_t part_offset;
    uint32_t off;
    uint8_t* buffer;
    uint8_t* tmp;

    if (NULL == load->buffer) {
        return;
    }
    buffer = load->buffer;

    w = load->width;
    h = load->height;

    switch (load->format) {
        case VRMS_RGB8:
            dfmt = GL_RGB;
            ifmt = GL_RGB8;
            bfmt = GL_UNSIGNED_BYTE;
            break;
        default:
            debug_print("texture format unrecognized: %d\n", load->format);
            break;
    }

    switch (load->type) {
        case VRMS_TEXTURE_2D:
            glGenTextures(1, load->destination);
            glBindTexture(GL_TEXTURE_2D, *load->destination);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, ifmt, w, h, 0, dfmt, bfmt, (void*)buffer);
            debug_print("vrms_queue_load_gl_texture_buffer(GL_TEXTURE_2D) loaded GL id: %d\n", *load->destination);
            break;
        case VRMS_TEXTURE_CUBE_MAP:
            off = 0;
            part_offset = w * h * 3;
            glGenTextures(1, load->destination);
            glBindTexture(GL_TEXTURE_CUBE_MAP, *load->destination);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, ifmt, w, h, 0, dfmt, bfmt, tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, ifmt, w, h, 0, dfmt, bfmt, tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, ifmt, w, h, 0, dfmt, bfmt, tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, ifmt, w, h, 0, dfmt, bfmt, tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, ifmt, w, h, 0, dfmt, bfmt, tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, ifmt, w, h, 0, dfmt, bfmt, tmp);
            debug_print("vrms_queue_load_gl_texture_buffer(GL_TEXTURE_CUBE_MAP) loaded GL id: %d\n", *load->destination);
            break;
        default:
            debug_print("unknown texture type: %d\n", load->type);
            break;
    }
    if (0 == *load->destination) {
        debug_print("unable to load gl texture buffer");
        printOpenGLError();
    }
}

void vrms_queue_load_gl_buffer(vrms_queue_item_data_load_t* load) {
    if (NULL == load->buffer) {
        return;
    }

    glGenBuffers(1, load->destination);
    glBindBuffer(GL_ARRAY_BUFFER, *load->destination);
    glBufferData(GL_ARRAY_BUFFER, load->size, load->buffer, GL_STATIC_DRAW);
    if (0 == *load->destination) {
        debug_print("unable to load gl buffer");
        printOpenGLError();
    }
    else {
        debug_print("vrms_queue_load_gl_buffer() loaded GL id: %d\n", *load->destination);
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

