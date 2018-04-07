#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vrms_gl.h"
#define DTR 0.0174532925
#include "safe_malloc.h"
#include "opengl_stereo.h"
#include "ogl_shader_loader.h"
#include "esm.h"

#ifdef RASPBERRYPI
static char screen_vert[] = "shaders/100/screen_vert.glsl";
static char screen_frag[] = "shaders/100/screen_frag.glsl";
static char onecolor_vert[] = "shaders/100/model/onecolor_vert.glsl";
static char onecolor_frag[] = "shaders/100/model/onecolor_frag.glsl";
static char texture_vert[] = "shaders/100/model/texture_vert.glsl";
static char texture_frag[] = "shaders/100/model/texture_frag.glsl";
static char cubemap_vert[] = "shaders/100/model/cubemap_vert.glsl";
static char cubemap_frag[] = "shaders/100/model/cubemap_frag.glsl";
#else /* not RASPBERRYPI */
static char screen_vert[] = "shaders/120/screen_vert.glsl";
static char screen_frag[] = "shaders/120/screen_frag.glsl";
static char onecolor_vert[] = "shaders/120/model/onecolor_vert.glsl";
static char onecolor_frag[] = "shaders/120/model/onecolor_frag.glsl";
static char texture_vert[] = "shaders/120/model/texture_vert.glsl";
static char texture_frag[] = "shaders/120/model/texture_frag.glsl";
static char cubemap_vert[] = "shaders/120/model/cubemap_vert.glsl";
static char cubemap_frag[] = "shaders/120/model/cubemap_frag.glsl";
#endif /* RASPBERRYPI */

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

int printGlError(char *file, int line) {
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

#define printOpenGLError() printGlError(__FILE__, __LINE__)

/*
Eye separation is typically kept at 1/30th of the convergence distance and objects
closer than half the convergence distance are avoided in the scene.
*/

void opengl_stereo_load_screen_shader(opengl_stereo* ostereo) {
    ostereo->screen_shader_program_id = ogl_shader_loader_load(screen_vert, screen_frag);
    ostereo->onecolor_shader_id = ogl_shader_loader_load(onecolor_vert, onecolor_frag);
    ostereo->texture_shader_id = ogl_shader_loader_load(texture_vert, texture_frag);
    ostereo->cubemap_shader_id = ogl_shader_loader_load(cubemap_vert, cubemap_frag);
}

void opengl_stereo_store_screen_plane(opengl_stereo* ostereo) {
    GLfloat* verts;
    GLfloat* uvs;
    GLushort* indicies;
    int voff;
    GLuint buff_size, vert_size, text_size, indi_size;

    vert_size = sizeof(GLfloat) * 12;
    text_size = sizeof(GLfloat) * 8;
    indi_size = sizeof(GLuint) * 6;
    buff_size = vert_size + text_size;

    ostereo->screen_text_offset = vert_size;

    verts = SAFEMALLOC(vert_size);
    uvs = SAFEMALLOC(text_size);
    indicies = SAFEMALLOC(indi_size);

    voff = 0;

    verts[voff + 0] = 0.0f;
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = 2.0f;
    verts[voff + 1] = 0.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = 0.0f;
    verts[voff + 1] = 2.0f;
    verts[voff + 2] = 0.0f;
    voff += 3;
    verts[voff + 0] = 2.0f;
    verts[voff + 1] = 2.0f;
    verts[voff + 2] = 0.0f;

    voff = 0;
    indicies[voff + 0] = 0;
    indicies[voff + 1] = 1;
    indicies[voff + 2] = 2;
    indicies[voff + 3] = 1;
    indicies[voff + 4] = 2;
    indicies[voff + 5] = 3;

    voff = 0;
    uvs[voff + 0] = 0.0f;
    uvs[voff + 1] = 0.0f;
    voff += 2;
    uvs[voff + 0] = 1.0f;
    uvs[voff + 1] = 0.0f;
    voff += 2;
    uvs[voff + 0] = 0.0f;
    uvs[voff + 1] = 1.0f;
    voff += 2;
    uvs[voff + 0] = 1.0f;
    uvs[voff + 1] = 1.0f;

    glGenBuffers(1, &ostereo->screen_plane_vdb);
    glBindBuffer(GL_ARRAY_BUFFER, ostereo->screen_plane_vdb);
    glBufferData(GL_ARRAY_BUFFER, buff_size, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vert_size, verts);
    glBufferSubData(GL_ARRAY_BUFFER, vert_size, text_size, uvs);

    glGenBuffers(1, &ostereo->screen_plane_idb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ostereo->screen_plane_idb);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indi_size, indicies, GL_STATIC_DRAW);

    free(verts);
    free(uvs);
    free(indicies);
}

