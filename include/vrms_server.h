typedef struct vrms_scene vrms_scene_t;
typedef struct vrms_queue_item vrms_queue_item_t;
typedef struct vrms_server {
    uint32_t next_scene_id;
    vrms_scene_t** scenes;
    vrms_queue_item_t** inbound_queue;
    uint32_t inbound_queue_index;
    pthread_mutex_t* inbound_queue_lock;
} vrms_server_t;

vrms_server_t* vrms_server_create();
vrms_scene_t* vrms_server_get_scene(vrms_server_t* vrms_server, uint32_t scene_id);
uint32_t vrms_server_create_scene(vrms_server_t* vrms_server, char* name);
void vrms_server_destroy_scene(vrms_server_t* server, uint32_t scene_id);
void vrms_server_draw_scenes(vrms_server_t* vrms_server, GLuint shader_id, GLfloat* projection_matrix, GLfloat* view_matrix, GLfloat* model_matrix);
void vrms_queue_item_process(vrms_queue_item_t* queue_item);
void vrms_server_process_queue(vrms_server_t* server);
