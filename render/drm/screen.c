#include <stdint.h>
#include "screen.h"

uint8_t screen_render(struct screen* screen) {
    if (!eglMakeCurrent(screen->egl_display, screen->egl_surface, screen->egl_surface, screen->egl_context)) {
        return 1;
    }

    // DRAW CALLBACK

    eglSwapBuffers(context->egl_display, context->egl_surface);

    struct gbm_bo* bo = gbm_surface_lock_front_buffer(context->gbm_surface);
    uint32_t handle = gbm_bo_get_handle(bo).u32;
    uint32_t pitch = gbm_bo_get_stride(bo);

    uint32_t fb;
    drmModeAddFB(screen->drm->fd, screen->drm->width, screen->drm->height, 24, 32, pitch, handle, &fb);
    drmModeSetCrtc(screen->drm->fd, screen->drm->crtc_id, fb, 0, 0, &screen->drm->connector_id, 1, &screen->drm->mode);

    if (screen->previous_bo) {
        drmModeRmFB(context->drm->fd, screen->previous_fb);
        gbm_surface_release_buffer(screen->gbm_surface, screen->previous_bo);
    }
    screen->previous_bo = bo;
    screen->previous_fb = fb;
    return 0;
}

uint8_t screen_init_egl(struct screen* screen) {
    EGLint major_v;
    EGLint minor_v;
    EGLint attributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };
    EGLConfig config;
    EGLint num_config;

    screen->egl_display = eglGetDisplay(screen->gbm);

    int ret = eglInitialize(screen->egl_display, &major_v, &minor_v);
    if (ret == EGL_FALSE) {
        return 1;
    }

    const char* version = eglQueryString(screen->egl_display, EGL_VERSION);
    const char* extensions = eglQueryString(screen->egl_display, EGL_EXTENSIONS);

    fprintf(stderr, "extensions: %s\n", extensions);
    fprintf(stderr, "version: %s\n", version);

    eglBindAPI(EGL_OPENGL_API);

    eglChooseConfig(screen->egl_display, attributes, &config, 1, &num_config);
    screen->egl_context = eglCreateContext(screen->egl_display, config, EGL_NO_CONTEXT, NULL);
    if (!screen->egl_context) {
        fprintf(stderr, "failed to create context\n");
        eglTerminate(screen->egl_display);
        return 1;
    }

    screen->gbm_surface = gbm_surface_create(screen->gbm, screen->drm->width, screen->drm->height, GBM_BO_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT|GBM_BO_USE_RENDERING);
    screen->egl_surface = eglCreateWindowSurface(screen->egl_display, config, screen->gbm_surface, NULL);

    if (!screen->egl_surface) {
        eglMakeCurrent(screen->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(screen->egl_display, context->egl_context);
        eglTerminate(screen->egl_display);
        return 1;
    }

    return 0;
}

uint8_t screen_init_gbm(struct screen* screen) {
    screen->gbm = gbm_create_device(screen->drm->fd);
    if (!screen->gbm) {
        return 1;
    }
    if (screen_init_egl(screen)) {
        gbm_device_destroy(context->gbm);
        return 1;
    }
    return 0;
}

uint8_t screen_init(struct screen* screen, struct drm_context* drm) {
    screen->drm = drm;
    screen_init_gbm(screen);
}

uint8_t screen_deinit(struct screen* screen) {
    eglMakeCurrent(screen->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(screen->egl_display, context->egl_context);
    eglTerminate(screen->egl_display);
    gbm_device_destroy(context->gbm);
}