void opengl_stereo_render_screen_plane(opengl_stereo* ostereo) {
    GLuint b_vertex, b_text;

    glBindBuffer(GL_ARRAY_BUFFER, ostereo->screen_plane_vdb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ostereo->screen_plane_idb);

    b_vertex = glGetAttribLocation(ostereo->screen_shader_program_id, "b_vertex" );
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);

    b_text = glGetAttribLocation(ostereo->screen_shader_program_id, "b_text" );
    glVertexAttribPointer(b_text, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(ostereo->screen_text_offset));
    glEnableVertexAttribArray(b_text);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}

void opengl_stereo_create_render_texture(opengl_stereo* ostereo) {
    GLuint depthRenderBuffer;
    GLenum status;

    glGenFramebuffers(1, &ostereo->screen_buffers->buffer);
    glGenTextures(1, &ostereo->screen_buffers->rendered_texture);
    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffers->buffer);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ostereo->width / 2, ostereo->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ostereo->width / 2, ostereo->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture, 0);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "FRAMEBUFFER incomplete: %d\n", (int)status);
    }

    return;
}

void opengl_stereo_camera_frustrum_L(opengl_stereo_camera* left_camera, double IODh, double top, double right, double frustumshift, double nearZ, double farZ) {
    left_camera->model_translation = IODh;
    esmFrustumf(
        left_camera->projection_matrix,
        -right + frustumshift,
        right + frustumshift,
        -top,
        top,
        nearZ,
        farZ
    );
}

void opengl_stereo_camera_frustrum_R(opengl_stereo_camera* right_camera, double IODh, double top, double right, double frustumshift, double nearZ, double farZ) {
    right_camera->model_translation = -IODh;
    esmFrustumf(
        right_camera->projection_matrix,
        -right - frustumshift,
        right - frustumshift,
        -top,
        top,
        nearZ,
        farZ
    );
}

void opengl_stereo_camera_frustrum_I(opengl_stereo_camera* camera, double IODh, double top, double right, double frustumshift, double nearZ, double farZ) {
    camera->model_translation = 0.0;
    esmFrustumf(
        camera->projection_matrix,
        -right,
        right,
        -top,
        top,
        nearZ,
        farZ
    );
}

void opengl_stereo_set_frustum(opengl_stereo* ostereo) {
    double IODh;
    double top;
    double right;
    double frustumshift;

    IODh = ostereo->IOD / 2;
    top = ostereo->nearZ * tan(DTR * ostereo->fovy / 2);
    right = ostereo->aspect * top;
    frustumshift = IODh * ostereo->nearZ / ostereo->screenZ;

    opengl_stereo_camera_frustrum_L(ostereo->left_camera, IODh, top, right, frustumshift, ostereo->nearZ, ostereo->farZ);
    opengl_stereo_camera_frustrum_R(ostereo->right_camera, IODh, top, right, frustumshift, ostereo->nearZ, ostereo->farZ);
    opengl_stereo_camera_frustrum_I(ostereo->skybox_camera, IODh, top, right, frustumshift, ostereo->nearZ, ostereo->farZ);
}

void opengl_stereo_reshape(opengl_stereo* ostereo, int w, int h) {
    if (h == 0) {
        h = 1;
    }
    ostereo->width = w;
    ostereo->height = h;
    ostereo->aspect = ( (double)w / 2 ) / (double)h;
    opengl_stereo_set_frustum(ostereo);
}

