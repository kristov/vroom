#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "vrms_gl.h"
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ev.h>
#include <unistd.h>
#include "array_heap.h"
#include "vroom_pb.h"
#include "safe_malloc.h"
#include "vrms_render_vm.h"
#include "vrms_object.h"
#include "vrms_server.h"
#include "vrms_scene.h"
#include "esm.h"
#include "opengl_stereo.h"
#include "vrms.h"

#define DEBUG 1
#define debug_print(fmt, ...) do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)

#define MAX_MSG_SIZE 1024

opengl_stereo* ostereo;
vrms_server_t* vrms_server;

pthread_t socket_thread;

struct sock_ev_serv {
    ev_io io;
    int fd;
    struct sockaddr_un socket;
    int socket_len;
    array clients;
    vrms_server_t* vrms_server;
};

struct sock_ev_client {
    ev_io io;
    int fd;
    int index;
    struct sock_ev_serv* server;
    uint32_t vrms_scene_id;
};

uint32_t receive_create_scene(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateScene* cs_msg;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    cs_msg = create_scene__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    id = vrms_server_create_scene(vrms_server, cs_msg->name);
    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create scene: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_memory(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error, int shm_fd) {
    uint32_t id;
    CreateMemory* cs_msg;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    cs_msg = create_memory__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, cs_msg->scene_id);
    if (NULL != vrms_scene) {
        id = vrms_scene_create_memory(vrms_scene, shm_fd, cs_msg->size);
    }

    if (0 == id) {
        fprintf(stderr, "create memory: out of memory\n");
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_data_object(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateDataObject* msg;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = create_data_object__unpack(NULL, length, in_buf);
    if (msg == NULL) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    vrms_data_type_t vrms_type;
    switch (msg->type) {
        case CREATE_DATA_OBJECT__TYPE__UV:
            debug_print("received data type: VRMS_UV\n");
            vrms_type = VRMS_UV;
            break;
        case CREATE_DATA_OBJECT__TYPE__COLOR:
            debug_print("received data type: VRMS_COLOR\n");
            vrms_type = VRMS_COLOR;
            break;
        case CREATE_DATA_OBJECT__TYPE__VERTEX:
            debug_print("received data type: VRMS_VERTEX\n");
            vrms_type = VRMS_VERTEX;
            break;
        case CREATE_DATA_OBJECT__TYPE__NORMAL:
            debug_print("received data type: VRMS_NORMAL\n");
            vrms_type = VRMS_NORMAL;
            break;
        case CREATE_DATA_OBJECT__TYPE__INDEX:
            debug_print("received data type: VRMS_INDEX\n");
            vrms_type = VRMS_INDEX;
            break;
        case CREATE_DATA_OBJECT__TYPE__MATRIX:
            debug_print("received data type: VRMS_MATRIX\n");
            vrms_type = VRMS_MATRIX;
            break;
        case CREATE_DATA_OBJECT__TYPE__TEXTURE:
            debug_print("received data type: VRMS_TEXTURE\n");
            vrms_type = VRMS_TEXTURE;
            break;
        case CREATE_DATA_OBJECT__TYPE__PROGRAM:
            debug_print("received data type: VRMS_PROGRAM\n");
            vrms_type = VRMS_PROGRAM;
            break;
        case CREATE_DATA_OBJECT__TYPE__REGISTER:
            debug_print("received data type: VRMS_REGISTER\n");
            vrms_type = VRMS_REGISTER;
            break;
        case _CREATE_DATA_OBJECT__TYPE_IS_INT_SIZE:
            break;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, msg->scene_id);
    if (NULL != vrms_scene) {
        id = vrms_scene_create_object_data(vrms_scene, msg->memory_id, msg->memory_offset, msg->memory_length, msg->item_length, msg->data_length, vrms_type);
    }

    if (0 == id) {
        fprintf(stderr, "create object data: out of memory\n");
        *error = VRMS_OUTOFMEMORY;
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_create_texture_object(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateTextureObject* msg;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = create_texture_object__unpack(NULL, length, in_buf);
    if (msg == NULL) {
        fprintf(stderr, "unpacking incoming message\n");
        *error = VRMS_INVALIDREQUEST;
        return 0;
    }

    vrms_texture_format_t format = msg->format;
    vrms_texture_type_t type = msg->type;

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, msg->scene_id);
    if (NULL != vrms_scene) {
        id = vrms_scene_create_object_texture(vrms_scene, msg->data_id, msg->width, msg->height, format, type);
    }

    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create object texture: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_create_geometry_object(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateGeometryObject* cs_msg;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    cs_msg = create_geometry_object__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, cs_msg->scene_id);
    if (NULL != vrms_scene) {
        id = vrms_scene_create_object_geometry(vrms_scene, cs_msg->vertex_id, cs_msg->normal_id, cs_msg->index_id);
    }

    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create object geometry: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_mesh_color(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateMeshColor* cs_msg;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    cs_msg = create_mesh_color__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, cs_msg->scene_id);
    if (NULL != vrms_scene) {
        id = vrms_scene_create_object_mesh_color(vrms_scene, cs_msg->geometry_id, cs_msg->r, cs_msg->g, cs_msg->b, cs_msg->a);
    }

    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create object mesh color: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_mesh_texture(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateMeshTexture* cs_msg;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    cs_msg = create_mesh_texture__unpack(NULL, length, in_buf);
    if (cs_msg == NULL) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, cs_msg->scene_id);
    if (NULL != vrms_scene) {
        id = vrms_scene_create_object_mesh_texture(vrms_scene, cs_msg->geometry_id, cs_msg->texture_id, cs_msg->uv_id);
    }

    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create object mesh texture: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(cs_msg);
    return id;
}

uint32_t receive_create_program(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateProgram* msg;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = create_program__unpack(NULL, length, in_buf);
    if (msg == NULL) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, msg->scene_id);
    if (NULL != vrms_scene) {
        id = vrms_scene_create_program(vrms_scene, msg->data_id);
    }

    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create program: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_run_program(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    RunProgram* msg;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = run_program__unpack(NULL, length, in_buf);
    if (msg == NULL) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, msg->scene_id);
    if (NULL != vrms_scene) {
        id = vrms_scene_run_program(vrms_scene, msg->program_id, msg->register_id);
    }

    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "run program: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

uint32_t receive_update_system_matrix(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t ok;
    UpdateSystemMatrix* msg;
    vrms_matrix_type_t matrix_type;
    vrms_update_type_t update_type;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = update_system_matrix__unpack(NULL, length, in_buf);
    if (msg == NULL) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    switch (msg->matrix_type) {
        case UPDATE_SYSTEM_MATRIX__MATRIX_TYPE__HEAD:
            matrix_type = VRMS_MATRIX_HEAD;
            break;
        case UPDATE_SYSTEM_MATRIX__MATRIX_TYPE__BODY:
            matrix_type = VRMS_MATRIX_BODY;
            break;
        case _UPDATE_SYSTEM_MATRIX__MATRIX_TYPE_IS_INT_SIZE:
            break;
    }

    switch (msg->update_type) {
        case UPDATE_SYSTEM_MATRIX__UPDATE_TYPE__MULTIPLY:
            update_type = VRMS_UPDATE_MULTIPLY;
            break;
        case UPDATE_SYSTEM_MATRIX__UPDATE_TYPE__SET:
            update_type = VRMS_UPDATE_SET;
            break;
        case _UPDATE_SYSTEM_MATRIX__UPDATE_TYPE_IS_INT_SIZE:
            break;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, msg->scene_id);
    if (NULL != vrms_scene) {
        ok = vrms_scene_update_system_matrix(vrms_scene, msg->data_id, msg->data_index, matrix_type, update_type);
    }


    if (0 == ok) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "set system matrix error\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return ok;
}

uint32_t receive_create_skybox(vrms_server_t* vrms_server, uint8_t* in_buf, int32_t length, vrms_error_t* error) {
    uint32_t id;
    CreateSkybox* msg;

    if (NULL == vrms_server) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "server not initialized\n");
        return 0;
    }

    msg = create_skybox__unpack(NULL, length, in_buf);
    if (msg == NULL) {
        *error = VRMS_INVALIDREQUEST;
        fprintf(stderr, "unpacking incoming message\n");
        return 0;
    }

    vrms_scene_t* vrms_scene = vrms_server_get_scene(vrms_server, msg->scene_id);
    if (NULL != vrms_scene) {
        id = vrms_scene_create_object_skybox(vrms_scene, msg->texture_id);
    }

    if (0 == id) {
        *error = VRMS_OUTOFMEMORY;
        fprintf(stderr, "create skybox: out of memory\n");
    }
    else {
        *error = VRMS_OK;
    }

    free(msg);
    return id;
}

void send_reply(int32_t fd, int32_t id, vrms_error_t* error) {
    void *out_buf;
    size_t length;
    Reply re_msg = REPLY__INIT;

    re_msg.id = id;
    length = reply__get_packed_size(&re_msg);
    out_buf = SAFEMALLOC(length);

    re_msg.error_code = (int32_t)*error;

    reply__pack(&re_msg, out_buf);
    if (send(fd, out_buf, length, 0) < 0) {
        fprintf(stderr, "error sending reply to client\n");
    }

    free(out_buf);
}

static void client_cb(EV_P_ ev_io *w, int revents) {
    struct msghdr msgh;
    struct iovec iov;
    union {
        struct cmsghdr cmsgh;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;
    struct cmsghdr *cmsgh;

    struct sock_ev_client* client = (struct sock_ev_client*) w;
    int32_t id = 0;
    int32_t shm_fd = 0;
    vrms_error_t error;
    uint8_t in_buf[MAX_MSG_SIZE];

    int32_t length_r;
    char type_c;
    vrms_server_t* vrms_server;

    if (NULL == client->server) {
        fprintf(stderr, "no server initialized\n");
        send_reply(client->fd, id, &error);
        return;
    }
    if (NULL == client->server->vrms_server) {
        fprintf(stderr, "no VRMS server initialized\n");
        send_reply(client->fd, id, &error);
    }
    vrms_server = client->server->vrms_server;

    length_r = recv(client->fd, &type_c, 1, 0);
    if (length_r <= 0) {
        if (0 == length_r) {
            printf("orderly disconnect\n");
            if (client->vrms_scene_id > 0) {
                vrms_server_destroy_scene(vrms_server, client->vrms_scene_id);
            }
            ev_io_stop(EV_A_ &client->io);
            close(client->fd);
        }
        else if (EAGAIN == errno) {
            printf("should never get in this state with libev\n");
        }
        else {
            perror("recv\n");
        }
        return;
    }
    vrms_type_t type = (vrms_type_t)type_c;

    iov.iov_base = in_buf;
    iov.iov_len = MAX_MSG_SIZE;

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;
    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    msgh.msg_control = control_un.control;
    msgh.msg_controllen = sizeof(control_un.control);

    length_r = recvmsg(client->fd, &msgh, 0);
    if (-1 == length_r) {
        fprintf(stderr, "receiving control fd\n");
        send_reply(client->fd, id, &error);
        return;
    }

    if (MAX_MSG_SIZE == length_r) {
        fprintf(stderr, "maximum message length exceeded\n");
        send_reply(client->fd, id, &error);
        return;
    }

    cmsgh = CMSG_FIRSTHDR(&msgh);
    if (!cmsgh) {
        fprintf(stderr, "expected one recvmsg header with an fd but got zero headers\n");
        send_reply(client->fd, id, &error);
        return;
    }

    if (cmsgh->cmsg_level != SOL_SOCKET) {
        fprintf(stderr, "invalid cmsg_level\n");
        send_reply(client->fd, id, &error);
        return;
    }

    if (cmsgh->cmsg_type != SCM_RIGHTS) {
        fprintf(stderr, "invalid cmsg_type\n");
        send_reply(client->fd, id, &error);
        return;
    }

    shm_fd = *((int *)CMSG_DATA(cmsgh));

    switch (type) {
        case VRMS_REPLY:
            error = VRMS_INVALIDREQUEST;
            fprintf(stderr, "received a reply message as a request\n");
            break;
        case VRMS_CREATESCENE:
            if (client->vrms_scene_id > 0) {
                error = VRMS_INVALIDREQUEST;
                fprintf(stderr, "connection already associated with a scene\n");
                id = 0;
            }
            else {
                id = receive_create_scene(vrms_server, in_buf, length_r, &error);
                if (id > 0) {
                    client->vrms_scene_id = id;
                }
            }
            break;
        case VRMS_DESTROYSCENE:
            break;
        case VRMS_CREATEMEMORY:
            id = receive_create_memory(vrms_server, in_buf, length_r, &error, shm_fd);
            break;
        case VRMS_CREATEDATAOBJECT:
            id = receive_create_data_object(vrms_server, in_buf, length_r, &error);
            break;
        case VRMS_DESTROYDATAOBJECT:
            break;
        case VRMS_CREATETEXTUREOBJECT:
            id = receive_create_texture_object(vrms_server, in_buf, length_r, &error);
            break;
        case VRMS_DESTROYTEXTUREOBJECT:
            break;
        case VRMS_CREATEGEOMETRYOBJECT:
            id = receive_create_geometry_object(vrms_server, in_buf, length_r, &error);
            break;
        case VRMS_CREATEMESHCOLOR:
            id = receive_create_mesh_color(vrms_server, in_buf, length_r, &error);
            break;
        case VRMS_CREATEMESHTEXTURE:
            id = receive_create_mesh_texture(vrms_server, in_buf, length_r, &error);
            break;
        case VRMS_CREATEPROGRAM:
            id = receive_create_program(vrms_server, in_buf, length_r, &error);
            break;
        case VRMS_RUNPROGRAM:
            id = receive_run_program(vrms_server, in_buf, length_r, &error);
            break;
        case VRMS_UPDATESYSTEMMATRIX:
            id = receive_update_system_matrix(vrms_server, in_buf, length_r, &error);
            break;
        case VRMS_CREATESKYBOX:
            id = receive_create_skybox(vrms_server, in_buf, length_r, &error);
            break;
        default:
            id = 0;
            error = VRMS_INVALIDREQUEST;
            fprintf(stderr, "unknown error\n");
            break;
    }

    send_reply(client->fd, id, &error);
}

int setnonblock(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

inline static struct sock_ev_client* client_new(int fd) {
    struct sock_ev_client* client;

    client = realloc(NULL, sizeof(struct sock_ev_client));
    client->fd = fd;
    setnonblock(client->fd);
    ev_io_init(&client->io, client_cb, client->fd, EV_READ);

    return client;
}

static void server_cb(EV_P_ ev_io *w, int revents) {
    fprintf(stderr, "unix stream socket has become readable\n");

    int client_fd;
    struct sock_ev_client* client;
    struct sock_ev_serv* server = (struct sock_ev_serv*) w;

    while (1) {
        client_fd = accept(server->fd, NULL, NULL);
        if( client_fd == -1 ) {
            if( errno != EAGAIN && errno != EWOULDBLOCK ) {
                fprintf(stderr, "accept() failed errno=%i (%s)\n",  errno, strerror(errno));
                exit(1);
            }
            break;
        }
        printf("accepted a client\n");
        client = client_new(client_fd);
        client->vrms_scene_id = 0;
        client->server = server;
        client->index = array_push(&server->clients, client);
        ev_io_start(EV_A_ &client->io);
    }
}

int unix_socket_init(struct sockaddr_un* socket_un, char* sock_path, int max_queue) {
    int fd;

    unlink(sock_path);

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == fd) {
        fprintf(stderr, "error creating server socket\n");
        exit(1);
    }

    if (-1 == setnonblock(fd)) {
        fprintf(stderr, "error making server socket nonblocking\n");
        exit(1);
    }

    socket_un->sun_family = AF_UNIX;
    strcpy(socket_un->sun_path, sock_path);

    return fd;
}

int server_init(struct sock_ev_serv* server, char* sock_path, int max_queue) {
    server->fd = unix_socket_init(&server->socket, sock_path, max_queue);
    server->socket_len = sizeof(server->socket.sun_family) + strlen(server->socket.sun_path);

    array_init(&server->clients, 128);

    if (-1 == bind(server->fd, (struct sockaddr*) &server->socket, server->socket_len))
    {
      fprintf(stderr, "error server bind\n");
      exit(1);
    }

    if (-1 == listen(server->fd, max_queue)) {
      fprintf(stderr, "error listen\n");
      exit(1);
    }
    return 0;
}

void* start_socket_thread(void* ptr) {
    int max_queue = 128;
    struct sock_ev_serv server;
    EV_P = ev_default_loop(0);

    server_init(&server, "/tmp/libev-echo.sock", max_queue);

    if (NULL == vrms_server) {
        fprintf(stderr, "error creating server\n");
        return NULL;
    }
    server.vrms_server = vrms_server;

    ev_io_init(&server.io, server_cb, server.fd, EV_READ);
    ev_io_start(EV_A_ &server.io);

    ev_loop(EV_A_ 0);

    close(server.fd);
    return NULL;
}

void draw_scene(opengl_stereo* ostereo) {
    if (NULL != vrms_server) {
        vrms_server_draw_scenes(vrms_server, ostereo->projection_matrix, ostereo->view_matrix, ostereo->model_matrix, ostereo->skybox_camera->projection_matrix);
    }
}

void vrms_server_socket_system_matrix_update(vrms_matrix_type_t matrix_type, vrms_update_type_t update_type, float* matrix) {
    memcpy(ostereo->hmd_matrix, matrix, sizeof(float) * 16);
}

void vrms_server_socket_init(int width, int height, double physical_width) {
    int32_t thread_ret;

    ostereo = opengl_stereo_create(width, height, physical_width);
    ostereo->draw_scene_callback = &draw_scene;

    vrms_server = vrms_server_create();
    vrms_server->color_shader_id = ostereo->onecolor_shader_id;
    vrms_server->texture_shader_id = ostereo->texture_shader_id;
    vrms_server->cubemap_shader_id = ostereo->cubemap_shader_id;
    vrms_server->system_matrix_update = vrms_server_socket_system_matrix_update;

    thread_ret = pthread_create(&socket_thread, NULL, start_socket_thread, NULL);
    if (thread_ret != 0) {
        fprintf(stderr, "unable to start socket thread\n");
        exit(1);
    }
}

void vrms_server_socket_display() {
    opengl_stereo_display(ostereo);
}

void vrms_server_socket_reshape(int w, int h) {
    opengl_stereo_reshape(ostereo, w, h);
}

void vrms_server_socket_process() {
    vrms_server_process_queue(vrms_server);
}

void vrms_server_socket_end() {
    pthread_join(socket_thread, NULL);
}
