uint32_t vrms_geometry_cube(vrms_client_t* client, uint32_t x, uint32_t y, uint32_t z, float r, float g, float b, float a);
uint32_t vrms_geometry_cube_textured(vrms_client_t* client, uint32_t x, uint32_t y, uint32_t z, const char* filename);
uint32_t vrms_geometry_plane(vrms_client_t* client, uint32_t x, uint32_t y, float r, float g, float b, float a);
uint32_t vrms_geometry_plane_textured(vrms_client_t* client, uint32_t x, uint32_t y, const char* filename);
uint32_t vrms_geometry_load_matrix_data(vrms_client_t* client, uint32_t nr_matricies, float* matrix_data);
uint32_t vrms_geometry_render_buffer_set(vrms_client_t* client, uint32_t nr_items, uint32_t* render_buffer);