void opengl_stereo_render_left_scene(opengl_stereo* ostereo) {
    GLint tex0;
    GLuint m_projection;

    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffers->buffer);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ostereo->width / 2, ostereo->height);

    esmLoadIdentity(ostereo->view_matrix);
    esmLoadIdentity(ostereo->model_matrix);

    ostereo->projection_matrix = ostereo->left_camera->projection_matrix;
    esmMultiply(ostereo->view_matrix, ostereo->hmd_matrix);
    esmTranslatef(ostereo->view_matrix, ostereo->left_camera->model_translation, 0.0, ostereo->depthZ);

    ostereo->draw_scene_callback(ostereo);

    glUseProgram(ostereo->screen_shader_program_id);

    ostereo->barrel_power_id = glGetUniformLocation(ostereo->screen_shader_program_id, "barrel_power");
    glUniform1f(ostereo->barrel_power_id, 1.1f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glClearColor(0.0f, 0.8f, 0.8f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    esmLoadIdentity(ostereo->screen_matrix);
    esmTranslatef(ostereo->screen_matrix, -1.0 + ostereo->texture_shift, -1.0, 0.0);

    m_projection = glGetUniformLocation(ostereo->screen_shader_program_id, "m_projection");
    glUniformMatrix4fv(m_projection, 1, GL_FALSE, ostereo->screen_matrix);

    glViewport(0, 0, ostereo->width / 2, ostereo->height);

    tex0 = glGetUniformLocation(ostereo->screen_shader_program_id, "tex0");
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(tex0, 0);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture);

    opengl_stereo_render_screen_plane(ostereo);
}

void opengl_stereo_render_right_scene(opengl_stereo* ostereo) {
    GLint texLoc;
    GLuint m_projection;

    glBindFramebuffer(GL_FRAMEBUFFER, ostereo->screen_buffers->buffer);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ostereo->width / 2, ostereo->height);

    esmLoadIdentity(ostereo->view_matrix);
    esmLoadIdentity(ostereo->model_matrix);

    ostereo->projection_matrix = ostereo->right_camera->projection_matrix;
    esmMultiply(ostereo->view_matrix, ostereo->hmd_matrix);
    esmTranslatef(ostereo->view_matrix, ostereo->right_camera->model_translation, 0.0, ostereo->depthZ);

    ostereo->draw_scene_callback(ostereo);

    glUseProgram(ostereo->screen_shader_program_id);

    ostereo->barrel_power_id = glGetUniformLocation(ostereo->screen_shader_program_id, "barrel_power");
    glUniform1f(ostereo->barrel_power_id, 1.1f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glClearColor(0.0f, 0.8f, 0.8f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    esmLoadIdentity(ostereo->screen_matrix);
    esmTranslatef(ostereo->screen_matrix, -1.0 - ostereo->texture_shift, -1.0, 0.0);

    m_projection = glGetUniformLocation(ostereo->screen_shader_program_id, "m_projection");
    glUniformMatrix4fv(m_projection, 1, GL_FALSE, ostereo->screen_matrix);

    glViewport(ostereo->width / 2, 0, ostereo->width / 2, ostereo->height);

    texLoc = glGetUniformLocation(ostereo->screen_shader_program_id, "tex0");
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(texLoc, 0);
    glBindTexture(GL_TEXTURE_2D, ostereo->screen_buffers->rendered_texture);

    opengl_stereo_render_screen_plane(ostereo);
}

void opengl_stereo_render_mono_scene(opengl_stereo* ostereo) {

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ostereo->width, ostereo->height);

    esmLoadIdentity(ostereo->view_matrix);
    esmLoadIdentity(ostereo->model_matrix);

    esmMultiply(ostereo->view_matrix, ostereo->hmd_matrix);
    esmTranslatef(ostereo->view_matrix, ostereo->left_camera->model_translation, 0.0, ostereo->depthZ);

    ostereo->draw_scene_callback(ostereo);
}

void opengl_stereo_render_scene(opengl_stereo* ostereo) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    opengl_stereo_render_left_scene(ostereo);
    opengl_stereo_render_right_scene(ostereo);
    //opengl_stereo_render_mono_scene(ostereo);
}

