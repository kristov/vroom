#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <gbm.h>
#include <libdrm/drm.h>
#include <xf86drmMode.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "vrms_server_socket.h"

#define NANO_SECOND_MULTIPLIER 1000000
const long INNER_LOOP_INTERVAL_MS = 50 * NANO_SECOND_MULTIPLIER;

int32_t main(int argc, char **argv) {
    int fd;
    struct gbm_device* gbm;
    EGLDisplay display;
    EGLint num_configs;
    EGLConfig config;
    EGLContext context;
    EGLint major_v;
    EGLint minor_v;
    EGLBoolean result;
    const char* version;
    const char* extensions;

    EGLint attr_context[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE, EGL_NONE
    };

    EGLint attr_list[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, EGL_DONT_CARE,
        EGL_DEPTH_SIZE, 16,
        EGL_NONE
    };

    fd = open("/dev/dri/card0", O_RDWR);

    gbm = gbm_create_device(fd);

    display = eglGetDisplay(gbm);

    result = eglInitialize(display, &major_v, &minor_v);
    if (result == EGL_FALSE) {
        fprintf(stderr, "eglInitialize failed\n");
        return 1;
    }

    version = eglQueryString(display, EGL_VERSION);
    extensions = eglQueryString(display, EGL_EXTENSIONS);

    fprintf(stderr, "extensions: %s\n", extensions);
    fprintf(stderr, "version: %s\n", version);

    //if (!strstr(extensions, "EGL_KHR_surfaceless_opengl")) {
    //    fprintf(stderr, "no surfaceless support, cannot initialize\n");
    //    exit(-1);
    //}

    drmModeRes *resources;
    drmModeConnector *connector;
    drmModeEncoder *encoder;
    drmModeModeInfo mode;
    int i;
 
    /* Find the first available connector with modes */
 
    resources = drmModeGetResources(fd);
    if (!resources) {
        fprintf(stderr, "drmModeGetResources failed\n");
        return 1;
    }

    for (i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(fd, resources->connectors[i]);
        if (connector == NULL)
            continue;

        if (connector->connection == DRM_MODE_CONNECTED &&
            connector->count_modes > 0)
            break;

        drmModeFreeConnector(connector);
    }

    if (i == resources->count_connectors) {
        fprintf(stderr, "No currently active connector found.\n");
        return 1;
    }

    for (i = 0; i < resources->count_encoders; i++) {
        encoder = drmModeGetEncoder(fd, resources->encoders[i]);

        if (encoder == NULL)
            continue;

        if (encoder->encoder_id == connector->encoder_id)
            break;

        drmModeFreeEncoder(encoder);
    }

    mode = connector->modes[0];

    fprintf(stderr, "W: %d, H: %d\n", mode.hdisplay, mode.vdisplay);

    result = eglGetConfigs(display, NULL, 0, &num_configs);
    if (result == EGL_FALSE) {
        fprintf(stderr, "eglGetConfigs failed (getting the number of configs)\n");
        return 1;
    }

    result = eglChooseConfig(display, attr_list, &config, 1, &num_configs);
    if (result == EGL_FALSE) {
        fprintf(stderr, "eglChooseConfig failed\n");
        return 1;
    }

    eglBindAPI(EGL_OPENGL_API);
    context = eglCreateContext(display, NULL, EGL_NO_CONTEXT, attr_context);
    if (context == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext failed\n");
        return 1;
    }

    result = eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);
    if (result == EGL_FALSE) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        return 1;
    }



/*
    int minor_v, major_v;
    uint32_t width, height;
    int32_t success;

    EGLBoolean result;
    EGLDisplay display;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (display == EGL_NO_DISPLAY) {
        fprintf(stderr, "eglGetDisplay failed\n");
        return 1;
    }

    result = eglInitialize(display, &major_v, &minor_v);

    if (result == EGL_FALSE) {
        fprintf(stderr, "eglInitialize failed\n");
        return 1;
    }

    fprintf(stderr, "EGL version %d.%d\n", major_v, minor_v);

    result = eglGetConfigs(display, NULL, 0, &num_configs);

    if (result == EGL_FALSE) {
        fprintf(stderr, "eglGetConfigs failed (getting the number of configs)\n");
        return 1;
    }

    result = eglChooseConfig(display, attr_list, &config, 1, &num_configs);

    if (result == EGL_FALSE) {
        fprintf(stderr, "eglChooseConfig failed\n");
        return 1;
    }

    surface = eglCreateWindowSurface(display, config, window, NULL);

    if (surface == EGL_NO_SURFACE) {
        fprintf(stderr, "eglCreateWindowSurface failed\n");
        return 1;
    }

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attr_context);

    if (context == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext failed\n");
        return 1;
    }

    result = eglMakeCurrent(display, surface, surface, context);

    if (result == EGL_FALSE) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        return 1;
    }

    const GLubyte* extensions = glGetString(GL_EXTENSIONS);
    fprintf(stderr, "extensions: %s\n", extensions);

    const GLubyte* shaderv = glGetString(GL_SHADING_LANGUAGE_VERSION);
    fprintf(stderr, "shaderv: %s\n", shaderv);

    double physical_width = 0.7;

    vrms_server_socket_init((int)width, (int)height, physical_width);

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = INNER_LOOP_INTERVAL_MS;

    while (GL_TRUE) {
        vrms_server_socket_display();
        eglSwapBuffers(display, surface);
        vrms_server_socket_process();
        nanosleep(&ts, NULL);
    }

    vrms_server_socket_end();

*/

    return 0;
}
