typedef struct vrms_server vrms_server_t;
typedef struct vrms_runtime {
    pthread_t module_threads[10];
    vrms_server_t* vrms_server;
    char* module_load_path;
} vrms_runtime_t;
vrms_runtime_t* vrms_runtime_init(int width, int height, double physical_width);
void vrms_runtime_display(vrms_runtime_t* vrms_runtime);
void vrms_runtime_reshape(vrms_runtime_t* vrms_runtime, int w, int h);
void vrms_runtime_process(vrms_runtime_t* vrms_runtime);
void vrms_runtime_end(vrms_runtime_t* vrms_runtime);
