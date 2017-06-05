#include <openhmd/openhmd.h>

typedef struct vrms_hmd {
    ohmd_context* ohmd_ctx;
    ohmd_device* ohmd_active_hmd;
    pthread_mutex_t* matrix_lock;
    float* matrix;
} vrms_hmd_t;

vrms_hmd_t* vrms_hmd_create();
int32_t vrms_hmd_init(vrms_hmd_t* hmd);
void vrms_hmd_run(vrms_hmd_t* hmd);
