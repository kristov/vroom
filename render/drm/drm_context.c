#include <stdint.h>
#include "drm_context.h"

void drm_context_add_screen(struct drm_context_t* context, drmModeConnector* connector) {
    struct screen* screen = &context->screens[context->screen_idx];
    screen->connector_id = connector->connector_id;
    context->screen_idx++;
}

void drm_context_scan(drm_context_t* context) {
    drmModeRes *resources;
    drmModeConnector *connector;
    drmModeEncoder *encoder;
    uint8_t i;

    resources = drmModeGetResources(context->fd);
    if (!resources) {
        fprintf(stderr, "drmModeGetResources failed\n");
        return 1;
    }
    for (i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(context->fd, resources->connectors[i]);

        if (!connector) {
            continue;
        }

        if ((connector->connection != DRM_MODE_CONNECTED) || (connector->count_modes == 0)) {
            drmModeFreeConnector(connector);
            continue;
        }

        fprintf(stderr, "connector: %d\n", i);
        fprintf(stderr, "     connector_type: %d\n", connector->connector_type);
        fprintf(stderr, "         connection: %d\n", connector->connection);
        fprintf(stderr, "        count_modes: %d\n", connector->count_modes);
        fprintf(stderr, "            mmWidth: %d\n", connector->mmWidth);
        fprintf(stderr, "           mmHeight: %d\n", connector->mmHeight);

        drm_context_add_screen(context, connector);
    }

    if (!context->kms_connector) {
        fprintf(stderr, "No currently active connector found.\n");
        return 1;
    }

    debug_print("Found connector\n");

    for (i = 0; i < resources->count_encoders; i++) {
        encoder = drmModeGetEncoder(context->fd, resources->encoders[i]);
        if (!encoder)
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

    gbm_device(context);

    drmModeFreeConnector(context->kms_connector);
    drmModeFreeEncoder(encoder);
}

uint8_t drm_context_card(struct drm_context_t* context, const char* device) {
    context->fd = open(device, O_RDWR);
    if (context->fd < 0) {
        fprintf(stderr, "couldn't open, skipping\n");
        return 1;
    }

    if (drm_context_scan(context)) {
        close(context->fd);
        return 1;
    }

    return 0;
}

uint8_t drm_context_init(
    return drm_context_card("/dev/dri/card0");
}
