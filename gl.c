#include <stdio.h>
#include "gl.h"
#include "gl-matrix.h"
#include "gl_compat.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

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

void vrms_gl_draw_mesh_color(vrms_gl_render_t render, vrms_gl_matrix_t matrix) {
    GLuint shader_id = (GLuint)render.shader_id;
    glUseProgram(shader_id);
printOpenGLError();

    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)render.vertex_id);
    GLuint b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);
printOpenGLError();

    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)render.normal_id);
    GLuint b_normal = glGetAttribLocation(shader_id, "b_normal");
    glVertexAttribPointer(b_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_normal);
printOpenGLError();

    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)render.color_id);
    GLuint b_color = glGetAttribLocation(shader_id, "b_color");
    glVertexAttribPointer(b_color, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_color);
printOpenGLError();

    GLuint m_mvp = glGetUniformLocation(shader_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, matrix.mvp);
printOpenGLError();

    GLuint m_mv = glGetUniformLocation(shader_id, "m_mv");
    glUniformMatrix4fv(m_mv, 1, GL_FALSE, matrix.mv);
printOpenGLError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)render.index_id);
    glDrawElements(GL_TRIANGLES, (GLuint)render.nr_indicies, GL_UNSIGNED_SHORT, NULL);
printOpenGLError();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
printOpenGLError();
}

void vrms_gl_draw_mesh_texture(vrms_gl_render_t render, vrms_gl_matrix_t matrix) {
    GLuint b_vertex;
    GLuint b_normal;
    GLuint b_uv;
    GLuint s_tex;
    GLuint m_mvp;
    GLuint m_mv;
    GLuint shader_id;

    shader_id = (GLuint)render.shader_id;
    glUseProgram(shader_id);
printOpenGLError();

    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)render.vertex_id);
    b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);
printOpenGLError();

    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)render.normal_id);
    b_normal = glGetAttribLocation(shader_id, "b_normal");
    glVertexAttribPointer(b_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_normal);
printOpenGLError();

    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)render.uv_id);
    b_uv = glGetAttribLocation(shader_id, "b_uv");
    glVertexAttribPointer(b_uv, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_uv);
printOpenGLError();

    s_tex = glGetUniformLocation(shader_id, "s_tex");
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(s_tex, 1);
    glBindTexture(GL_TEXTURE_2D, (GLuint)render.texture_id);
printOpenGLError();

    m_mvp = glGetUniformLocation(shader_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, matrix.mvp);
printOpenGLError();

    m_mv = glGetUniformLocation(shader_id, "m_mv");
    glUniformMatrix4fv(m_mv, 1, GL_FALSE, matrix.mv);
printOpenGLError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)render.index_id);
    glDrawElements(GL_TRIANGLES, (GLuint)render.nr_indicies, GL_UNSIGNED_SHORT, NULL);
printOpenGLError();

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
printOpenGLError();
}

void vrms_gl_draw_skybox(vrms_gl_render_t render, vrms_gl_matrix_t matrix) {
    GLuint b_vertex;
    GLuint s_tex;
    GLuint m_mvp;
    GLuint shader_id;

    shader_id = (GLuint)render.shader_id;
    glUseProgram(shader_id);
printOpenGLError();
    glDisable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)render.vertex_id);
    b_vertex = glGetAttribLocation(shader_id, "b_vertex");
    glVertexAttribPointer(b_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(b_vertex);
printOpenGLError();

    s_tex = glGetUniformLocation(shader_id, "s_tex");
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(s_tex, 1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, (GLuint)render.texture_id);
printOpenGLError();

    m_mvp = glGetUniformLocation(shader_id, "m_mvp");
    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, matrix.mvp);
printOpenGLError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)render.index_id);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, NULL);
printOpenGLError();

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
}

void vrms_gl_load_buffer(uint8_t* buffer, uint32_t* destination, uint32_t size, vrms_data_type_t type) {
    glGenBuffers(1, destination);
    if (VRMS_UINT16 == type) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *destination);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
    }
    else {
        glBindBuffer(GL_ARRAY_BUFFER, *destination);
        glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
    }
    if (0 == *destination) {
        debug_print("unable to load gl buffer\n");
        printOpenGLError();
    }
    else {
        debug_print("C|DEBUG|gl.c|vrms_queue_load_gl_buffer() loaded GL id: %d\n", *destination);
    }
}

void vrms_gl_load_texture_buffer(uint8_t* buffer, uint32_t* destination, uint32_t width, uint32_t height, vrms_texture_format_t format, vrms_texture_type_t type) {
    GLint ifmt;
    GLenum dfmt;
    GLenum bfmt;
    uint32_t part_offset;
    uint32_t off;
    uint8_t* tmp;

    switch (format) {
        case VRMS_FORMAT_BGR888:
            ifmt = GL_RGB8;
            dfmt = GL_RGB;
            bfmt = GL_UNSIGNED_BYTE;
            break;
        case VRMS_FORMAT_XBGR8888:
            ifmt = GL_RGBA8;
            dfmt = GL_RGBA;
            bfmt = GL_UNSIGNED_BYTE;
            break;
        case VRMS_FORMAT_ABGR8888:
            ifmt = GL_RGBA8;
            dfmt = GL_RGBA;
            bfmt = GL_UNSIGNED_BYTE;
            break;
        //case VRMS_FORMAT_RGB888:
        //    ifmt = GL_RGB8;
        //    dfmt = GL_BGR;
        //    bfmt = GL_UNSIGNED_BYTE;
        //    break;
        case VRMS_FORMAT_XRGB8888:
            ifmt = GL_RGBA8;
            dfmt = GL_BGRA;
            bfmt = GL_UNSIGNED_BYTE;
            break;
        case VRMS_FORMAT_ARGB8888:
            ifmt = GL_RGBA8;
            dfmt = GL_BGRA;
            bfmt = GL_UNSIGNED_BYTE;
            break;
        default:
            debug_print("texture format unrecognized: %d\n", format);
            break;
    }

    switch (type) {
        case VRMS_TEXTURE_2D:
            glGenTextures(1, destination);
            glBindTexture(GL_TEXTURE_2D, *destination);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, dfmt, bfmt, (void*)buffer);
            debug_print("C|DEBUG|gl.c|vrms_queue_load_gl_texture_buffer(GL_TEXTURE_2D) loaded GL id: %d\n", *destination);
            break;
        case VRMS_TEXTURE_CUBE_MAP:
            off = 0;
            part_offset = width * height * 3;
            glGenTextures(1, destination);
            glBindTexture(GL_TEXTURE_CUBE_MAP, *destination);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, ifmt, width, height, 0, dfmt, bfmt, tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, ifmt, width, height, 0, dfmt, bfmt, tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, ifmt, width, height, 0, dfmt, bfmt, tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, ifmt, width, height, 0, dfmt, bfmt, tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, ifmt, width, height, 0, dfmt, bfmt, tmp);
            off += part_offset;
            tmp = &buffer[off];
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, ifmt, width, height, 0, dfmt, bfmt, tmp);
            debug_print("C|DEBUG|gl.c|vrms_queue_load_gl_texture_buffer(GL_TEXTURE_CUBE_MAP) loaded GL id: %d\n", *destination);
            break;
        default:
            debug_print("unknown texture type: %d\n", type);
            break;
    }
    if (0 == *destination) {
        debug_print("unable to load gl texture buffer\n");
        printOpenGLError();
    }
}

void vrms_gl_delete_buffer(uint32_t* gl_id) {
    glDeleteBuffers(1, gl_id);
}
