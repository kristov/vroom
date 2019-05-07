#ifndef DRM_CONTEXT_H
#define DRM_CONTEXT_H

#include <stdint.h>

struct drm_context {
    int fd;
    uint32_t connector_id;
    uint32_t crtc_id;
    drmModeModeInfo* mode;
};

#endif
