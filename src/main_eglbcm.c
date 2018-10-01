#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef RASPBERRYPI
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#else /* not RASPBERRYPI */
#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#endif /* RASPBERRYPI */
#include "bcm_host.h"
#include "vrms_server_socket.h"

vrms_server_t* vrms_server;

#define NANO_SECOND_MULTIPLIER 1000000
const long INNER_LOOP_INTERVAL_MS = 50 * NANO_SECOND_MULTIPLIER;

int32_t main(int argc, char **argv) {
    int minor_v, major_v;
    uint32_t width, height;
    int32_t success;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;
    EGL_DISPMANX_WINDOW_T dispman_window;

    EGLBoolean result;
    EGLDisplay display;
    EGLint num_configs;
    EGLConfig config;
    EGLSurface surface;
    EGLNativeWindowType window;
    EGLContext context;

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

    bcm_host_init();
    success = graphics_get_display_size( 0, &width, &height);

    if (success < 0) {
        fprintf(stderr, "graphics_get_display_size failed (%d)\n", success);
        return 1;
    }

    fprintf(stderr, "size: [%dx%d]\n", width, height);

    vc_dispmanx_rect_set(&dst_rect, 0, 0, width, height);
    vc_dispmanx_rect_set(&src_rect, 0, 0, width, height);

    dispman_display = vc_dispmanx_display_open(0);
    dispman_update = vc_dispmanx_update_start(0);
    dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display, 0, &dst_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, 0, 0, 0);

    dispman_window.element = dispman_element;
    dispman_window.width = width;
    dispman_window.height = height;

    vc_dispmanx_update_submit_sync(dispman_update);

    window = &dispman_window;

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

    vrms_server = vrms_server_socket_init((int)width, (int)height, physical_width);

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = INNER_LOOP_INTERVAL_MS;

    while (GL_TRUE) {
        vrms_server_socket_display(vrms_server);
        eglSwapBuffers(display, surface);
        vrms_server_socket_process(vrms_server);
        nanosleep(&ts, NULL);
    }

    vrms_server_socket_end(vrms_server);
    return 0;
}
