#ifndef VRMS_GL_H
#define VRMS_GL_H

#include <stdint.h>
#include "vroom.h"

typedef struct vrms_gl_render {
    uint32_t shader_id;
    uint32_t vertex_id;
    uint32_t normal_id;
    uint32_t index_id;
    uint32_t color_id;
    uint32_t uv_id;
    uint32_t texture_id;
    uint32_t nr_indicies;
    uint8_t realized;
} vrms_gl_render_t;

typedef struct vrms_gl_matrix {
    float* m;
    float* v;
    float* p;
    float mvp[16];
    float mv[16];
    uint8_t realized;
} vrms_gl_matrix_t;

void vrms_gl_draw_mesh_color(vrms_gl_render_t render, vrms_gl_matrix_t matrix);

void vrms_gl_draw_mesh_texture(vrms_gl_render_t render, vrms_gl_matrix_t matrix);

void vrms_gl_draw_skybox(vrms_gl_render_t render, vrms_gl_matrix_t matrix);

void vrms_gl_load_buffer(uint8_t* buffer, uint32_t* destination, uint32_t size, vrms_data_type_t type);

void vrms_gl_load_texture_buffer(uint8_t* buffer, uint32_t* destination, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type);

void vrms_gl_delete_buffer(uint32_t* gl_id);

#endif
