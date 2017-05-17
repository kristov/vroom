#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
#include "vrms_server.h"

vrms_server_t* vrms_server_create() {
    vrms_server_t* vrms_server = malloc(sizeof(vrms_server_t));
    vrms_server->scenes = malloc(sizeof(vrms_scene_t) * 10);
    memset(vrms_server->scenes, 0, sizeof(vrms_scene_t) * 10);
    vrms_server->next_scene_id = 1;
    return vrms_server;
}

vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id) {
    return vrms_server->scenes[scene_id];
}

uint32_t vrms_create_scene(vrms_server_t* vrms_server, char* name) {
    vrms_scene_t* vrms_scene = malloc(sizeof(vrms_scene_t));
    vrms_scene->objects = malloc(sizeof(vrms_object_t) * 10);
    memset(vrms_scene->objects, 0, sizeof(vrms_object_t) * 10);
    vrms_scene->next_object_id = 1;
    vrms_server->scenes[vrms_server->next_scene_id] = vrms_scene;
    vrms_scene->id = vrms_server->next_scene_id;
    vrms_server->next_scene_id++;
    return vrms_scene->id;
}

vrms_object_t* vrms_object_create(vrms_scene_t* vrms_scene) {
    vrms_object_t* vrms_object = malloc(sizeof(vrms_object_t));
    vrms_object->type = VRMS_OBJECT_INVALID;
    vrms_scene->objects[vrms_scene->next_object_id] = vrms_object;
    vrms_object->id = vrms_scene->next_object_id;
    vrms_scene->next_object_id++;
    return vrms_object;
}

uint32_t vrms_create_data_object(vrms_scene_t* vrms_scene, vrms_data_type_t type, uint32_t fd, uint32_t dtype, uint32_t offset, uint32_t size, uint32_t stride) {
    void* address;
    void* buffer;
    uint32_t* int_buffer;
    float* float_buffer;
    int32_t seals;
    vrms_object_t* vrms_object;

    buffer = malloc(size);
    seals = fcntl(fd, F_GET_SEALS);
    if (!(seals & F_SEAL_SHRINK)) {
        fprintf(stderr, "got non-sealed memfd\n");
        return 0;
    }

    address = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (MAP_FAILED == address) {
        fprintf(stderr, "memory map failed\n");
        return 0;
    }

    memcpy(buffer, &((char*)address)[offset], size);

    vrms_object = vrms_object_create(vrms_scene);
    vrms_object->type = VRMS_OBJECT_DATA;

    vrms_object_data_t* object_data = malloc(sizeof(vrms_object_data_t));
    object_data->type = type;
    vrms_object->object.object_data = object_data;

    if (VRMS_FLOAT == dtype) {
        object_data->nr_values = size / sizeof(float);
        float_buffer = (float*)buffer;
        glGenBuffers(1, &object_data->gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, object_data->gl_id);
        glBufferData(GL_ARRAY_BUFFER, size, float_buffer, GL_STATIC_DRAW);
    }
    else if (VRMS_INT == dtype) {
        object_data->nr_values = size / sizeof(uint32_t);
        int_buffer = (uint32_t*)buffer;
        glGenBuffers(1, &object_data->gl_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_data->gl_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, int_buffer, GL_STATIC_DRAW);
    }

    return vrms_object->id;
}

uint32_t vrms_create_geometry_object(vrms_scene_t* vrms_scene, uint32_t vertex_id, uint32_t normal_id, uint32_t index_id) {
    vrms_object_t* vrms_object;
    vrms_object = vrms_object_create(vrms_scene);
    vrms_object->type = VRMS_OBJECT_GEOMETRY;

    vrms_object_geometry_t* object_geometry = malloc(sizeof(vrms_object_geometry_t));
    object_geometry->vertex_id = vertex_id;
    object_geometry->normal_id = normal_id;
    object_geometry->index_id = index_id;

    vrms_object->object.object_geometry = object_geometry;

    return vrms_object->id;
}

uint32_t vrms_create_mesh_color(vrms_scene_t* vrms_scene, uint32_t geometry_id, float r, float g, float b, float a) {
    vrms_object_t* vrms_object;
    vrms_object = vrms_object_create(vrms_scene);
    vrms_object->type = VRMS_OBJECT_MESH_COLOR;

    vrms_object_mesh_color_t* object_mesh_color = malloc(sizeof(vrms_object_mesh_color_t));
    object_mesh_color->geometry_id = geometry_id;
    object_mesh_color->r = r;
    object_mesh_color->g = g;
    object_mesh_color->b = b;
    object_mesh_color->a = a;

    vrms_object->object.object_mesh_color = object_mesh_color;

    return vrms_object->id;
}

uint32_t vrms_create_mesh_texture(vrms_scene_t* vrms_scene, uint32_t geometry_id, uint32_t uv_id, uint32_t texture_id) {
    vrms_object_t* vrms_object;
    vrms_object = vrms_object_create(vrms_scene);
    vrms_object->type = VRMS_OBJECT_MESH_TEXTURE;

    vrms_object_mesh_texture_t* object_mesh_texture = malloc(sizeof(vrms_object_mesh_texture_t));
    object_mesh_texture->geometry_id = geometry_id;
    object_mesh_texture->uv_id = uv_id;
    object_mesh_texture->texture_id = texture_id;

    vrms_object->object.object_mesh_texture = object_mesh_texture;

    return vrms_object->id;
}

