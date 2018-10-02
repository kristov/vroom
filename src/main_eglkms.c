#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <gbm.h>
#include <libdrm/drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "vrms_runtime.h"

vrms_runtime_t* vrms_runtime;

#define NANO_SECOND_MULTIPLIER 1000000
const long INNER_LOOP_INTERVAL_MS = 50 * NANO_SECOND_MULTIPLIER;

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

// Hack to render to Oculus rift (0 width, 0 height reported by DRM connector)
#define MAX_WIDTH_MM_NON_HMS 200

typedef struct eglkms_context {
    int fd;
    struct gbm_device* gbm;
    struct gbm_surface* gbm_surface;
    GLuint color_rb[2];
    uint16_t width;
    uint16_t height;
    drmModeConnector* kms_connector;
    drmModeEncoder* kms_encoder;
    drmModeModeInfo kms_mode;
    drmModeCrtc* kms_crtc;
    uint32_t kms_fb_id[2];
    GLuint fb;
    struct gbm_bo* buff_obj[2];
    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLSurface egl_surface;
//    EGLImageKHR khr_image[2];
} eglkms_context_t;

//static void page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data) {
//    ;
//}

void render_loop(eglkms_context_t* context) {
    struct gbm_bo *previous_bo;
    uint32_t previous_fb;
    uint8_t quit;
    struct gbm_bo* bo;
    uint32_t handle;
    uint32_t pitch;
    uint32_t fb;
    struct timespec ts;

    ts.tv_sec = 0;
    ts.tv_nsec = INNER_LOOP_INTERVAL_MS;

    double physical_width = 0.7;
    vrms_runtime = vrms_runtime_init(context->width, context->height, physical_width);

    quit = 0;
    do {
        vrms_runtime_display(vrms_runtime);
        eglSwapBuffers(context->egl_display, context->egl_surface);
        bo = gbm_surface_lock_front_buffer(context->gbm_surface);
        handle = gbm_bo_get_handle(bo).u32;
        pitch = gbm_bo_get_stride(bo);

        drmModeAddFB(context->fd, context->width, context->height, 24, 32, pitch, handle, &fb);
        drmModeSetCrtc(context->fd, context->kms_crtc->crtc_id, fb, 0, 0, &context->kms_connector->connector_id, 1, &context->kms_mode);

        if (previous_bo) {
            drmModeRmFB(context->fd, previous_fb);
            gbm_surface_release_buffer(context->gbm_surface, previous_bo);
        }
        previous_bo = bo;
        previous_fb = fb;
        vrms_runtime_process(vrms_runtime);
        nanosleep(&ts, NULL);
    } while (!quit);
}

void drm_crtc(eglkms_context_t* context) {
    drmModeCrtcPtr saved_crtc;
    int ret;

    saved_crtc = drmModeGetCrtc(context->fd, context->kms_encoder->crtc_id);
    if (saved_crtc == NULL) {
        fprintf(stderr, "failed to save crtc\n");
        return;
    }

    render_loop(context);

    ret = drmModeSetCrtc(context->fd, saved_crtc->crtc_id, saved_crtc->buffer_id, saved_crtc->x, saved_crtc->y, &context->kms_connector->connector_id, 1, &saved_crtc->mode);
    if (ret) {
        fprintf(stderr, "failed to restore crtc\n");
        return;
    }
    drmModeFreeCrtc(saved_crtc);
}

void egl_context(eglkms_context_t* context) {
    EGLint attributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };
    EGLConfig config;
    EGLint num_config;

    eglBindAPI(EGL_OPENGL_API);

    eglChooseConfig(context->egl_display, attributes, &config, 1, &num_config);
    context->egl_context = eglCreateContext(context->egl_display, config, EGL_NO_CONTEXT, NULL);
    if (context->egl_context == NULL) {
        fprintf(stderr, "failed to create context\n");
        return;
    }

    context->gbm_surface = gbm_surface_create (context->gbm, context->width, context->height, GBM_BO_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT|GBM_BO_USE_RENDERING);
    context->egl_surface = eglCreateWindowSurface(context->egl_display, config, context->gbm_surface, NULL);
    
    if (!eglMakeCurrent(context->egl_display, context->egl_surface, context->egl_surface, context->egl_context)) {
        fprintf(stderr, "failed to make context current\n");
        return;
    }

    drm_crtc(context);

    eglMakeCurrent(context->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(context->egl_display, context->egl_context);
}

