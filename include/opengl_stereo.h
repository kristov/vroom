typedef struct opengl_stereo_camera {
    float* projection_matrix;
    GLfloat model_translation;
} opengl_stereo_camera;

typedef struct opengl_stereo_buffer_store {
    GLuint buffer;
    GLuint rendered_texture;
} opengl_stereo_buffer_store;

typedef struct opengl_stereo opengl_stereo;

typedef void (*ostereo_draw_scene_callback_t)(opengl_stereo* ostereo);

typedef struct opengl_stereo {
    double width;
    double height;
    double depthZ;
    double fovy;
    double aspect;
    double nearZ;
    double farZ;
    double screenZ;
    double IOD;
    double physical_width;
    double texture_shift;
    GLuint screen_plane_vdb;
    GLuint screen_plane_idb;
    GLuint screen_text_offset;
    float* screen_matrix;
    GLuint screen_shader_program_id;
    GLuint onecolor_shader_id;
    GLuint texture_shader_id;
    GLuint cubemap_shader_id;
    float* model_matrix;
    float* view_matrix;
    float* hmd_matrix;
    float* projection_matrix;
    ostereo_draw_scene_callback_t draw_scene_callback;
    GLuint barrel_power_id;
    opengl_stereo_camera* left_camera;
    opengl_stereo_camera* right_camera;
    opengl_stereo_camera* skybox_camera;
    opengl_stereo_buffer_store* screen_buffers;
} opengl_stereo;

void opengl_stereo_reshape(opengl_stereo* ostereo, int w, int h);
void opengl_stereo_display(opengl_stereo* ostereo);
opengl_stereo* opengl_stereo_create(int width, int height, double physical_width);

double opengl_stereo_get_config_value(opengl_stereo* ostereo, char* name);
void opengl_stereo_set_config_value(opengl_stereo* ostereo, char* name, double value);
