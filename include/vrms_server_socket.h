#include "vrms_object.h"
#include "vrms_server.h"
vrms_server_t* vrms_server_socket_init(int width, int height, double physical_width);
void vrms_server_socket_display(vrms_server_t* vrms_server);
void vrms_server_socket_reshape(vrms_server_t* vrms_server, int w, int h);
void vrms_server_socket_process(vrms_server_t* vrms_server);
void vrms_server_socket_end(vrms_server_t* vrms_server);
