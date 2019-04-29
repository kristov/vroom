#ifndef OPENGL_STEREO_H
#define OPENGL_STEREO_H

#include "gl_compat.h"

typedef struct opengl_stereo_camera {
    float projection_matrix[16];
    float model_translation;
} opengl_stereo_camera;

typedef struct opengl_stereo_buffer_store {
    GLuint buffer;
    GLuint rendered_texture;
} opengl_stereo_buffer_store;

typedef struct opengl_stereo opengl_stereo;

typedef void (*ostereo_draw_scene_callback_t)(opengl_stereo* ostereo, void* data);

typedef struct opengl_stereo {
    double width;
    double height;
    double depthZ;
    double fovy;
    double aspect;
    double nearZ;
    double farZ;
    double screenZ;
    double IOD;
    double physical_width;
    double texture_shift;
    GLuint screen_plane_vdb;
    GLuint screen_plane_idb;
    GLuint screen_text_offset;
    float screen_matrix[16];
    GLuint screen_shader_program_id;
    GLuint color_shader_id;
    GLuint texture_shader_id;
    GLuint cubemap_shader_id;
    float model_matrix[16];
    float view_matrix[16];
    float hmd_matrix[16];
    float projection_matrix[16];
    ostereo_draw_scene_callback_t draw_scene_callback;
    void* draw_scene_callback_data;
    GLuint barrel_power_id;
    opengl_stereo_camera left_camera;
    opengl_stereo_camera right_camera;
    opengl_stereo_camera skybox_camera;
    opengl_stereo_buffer_store* screen_buffers;
} opengl_stereo;

void opengl_stereo_draw_scene_callback(opengl_stereo* ostereo, ostereo_draw_scene_callback_t callback, void* callback_data);
void opengl_stereo_reshape(opengl_stereo* ostereo, int w, int h);
void opengl_stereo_display(opengl_stereo* ostereo);
void opengl_stereo_init(opengl_stereo* ostereo, int width, int height, double physical_width);

double opengl_stereo_get_config_value(opengl_stereo* ostereo, char* name);
void opengl_stereo_set_config_value(opengl_stereo* ostereo, char* name, double value);

#endif