vrms_object_t* vrms_server_get_object_by_id(vrms_scene_t* vrms_scene, uint32_t id) {
    vrms_object_t* vrms_object;
    if (vrms_scene->next_object_id <= id) {
        fprintf(stderr, "id out of range\n");
        return NULL;
    }
    vrms_object = vrms_scene->objects[id];
    if (NULL == vrms_object) {
        fprintf(stderr, "undefined object ofr id: %d\n", id);
        return NULL;
    }
    return vrms_object;
}

void vrms_server_draw_mesh_color(vrms_scene_t* vrms_scene, vrms_object_mesh_color_t* vrms_object_mesh_color, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
    GLuint b_vertex, b_normal, u_color, m_mvp, m_mv, m_m;
    GLfloat* mvp_matrix;
    GLfloat* mv_matrix;
    GLfloat* color;

    color = malloc(sizeof(GLfloat) * 4);
    
    vrms_object_t* vrms_object;
    vrms_object_geometry_t* object_geometry;
    vrms_object_data_t* object_data_vertex;
    vrms_object_data_t* object_data_normal;
    vrms_object_data_t* object_data_index;

    color[0] = vrms_object_mesh_color->r;
    color[1] = vrms_object_mesh_color->g;
    color[2] = vrms_object_mesh_color->b;
    color[3] = vrms_object_mesh_color->a;

    vrms_object = vrms_server_get_object_by_id(vrms_scene, vrms_object_mesh_color->geometry_id);
    object_geometry = vrms_object->object.object_geometry;

    vrms_object = vrms_server_get_object_by_id(vrms_scene, object_geometry->vertex_id);
    object_data_vertex = vrms_object->object.object_data;

    vrms_object = vrms_server_get_object_by_id(vrms_scene, object_geometry->normal_id);
    object_data_normal = vrms_object->object.object_data;

    vrms_object = vrms_server_get_object_by_id(vrms_scene, object_geometry->index_id);
    object_data_index = vrms_object->object.object_data;

    mv_matrix = esmCreateCopy(view_matrix);
    esmMultiply(mv_matrix, model_matrix);

    mvp_matrix = esmCreateCopy(projection_matrix);
    esmMultiply(mvp_matrix, view_matrix);
    esmMultiply(mvp_matrix, model_matrix);

    glBindBuffer(GL_ARRAY_BUFFER, object_data_vertex->gl_id);
    b_vertex = glGetAttribLocation(mesh->shader_program_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);

    glBindBuffer(GL_ARRAY_BUFFER, object_data_normal->gl_id);
    b_normal = glGetAttribLocation(mesh->shader_program_id, "b_normal");
    glVertexAttribPointer(b_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_normal);

    u_color = glGetUniformLocation(mesh->shader_program_id, "u_color");
    glUniformMatrix3fv(u_color, 1, GL_FALSE, color);
    glEnableVertexAttribArray(u_color);

    m_mvp = glGetUniformLocation(mesh->shader_program_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, mvp_matrix);
    glEnableVertexAttribArray(m_mvp);

    m_mv = glGetUniformLocation(mesh->shader_program_id, "m_mv");
    glUniformMatrix4fv(m_mv, 1, GL_FALSE, mv_matrix);
    glEnableVertexAttribArray(m_mv);

    m_m = glGetUniformLocation(mesh->shader_program_id, "m_m");
    glUniformMatrix4fv(m_m, 1, GL_FALSE, model_matrix);
    glEnableVertexAttribArray(m_m);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_data_index->gl_id);
    glDrawElements(GL_TRIANGLES, mesh->nr_indicies, GL_UNSIGNED_SHORT, NULL);

    esmDestroy(mvp_matrix);
    esmDestroy(mv_matrix);
}

void vrms_server_draw_mesh_texture(vrms_scene_t* vrms_scene, vrms_object_mesh_texture_t* vrms_object_mesh_texture, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
}

void vrms_server_draw_scene(vrms_server_t* vrms_server, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix) {
    int si, oi;
    vrms_scene_t* vrms_scene;
    vrms_object_t* vrms_object;
    for (si = 1; si < vrms_server->next_scene_id; si++) {
        vrms_scene = vrms_server->scenes[si];
        if (NULL != vrms_scene) {
            for (oi = 0; oi < vrms_scene->next_object_id; oi++) {
                vrms_object = vrms_scene->objects[oi];
                switch (vrms_object->type) {
                    case VRMS_OBJECT_INVALID:
                    break;
                    case VRMS_OBJECT_SCENE:
                    break;
                    case VRMS_OBJECT_DATA:
                    break;
                    case VRMS_OBJECT_GEOMETRY:
                    break;
                    case VRMS_OBJECT_TEXTURE:
                    break;
                    case VRMS_OBJECT_MESH_COLOR:
                        vrms_server_draw_mesh_color(vrms_scene, vrms_object->object.object_mesh_color, projection_matrix, view_matrix, model_matrix);
                    break;
                    case VRMS_OBJECT_MESH_TEXTURE:
                        vrms_server_draw_mesh_texture(vrms_scene, vrms_object->object.object_mesh_texture, projection_matrix, view_matrix, model_matrix);
                    break;
                    case VRMS_OBJECT_MATRIX:
                        //vrms_server_draw_matrix();
                    break;
                }
                if (oi >= 2000) break;
            }
        }
        else {
            fprintf(stderr, "null scene at %d\n", si);
        }
        if (si >= 2000) break;
    }
}