void kms_drm(eglkms_context_t* context) {
    drmModeRes *resources;
    drmModeConnector *connector;
    drmModeEncoder *encoder;
    uint8_t i;

    resources = drmModeGetResources(context->fd);
    if (!resources) {
        fprintf(stderr, "drmModeGetResources failed\n");
        return;
    }
    for (i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(context->fd, resources->connectors[i]);

        if (NULL == connector) {
            continue;
        }

        if ((connector->connection != DRM_MODE_CONNECTED) || (connector->count_modes == 0)) {
            drmModeFreeConnector(connector);
            continue;
        }

        if (connector->mmWidth > MAX_WIDTH_MM_NON_HMS) {
            drmModeFreeConnector(connector);
            continue;
        }

        fprintf(stderr, "connector: %d\n", i);
        fprintf(stderr, "     connector_type: %d\n", connector->connector_type);
        fprintf(stderr, "         connection: %d\n", connector->connection);
        fprintf(stderr, "        count_modes: %d\n", connector->count_modes);
        fprintf(stderr, "            mmWidth: %d\n", connector->mmWidth);
        fprintf(stderr, "           mmHeight: %d\n", connector->mmHeight);
        if (NULL == context->kms_connector) {
            context->kms_connector = connector;
        }
        else {
            drmModeFreeConnector(connector);
        }
    }
    if (NULL == context->kms_connector) {
        fprintf(stderr, "No currently active connector found.\n");
        return;
    }

    debug_print("Found connector\n");

    for (i = 0; i < resources->count_encoders; i++) {
        encoder = drmModeGetEncoder(context->fd, resources->encoders[i]);
        if (encoder == NULL)
            continue;
        if (encoder->encoder_id == context->kms_connector->encoder_id)
            break;
        drmModeFreeEncoder(encoder);
    }

    debug_print("Found encoder\n");

    if (encoder->crtc_id) {
        context->kms_crtc = drmModeGetCrtc(context->fd, encoder->crtc_id);
    }

    context->kms_encoder = encoder;
    context->kms_mode = context->kms_connector->modes[0];
    
    context->width = context->kms_mode.hdisplay;
    context->height = context->kms_mode.vdisplay;

    debug_print("Using mode %dx%d\n", context->width, context->height);

    egl_context(context);

    drmModeFreeConnector(context->kms_connector);
    drmModeFreeEncoder(encoder);
}

void egl_root(eglkms_context_t* context) {
    int ret;
    const char *version;
    const char *extensions;
    EGLint major_v;
    EGLint minor_v;

    context->egl_display = eglGetDisplay(context->gbm);

    ret = eglInitialize(context->egl_display, &major_v, &minor_v);
    if (ret == EGL_FALSE) {
        fprintf(stderr, "eglInitialize failed\n");
        return;
    }

    version = eglQueryString(context->egl_display, EGL_VERSION);
    extensions = eglQueryString(context->egl_display, EGL_EXTENSIONS);

    fprintf(stderr, "extensions: %s\n", extensions);
    fprintf(stderr, "version: %s\n", version);

    kms_drm(context);

    eglTerminate(context->egl_display);
}

void gbm_device(eglkms_context_t* context) {
    context->gbm = gbm_create_device(context->fd);
    if (context->gbm == NULL) {
        fprintf(stderr, "couldn't create gbm device\n");
        return;
    }
    egl_root(context);
    gbm_device_destroy(context->gbm);
}

void device_filehandle(eglkms_context_t* context) {
    context->fd = open("/dev/dri/card0", O_RDWR);
    if (context->fd < 0) {
        /* Probably permissions error */
        fprintf(stderr, "couldn't open, skipping\n");
        return;
    }

    gbm_device(context);

    close(context->fd);
}

int32_t main(int argc, char **argv) {
    eglkms_context_t* context;

    context = malloc(sizeof(eglkms_context_t));
    if (context == NULL) {
        return 1;
    }

    device_filehandle(context);

    return 0;
}

