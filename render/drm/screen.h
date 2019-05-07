#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include "drm_context.h"

struct screen {
    struct drm_context* drm;
    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLSurface egl_surface;
    struct gbm_surface* gbm_surface;
    uint32_t previous_fb;
    struct gbm_bo* previous_bo;
};

uint8_t screen_render(struct screen* screen);

uint8_t screen_init(struct screen* screen, struct drm_context* drm);

uint8_t screen_deinit(struct screen* screen);

#endif
