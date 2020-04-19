#include <stdint.h>
#include "runtime.h"
#include "udm.h"

void process_event(udm_device_t* device, const char* action, void *user_data) {
    vrms_module_t* module = (vrms_module_t*)user_data;
    module->interface.debug(module, "action: %s, device: %s", action, device->devnode);
}

void* run_module(vrms_module_t* module) {
    udm_context_t udm;

    if (!udm_init(&udm, "drm")) {
        return NULL;
    }
    udm_set_select_interval(&udm, 1, 0);
    udm_set_callback(&udm, process_event, (void*)module);

    module->interface.debug(module, "initialized");

    while (1) {
        udm_process_events(&udm);
    }

    return NULL;
}