/*
    display():
        opengl_stereo_render_scene_to_buffers():
            glUseProgram(buffer) <-- Rendering a 3d scene
            opengl_stereo_render_scene_to_left_buffer()
            opengl_stereo_render_scene_to_right_buffer()
        opengl_stereo_render_buffers_to_window():
            glUseProgram(screen) <-- Rendering a texture
            opengl_stereo_render_left_buffer_to_window()
            opengl_stereo_render_right_buffer_to_window()

    alt_display(): (one buffer)
        opengl_stereo_render_left_scene():
            glUseProgram(buffer) <-- Rendering a 3d scene
            opengl_stereo_render_left_scene_to_buffer()
            glUseProgram(screen) <-- Rendering a texture
            opengl_stereo_render_buffer_to_window()
        opengl_stereo_render_right_scene():
            glUseProgram(buffer) <-- Rendering a 3d scene
            opengl_stereo_render_right_scene_to_buffer()
            glUseProgram(screen) <-- Rendering a texture
            opengl_stereo_render_buffer_to_window()
*/
void opengl_stereo_display(opengl_stereo* ostereo) {
    if (ostereo->draw_scene_callback == NULL) {
        fprintf(stderr, "opengl_stereo_ERROR: draw_scene_callback not attached\n");
        return;
    }
    opengl_stereo_render_scene(ostereo);
}

void initGL(opengl_stereo* ostereo) {
    glEnable(GL_DEPTH_TEST);
    //glMatrixMode(GL_PROJECTION);
    //glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_CUBE_MAP);
    //glLoadIdentity();
}

void opengl_stereo_load_defaults(opengl_stereo* ostereo) {
    ostereo->IOD = 0.5;
    ostereo->depthZ = 0.0;
    ostereo->fovy = 45;
    ostereo->nearZ = 0.1;
    ostereo->farZ = 300.0;
    ostereo->screenZ = 100.0;
}

void opengl_stereo_init_system(opengl_stereo* ostereo) {
    initGL(ostereo);

    ostereo->aspect = ( ostereo->width / 2 ) / ostereo->height;
    //double half_screen_width = ostereo->physical_width / 2;
    //double half_iod = ostereo->IOD / 2;
    //double correction_ratio = 2 / ostereo->physical_width;

    ostereo->texture_shift = 0.095f;
    //ostereo->texture_shift = (half_screen_width - half_iod) * correction_ratio;

    //fprintf(stderr, "           Physical width (dm): %0.2f\n", ostereo->physical_width);
    //fprintf(stderr, "              Correction ratio: %0.2f\n", correction_ratio);
    //fprintf(stderr, "(half_screen_width - half_iod): %0.2f\n", (half_screen_width - half_iod));
    //fprintf(stderr, "                 Texture shift: %0.2f\n", ostereo->texture_shift);

    opengl_stereo_load_screen_shader(ostereo);
    opengl_stereo_set_frustum(ostereo);
    opengl_stereo_create_render_texture(ostereo);
    opengl_stereo_store_screen_plane(ostereo);
}

void opengl_stereo_init(opengl_stereo* ostereo) {
    opengl_stereo_load_defaults(ostereo);
    opengl_stereo_init_system(ostereo);
}

opengl_stereo_camera* opengl_stereo_camera_new() {
    opengl_stereo_camera* camera = SAFEMALLOC(sizeof(opengl_stereo_camera));
    memset(camera, 0, sizeof(opengl_stereo_camera));
    camera->projection_matrix = esmCreate();
    camera->model_translation = 0.0f;
    return camera;
}

opengl_stereo_buffer_store* opengl_stereo_buffer_store_new() {
    opengl_stereo_buffer_store* buffer = SAFEMALLOC(sizeof(opengl_stereo_buffer_store));
    memset(buffer, 0, sizeof(opengl_stereo_buffer_store));
    return buffer;
}

opengl_stereo* opengl_stereo_new() {
    opengl_stereo* ostereo = SAFEMALLOC(sizeof(opengl_stereo));
    memset(ostereo, 0, sizeof(opengl_stereo));
    return ostereo;
}

opengl_stereo* opengl_stereo_create(int width, int height, double physical_width) {
    opengl_stereo* ostereo = opengl_stereo_new();
    ostereo->width = width;
    ostereo->height = height;
    ostereo->physical_width = physical_width;
    ostereo->left_camera = opengl_stereo_camera_new();
    ostereo->right_camera = opengl_stereo_camera_new();
    ostereo->skybox_camera = opengl_stereo_camera_new();
    ostereo->screen_buffers = opengl_stereo_buffer_store_new();
    ostereo->screen_matrix = esmCreate();
    ostereo->model_matrix = esmCreate();
    ostereo->view_matrix = esmCreate();
    ostereo->hmd_matrix = esmCreate();
    opengl_stereo_init(ostereo);
    return ostereo;
}
